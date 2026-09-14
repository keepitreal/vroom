#include "atr_pane.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypeface.h"
#pragma clang diagnostic pop

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "atr.h"
#include "chart.h"
#include "fonts.h"
#include "line_morph.h"
#include "pane_series.h"
#include "price_format.h"
#include "style_inherit.h"
#include "theme.h"
#include "viewport.h"

namespace vroom::atr_pane {

namespace {
using vroom::style::color_or;
using vroom::style::width_or;

constexpr SkColor kAtrLine = 0xff26a69a;  // teal
constexpr SkColor kDivider = 0xff21262d;  // pane separator
constexpr float kLineWidth = 1.5f;
}  // namespace

void draw(SkCanvas* canvas,
          const VroomChart& chart,
          const Layout& lay,
          const ::VroomCandle* visible,
          std::size_t n,
          const double* atr_visible,
          int64_t window_ms,
          int64_t visible_start_ms,
          int64_t candle_duration_ms,
          float candle_right,
          float pane_top,
          float pane_bottom,
          const vroom::LineMorph* from,
          float morph_t) {
    if (!canvas || candle_right <= 0.f) return;
    const float band_h = pane_bottom - pane_top;
    if (band_h <= 0.f) return;
    morph_t = std::clamp(morph_t, 0.f, 1.f);
    // A fade's outgoing half has no new data at all, so the capture is the whole
    // frame. Nothing on either side means nothing to paint, shell included.
    if (n == 0 && vroom::morph_line_count(from, morph_t) == 0) return;

    const VroomATR& cfg = chart.atr;

    // ATR is strictly positive, so the domain runs 0..peak off the bottom edge.
    // User y-zoom stretches it from there; 1.0 is the default fit.
    const double scale = vroom::atr::autoscale(atr_visible, n);
    auto y_for = [&](double v) -> float {
        return pane_bottom -
               static_cast<float>(
                   vroom::atr::band_fraction(v, scale, chart.atr_y_scale)) *
                   band_h;
    };

    // The peak the label reports. Mid-morph it eases out of the fit the capture
    // was taken against, so the number tracks the curve instead of jumping to
    // the new resolution's peak on the first frame.
    const double label_scale =
        (from && morph_t < 1.f)
            ? from->scale + (scale - from->scale) * static_cast<double>(morph_t)
            : scale;

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

    if (atr_visible || from) {
        SkPaint line;
        line.setAntiAlias(true);
        line.setColor(color_or(cfg.line_color, kAtrLine));
        line.setStyle(SkPaint::kStroke_Style);
        line.setStrokeWidth(width_or(cfg.line_width, kLineWidth));
        canvas->drawPath(
            vroom::pane_series::build_path(
                lay, visible, n, atr_visible, window_ms, visible_start_ms,
                candle_duration_ms, pane_top, pane_bottom, from, morph_t, y_for),
            line);
    }

    // Caption, top-left of the pane.
    auto tf = vroom::axis_typeface();
    if (tf) {
        SkFont font(tf, chart.theme.floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX]);
        font.setSubpixel(true);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        char caption[24];
        std::snprintf(caption, sizeof(caption), "ATR %d", cfg.period);
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

    // Peak label in the y-axis strip — ATR is in price units, so it shares the
    // price axis's formatting. Hidden with that strip.
    if (tf && label_scale > 0.0 && lay.y_axis_opacity > 0.f) {
        SkFont font(tf, chart.theme.floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX]);
        font.setSubpixel(true);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        SkPaint text_paint;
        text_paint.setAntiAlias(true);
        text_paint.setColor(chart.theme.colors[VROOM_COLOR_AXIS_TEXT]);
        text_paint.setAlphaf(text_paint.getAlphaf() * lay.y_axis_opacity);
        char label[48];
        vroom::format_price(label, sizeof(label), label_scale, chart.price_fmt);
        SkRect tb;
        const float tw = font.measureText(label, std::strlen(label),
                                          SkTextEncoding::kUTF8, &tb);
        const float axis_center_x = lay.width_px - lay.y_axis_width_px * 0.5f;
        const float text_x = axis_center_x - tw * 0.5f;
        // A series' own peak lands at the top of the padded band whatever the
        // fit is, so the label's height doesn't move as the morph re-scales —
        // only the number it reports does. Clamped into the band so a zoomed-in
        // peak doesn't label another pane.
        const float half_text = (tb.fBottom - tb.fTop) * 0.5f;
        const float peak_y = std::clamp(
            pane_bottom - static_cast<float>(vroom::atr::band_fraction(
                              label_scale, label_scale, chart.atr_y_scale)) *
                              band_h,
            pane_top + half_text, pane_bottom - half_text);
        const float baseline_y = peak_y - (tb.fTop + tb.fBottom) * 0.5f;
        canvas->drawString(label, text_x, baseline_y, font, text_paint);
    }
}

}  // namespace vroom::atr_pane
