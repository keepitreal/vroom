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
#include "line_morph.h"
#include "macd.h"
#include "pane_series.h"
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
          float pane_bottom,
          const vroom::LineMorph* macd_from,
          const vroom::LineMorph* signal_from,
          const vroom::LineMorph* hist_from,
          float morph_t) {
    if (!canvas || candle_right <= 0.f) return;
    const float band_h = pane_bottom - pane_top;
    if (band_h <= 0.f) return;
    morph_t = std::clamp(morph_t, 0.f, 1.f);
    // A fade's outgoing half has no new data at all, so the captures are the
    // whole frame. Nothing on either side means nothing to paint, shell
    // included.
    const std::size_t hist_slots =
        vroom::pane_series::slots(n, hist_from, morph_t);
    if (n == 0 && hist_slots == 0 &&
        vroom::morph_line_count(macd_from, morph_t) == 0 &&
        vroom::morph_line_count(signal_from, morph_t) == 0) {
        return;
    }

    const VroomMACD& cfg = chart.macd;
    const float mid = (pane_top + pane_bottom) * 0.5f;

    // Auto-scale symmetric about zero across the finite values on show, so
    // hiding a series lets the rest fill the pane. User y-zoom scales the
    // amplitude about the zero line; 1.0 is the default auto-fit.
    const double scale = vroom::macd::autoscale(
        cfg.line_visible ? macd_visible : nullptr,
        cfg.signal_visible ? signal_visible : nullptr,
        cfg.hist_visible ? hist_visible : nullptr, n);

    auto y_for = [&](double v) -> float {
        return pane_bottom -
               static_cast<float>(
                   vroom::macd::band_fraction(v, scale, chart.macd_y_scale)) *
                   band_h;
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
    if ((hist_visible || hist_from) && cfg.hist_visible) {
        // The new resolution's width from the first frame, matching how
        // candles::draw widens its bodies while their positions still slide.
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
        const std::size_t hist_from_count =
            vroom::morph_line_count(hist_from, morph_t);
        // Bars read their sign and direction off the drawn geometry rather than
        // the values: mid-morph a top is a blend of two resolutions' numbers,
        // and only where it ended up says which side of zero it is on. y grows
        // downward, so above zero is the smaller y.
        MorphVertex prev;
        for (std::size_t j = 0; j < hist_slots; ++j) {
            const MorphVertex p = vroom::pane_series::vertex(
                lay, visible, n, hist_visible, window_ms, visible_start_ms,
                candle_duration_ms, pane_top, pane_bottom, hist_from,
                hist_from_count, morph_t, hist_slots - 1 - j, y_for);
            if (!p.valid) {
                prev = MorphVertex{};
                continue;
            }
            const bool above = p.y <= zero_y;
            // Building while the bar grows away from zero, easing while it
            // falls back toward it.
            const bool building =
                !prev.valid || (above ? p.y <= prev.y : p.y >= prev.y);
            SkPaint bar;
            bar.setAntiAlias(true);
            if (above) {
                bar.setColor(building ? up : up_easing);
            } else {
                bar.setColor(building ? down : down_easing);
            }
            canvas->drawRect(SkRect::MakeLTRB(p.x - half_body,
                                              std::min(zero_y, p.y),
                                              p.x + half_body,
                                              std::max(zero_y, p.y)),
                             bar);
            prev = p;
        }
    }

    // MACD + signal lines.
    auto stroke_series = [&](const double* series, const vroom::LineMorph* from,
                             SkColor color, float width) {
        if (!series && !from) return;
        SkPaint line;
        line.setAntiAlias(true);
        line.setColor(color);
        line.setStyle(SkPaint::kStroke_Style);
        line.setStrokeWidth(width);
        canvas->drawPath(
            vroom::pane_series::build_path(
                lay, visible, n, series, window_ms, visible_start_ms,
                candle_duration_ms, pane_top, pane_bottom, from, morph_t, y_for),
            line);
    };
    if (cfg.signal_visible) {
        stroke_series(signal_visible, signal_from,
                      color_or(cfg.signal_color, kSignalLine),
                      width_or(cfg.signal_width, kLineWidth));
    }
    if (cfg.line_visible) {  // MACD over signal
        stroke_series(macd_visible, macd_from, color_or(cfg.line_color, kMacdLine),
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
