#include "macd_pane.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypeface.h"
#pragma clang diagnostic pop

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "chart.h"
#include "fonts.h"
#include "style_inherit.h"
#include "theme.h"
#include "viewport.h"

namespace vroom::macd_pane {

namespace {
using vroom::style::color_or;
using vroom::style::width_or;

constexpr SkColor kMacdLine = 0xff2962ff;    // blue MACD line
constexpr SkColor kSignalLine = 0xffff6d00;  // orange signal line
constexpr SkColor kZeroLine = 0xff484f58;    // zero reference
constexpr SkColor kDivider = 0xff21262d;     // pane separator
constexpr float kPadFrac = 0.85f;            // keep curves off the band edges
constexpr float kLineWidth = 1.5f;           // default stroke for both lines
constexpr float kFadedAlpha = 0.5f;          // easing histogram bars

// The easing histogram bars default to their own base color dimmed, which is
// how the pane read before the colors were configurable.
SkColor faded_or(uint32_t configured, SkColor base) {
    return vroom::style::faded_or(configured, base, kFadedAlpha);
}
}  // namespace

void draw(SkCanvas* canvas,
          const VroomChart& chart,
          const Layout& lay,
          const ::VroomCandle* visible,
          std::size_t n,
          const double* macd_visible,
          const double* signal_visible,
          const double* hist_visible,
          int64_t window_ms,
          int64_t visible_start_ms,
          int64_t candle_duration_ms,
          float candle_right,
          float pane_top,
          float pane_bottom) {
    if (!canvas || n == 0 || candle_right <= 0.f) return;
    const float band_h = pane_bottom - pane_top;
    if (band_h <= 0.f) return;

    const VroomMACD& cfg = chart.macd;
    const float mid = (pane_top + pane_bottom) * 0.5f;
    // User y-zoom scales the amplitude about the zero line (mid). 1.0 = the
    // default auto-fit; >1 zooms in, <1 zooms out.
    const float half =
        band_h * 0.5f * kPadFrac * static_cast<float>(chart.macd_y_scale);

    // Auto-scale symmetric about zero across the finite values on show, so
    // hiding a series lets the rest fill the pane.
    double scale = 0.0;
    auto track = [&](const double* s, bool shown) {
        if (!s || !shown) return;
        for (std::size_t i = 0; i < n; ++i) {
            if (std::isfinite(s[i])) scale = std::max(scale, std::abs(s[i]));
        }
    };
    track(macd_visible, cfg.line_visible != 0);
    track(signal_visible, cfg.signal_visible != 0);
    track(hist_visible, cfg.hist_visible != 0);

    auto y_for = [&](double v) -> float {
        if (scale <= 0.0) return mid;
        return mid - static_cast<float>(v / scale) * half;
    };

    // Mask the band (candles can overflow below the shortened price pane).
    SkPaint bg;
    bg.setColor(chart.theme.colors[VROOM_COLOR_BACKGROUND]);
    canvas->drawRect(SkRect::MakeLTRB(0.f, pane_top, candle_right, pane_bottom),
                     bg);

    // Pane separator (top edge).
    SkPaint divider;
    divider.setColor(kDivider);
    divider.setStrokeWidth(1.f);
    canvas->drawLine(0.f, pane_top, candle_right, pane_top, divider);

    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(0.f, pane_top, candle_right, pane_bottom));

    // Zero line.
    if (cfg.zero_visible) {
        SkPaint zero;
        zero.setAntiAlias(true);
        zero.setColor(color_or(cfg.zero_color, kZeroLine));
        zero.setStrokeWidth(1.f);
        canvas->drawLine(0.f, mid, candle_right, mid, zero);
    }

