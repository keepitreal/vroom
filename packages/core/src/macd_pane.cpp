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
#include "theme.h"
#include "viewport.h"

namespace vroom::macd_pane {

namespace {
constexpr SkColor kMacdLine = 0xff2962ff;    // blue MACD line
constexpr SkColor kSignalLine = 0xffff6d00;  // orange signal line
constexpr SkColor kZeroLine = 0xff484f58;    // zero reference
constexpr SkColor kDivider = 0xff21262d;     // pane separator
constexpr float kPadFrac = 0.85f;            // keep curves off the band edges
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

    const float mid = (pane_top + pane_bottom) * 0.5f;
    // User y-zoom scales the amplitude about the zero line (mid). 1.0 = the
    // default auto-fit; >1 zooms in, <1 zooms out.
    const float half =
        band_h * 0.5f * kPadFrac * static_cast<float>(chart.macd_y_scale);

    // Auto-scale symmetric about zero across all visible finite values.
    double scale = 0.0;
    auto track = [&](const double* s) {
        if (!s) return;
        for (std::size_t i = 0; i < n; ++i) {
            if (std::isfinite(s[i])) scale = std::max(scale, std::abs(s[i]));
        }
    };
    track(macd_visible);
    track(signal_visible);
    track(hist_visible);

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
    SkPaint zero;
    zero.setAntiAlias(true);
    zero.setColor(kZeroLine);
    zero.setStrokeWidth(1.f);
    canvas->drawLine(0.f, mid, candle_right, mid, zero);

    // Histogram bars: from the zero line to y_for(hist), 4-color (green above /
    // red below, full alpha when accelerating, half when decelerating).
    if (hist_visible) {
        const float body_w =
            vroom::candle_body_width(lay, window_ms, candle_duration_ms);
        const float half_body = body_w * 0.5f;
        const float zero_y = y_for(0.0);
        const SkColor bull = chart.theme.colors[VROOM_COLOR_ACCENT_BULL];
        const SkColor bear = chart.theme.colors[VROOM_COLOR_ACCENT_BEAR];
        for (std::size_t i = 0; i < n; ++i) {
            const double h = hist_visible[i];
            if (!std::isfinite(h)) continue;
            const bool have_prev = i > 0 && std::isfinite(hist_visible[i - 1]);
            const double prev = have_prev ? hist_visible[i - 1] : h;
            bool accelerating = true;
            if (have_prev) {
                accelerating = h >= 0.0 ? h >= prev : h <= prev;
            }
            SkPaint bar;
            bar.setAntiAlias(true);
            bar.setColor(h >= 0.0 ? bull : bear);
            bar.setAlphaf(accelerating ? 1.0f : 0.5f);
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
    auto stroke_series = [&](const double* series, SkColor color) {
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
        line.setStrokeWidth(1.5f);
        canvas->drawPath(path.detach(), line);
    };
    stroke_series(signal_visible, kSignalLine);
    stroke_series(macd_visible, kMacdLine);  // MACD over signal

    // Caption, top-left of the pane.
    auto tf = vroom::axis_typeface();
    if (tf) {
        SkFont font(tf, chart.theme.floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX]);
        font.setSubpixel(true);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        char caption[40];
        std::snprintf(caption, sizeof(caption), "MACD %d %d %d", chart.macd_fast,
                      chart.macd_slow, chart.macd_signal);
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

    // "0" label in the y-axis strip at the zero line.
    if (tf) {
        SkFont font(tf, chart.theme.floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX]);
        font.setSubpixel(true);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        SkPaint text_paint;
        text_paint.setAntiAlias(true);
        text_paint.setColor(chart.theme.colors[VROOM_COLOR_AXIS_TEXT]);
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
