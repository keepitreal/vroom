#include "rsi_pane.h"

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
#include "include/effects/SkDashPathEffect.h"
#pragma clang diagnostic pop

#include <cmath>
#include <cstdio>
#include <cstring>

#include "chart.h"
#include "fonts.h"
#include "style_inherit.h"
#include "theme.h"
#include "viewport.h"

namespace vroom::rsi_pane {

namespace {
using vroom::style::color_or;
using vroom::style::width_or;

constexpr SkColor kRsiLine = 0xff8957e5;    // violet RSI line
constexpr SkColor kRsiMaLine = 0xffd29922;  // amber RSI moving-average trendline
constexpr SkColor kRefLine = 0xff30363d;    // band reference lines
constexpr SkColor kDivider = 0xff21262d;    // pane separator
constexpr SkScalar kDash[2] = {3.f, 3.f};
constexpr float kLineWidth = 1.5f;          // default stroke for both lines
}  // namespace

void draw(SkCanvas* canvas,
          const VroomChart& chart,
          const Layout& lay,
          const ::VroomCandle* visible,
          std::size_t n,
          const double* rsi_visible,
          const double* rsi_ma_visible,
          int64_t window_ms,
          int64_t visible_start_ms,
          int64_t candle_duration_ms,
          float candle_right,
          float pane_top,
          float pane_bottom) {
    if (!canvas || n == 0 || candle_right <= 0.f) return;
    const float band_h = pane_bottom - pane_top;
    if (band_h <= 0.f) return;

    // Map 0..100 about the pane center so the user y-zoom scales symmetrically
    // around 50. z == 1 reproduces the default fit (0 -> pane_bottom, 100 ->
    // pane_top); z > 1 zooms in (taller), z < 1 zooms out (flatter).
    const VroomRSI& cfg = chart.rsi;
    const float center_y = (pane_top + pane_bottom) * 0.5f;
    const float z = static_cast<float>(chart.rsi_y_scale);
    auto y_for = [&](double v) -> float {
        return center_y - static_cast<float>((v - 50.0) / 100.0) * band_h * z;
    };

    // Mask the band with the background so candles/volume that overflow below
    // the (shortened) price pane don't show through behind the RSI. The y-axis
    // strip to the right is already masked full-height by draw_chart.
    SkPaint bg;
    bg.setColor(chart.theme.colors[VROOM_COLOR_BACKGROUND]);
    canvas->drawRect(SkRect::MakeLTRB(0.f, pane_top, candle_right, pane_bottom),
                     bg);

    // Pane separator (top edge).
    SkPaint divider;
    divider.setColor(kDivider);
    divider.setStrokeWidth(1.f);
    canvas->drawLine(0.f, pane_top, candle_right, pane_top, divider);

    // Keep the line + reference lines inside the pane / candle area.
    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(0.f, pane_top, candle_right, pane_bottom));

    // Configurable band reference lines (overbought / oversold).
    const float y_upper = y_for(cfg.upper_band);
    const float y_lower = y_for(cfg.lower_band);
    if (cfg.bands_visible) {
        SkPaint ref;
        ref.setAntiAlias(true);
        ref.setColor(color_or(cfg.band_color, kRefLine));
        ref.setStrokeWidth(1.f);
        ref.setPathEffect(SkDashPathEffect::Make(kDash, 0.f));
        canvas->drawLine(0.f, y_upper, candle_right, y_upper, ref);
        canvas->drawLine(0.f, y_lower, candle_right, y_lower, ref);
    }

    // Strokes a value series as a polyline; a NaN lifts the pen so undefined
    // leading values (i < period) leave a gap.
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
    if (cfg.line_visible) {
        stroke_series(rsi_visible, color_or(cfg.line_color, kRsiLine),
                      width_or(cfg.line_width, kLineWidth));
    }
    // Trendline on top. Its own visibility is handled upstream: a hidden
    // trendline isn't computed, so the caller passes null.
    stroke_series(rsi_ma_visible, color_or(cfg.ma_color, kRsiMaLine),
                  width_or(cfg.ma_width, kLineWidth));

    // Caption, top-left of the pane.
    auto tf = vroom::axis_typeface();
    if (tf) {
        SkFont font(tf, chart.theme.floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX]);
        font.setSubpixel(true);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);

        char caption[24];
        std::snprintf(caption, sizeof(caption), "RSI %d", cfg.period);
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

    // 70 / 30 labels in the y-axis strip (drawn after the clip; the strip's
    // background was already masked by draw_chart before this call). They label
    // the band rules, so they go with them.
    if (tf && cfg.bands_visible) {
        SkFont font(tf, chart.theme.floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX]);
        font.setSubpixel(true);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        SkPaint text_paint;
        text_paint.setAntiAlias(true);
        text_paint.setColor(chart.theme.colors[VROOM_COLOR_AXIS_TEXT]);

        const float axis_center_x = lay.width_px - lay.y_axis_width_px * 0.5f;
        auto label = [&](const char* s, float y) {
            SkRect tb;
            const float tw = font.measureText(s, std::strlen(s),
                                              SkTextEncoding::kUTF8, &tb);
            const float text_x = axis_center_x - tw * 0.5f;
            const float baseline_y = y - (tb.fTop + tb.fBottom) * 0.5f;
            canvas->drawString(s, text_x, baseline_y, font, text_paint);
        };
        char ub[8];
        char lb[8];
        std::snprintf(ub, sizeof(ub), "%.0f", cfg.upper_band);
        std::snprintf(lb, sizeof(lb), "%.0f", cfg.lower_band);
        label(ub, y_upper);
        label(lb, y_lower);
    }
}

}  // namespace vroom::rsi_pane