    // Histogram bars: from the zero line to y_for(hist), 4-color — above or
    // below zero, each in a building and an easing shade. A bar is building
    // while it grows away from zero and easing while it falls back toward it.
    if (hist_visible && cfg.hist_visible) {
        const float body_w =
            vroom::candle_body_width(lay, window_ms, candle_duration_ms);
        const float half_body = body_w * 0.5f;
        const float zero_y = y_for(0.0);
        const SkColor up =
            color_or(cfg.hist_up_color, chart.theme.colors[VROOM_COLOR_ACCENT_BULL]);
        const SkColor down =
            color_or(cfg.hist_down_color, chart.theme.colors[VROOM_COLOR_ACCENT_BEAR]);
        const SkColor up_easing = faded_or(cfg.hist_up_fading_color, up);
        const SkColor down_easing = faded_or(cfg.hist_down_fading_color, down);
        for (std::size_t i = 0; i < n; ++i) {
            const double h = hist_visible[i];
            if (!std::isfinite(h)) continue;
            const bool have_prev = i > 0 && std::isfinite(hist_visible[i - 1]);
            const double prev = have_prev ? hist_visible[i - 1] : h;
            bool building = true;
            if (have_prev) {
                building = h >= 0.0 ? h >= prev : h <= prev;
            }
            SkPaint bar;
            bar.setAntiAlias(true);
            if (h >= 0.0) {
                bar.setColor(building ? up : up_easing);
            } else {
                bar.setColor(building ? down : down_easing);
            }
            const float cx = vroom::candle_center_x(
                lay, visible[i].time_ms, candle_duration_ms, visible_start_ms,
                window_ms);
            const float vy = y_for(h);
            const float top = std::min(zero_y, vy);
            const float bot = std::max(zero_y, vy);
            canvas->drawRect(
                SkRect::MakeLTRB(cx - half_body, top, cx + half_body, bot), bar);
        }
    }

    // MACD + signal lines.
    auto stroke_series = [&](const double* series, SkColor color, float width) {
        if (!series) return;
        SkPathBuilder path;
        bool pen_down = false;
        for (std::size_t i = 0; i < n; ++i) {
            const double v = series[i];
            if (!std::isfinite(v)) {
                pen_down = false;
                continue;
            }
            const float x = vroom::candle_center_x(
                lay, visible[i].time_ms, candle_duration_ms, visible_start_ms,
                window_ms);
            const float y = y_for(v);
            if (pen_down) {
                path.lineTo(x, y);
            } else {
                path.moveTo(x, y);
                pen_down = true;
            }
        }
        SkPaint line;
        line.setAntiAlias(true);
        line.setColor(color);
        line.setStyle(SkPaint::kStroke_Style);
        line.setStrokeWidth(width);
        canvas->drawPath(path.detach(), line);
    };
    if (cfg.signal_visible) {
        stroke_series(signal_visible, color_or(cfg.signal_color, kSignalLine),
                      width_or(cfg.signal_width, kLineWidth));
    }
    if (cfg.line_visible) {  // MACD over signal
        stroke_series(macd_visible, color_or(cfg.line_color, kMacdLine),
                      width_or(cfg.line_width, kLineWidth));
    }

    // Caption, top-left of the pane.
    auto tf = vroom::axis_typeface();
    if (tf) {
        SkFont font(tf, chart.theme.floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX]);
        font.setSubpixel(true);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        char caption[40];
        std::snprintf(caption, sizeof(caption), "MACD %d %d %d", cfg.fast,
                      cfg.slow, cfg.signal);
        SkRect cb;
        font.measureText(caption, std::strlen(caption), SkTextEncoding::kUTF8,
                         &cb);
        SkPaint cap_paint;
        cap_paint.setAntiAlias(true);
        cap_paint.setColor(chart.theme.colors[VROOM_COLOR_AXIS_TEXT]);
        canvas->drawString(caption, 6.f, pane_top + 4.f - cb.fTop, font,
                           cap_paint);
    }
    canvas->restore();

    // "0" label in the y-axis strip at the zero line — hidden with that strip.
    if (tf && lay.y_axis_opacity > 0.f) {
        SkFont font(tf, chart.theme.floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX]);
        font.setSubpixel(true);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        SkPaint text_paint;
        text_paint.setAntiAlias(true);
        text_paint.setColor(chart.theme.colors[VROOM_COLOR_AXIS_TEXT]);
        text_paint.setAlphaf(text_paint.getAlphaf() * lay.y_axis_opacity);
        SkRect tb;
        const float tw =
            font.measureText("0", 1, SkTextEncoding::kUTF8, &tb);
        const float axis_center_x = lay.width_px - lay.y_axis_width_px * 0.5f;
        const float text_x = axis_center_x - tw * 0.5f;
        const float baseline_y = mid - (tb.fTop + tb.fBottom) * 0.5f;
        canvas->drawString("0", text_x, baseline_y, font, text_paint);
    }
}

}  // namespace vroom::macd_pane
