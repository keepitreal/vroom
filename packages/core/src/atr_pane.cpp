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

#include "chart.h"
#include "fonts.h"
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
constexpr float kPadFrac = 0.85f;         // keep the curve off the band edges
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
          float pane_bottom) {
    if (!canvas || n == 0 || candle_right <= 0.f) return;
    const float band_h = pane_bottom - pane_top;
    if (band_h <= 0.f) return;

    const VroomATR& cfg = chart.atr;

    // ATR is strictly positive, so the domain runs 0..peak off the bottom edge.
    // User y-zoom stretches it from there; 1.0 is the default fit.
    double scale = 0.0;
    if (atr_visible) {
        for (std::size_t i = 0; i < n; ++i) {
            if (std::isfinite(atr_visible[i])) {
                scale = std::max(scale, atr_visible[i]);
            }
        }
    }
    const float span = band_h * kPadFrac * static_cast<float>(chart.atr_y_scale);
    auto y_for = [&](double v) -> float {
        if (scale <= 0.0) return pane_bottom;
        return pane_bottom - static_cast<float>(v / scale) * span;
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

    if (atr_visible) {
        SkPathBuilder path;
        bool pen_down = false;
        for (std::size_t i = 0; i < n; ++i) {
            const double v = atr_visible[i];
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
        line.setColor(color_or(cfg.line_color, kAtrLine));
        line.setStyle(SkPaint::kStroke_Style);
        line.setStrokeWidth(width_or(cfg.line_width, kLineWidth));
        canvas->drawPath(path.detach(), line);
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
    if (tf && scale > 0.0 && lay.y_axis_opacity > 0.f) {
        SkFont font(tf, chart.theme.floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX]);
        font.setSubpixel(true);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        SkPaint text_paint;
        text_paint.setAntiAlias(true);
        text_paint.setColor(chart.theme.colors[VROOM_COLOR_AXIS_TEXT]);
        text_paint.setAlphaf(text_paint.getAlphaf() * lay.y_axis_opacity);
        char label[48];
        vroom::format_price(label, sizeof(label), scale, chart.price_fmt);
        SkRect tb;
        const float tw = font.measureText(label, std::strlen(label),
                                          SkTextEncoding::kUTF8, &tb);
        const float axis_center_x = lay.width_px - lay.y_axis_width_px * 0.5f;
        const float text_x = axis_center_x - tw * 0.5f;
        // Clamp into the band so a zoomed-in peak doesn't label another pane.
        const float half_text = (tb.fBottom - tb.fTop) * 0.5f;
        const float peak_y = std::clamp(y_for(scale), pane_top + half_text,
                                        pane_bottom - half_text);
        const float baseline_y = peak_y - (tb.fTop + tb.fBottom) * 0.5f;
        canvas->drawString(label, text_x, baseline_y, font, text_paint);
    }
}

}  // namespace vroom::atr_pane
