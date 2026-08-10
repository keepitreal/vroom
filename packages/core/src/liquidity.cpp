#include "liquidity.h"

#include <algorithm>
#include <cstdint>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkShader.h"
#pragma clang diagnostic pop

#include "chart.h"
#include "gradient.h"

namespace vroom::liquidity {

void draw(SkCanvas* canvas,
          const VroomChart& chart,
          const Layout& lay,
          const PriceBounds& bounds,
          float candle_right,
          float candle_area_h) {
    if (!canvas || candle_right <= 0.f || candle_area_h <= 0.f) return;
    if (chart.bands.empty()) return;

    const VroomLiquidityStyle& style = chart.liquidity_style;

    // Leftward reach: the smaller of the px cap and the pane-fraction, clamped so
    // the band never starts left of the pane.
    const float frac_reach = style.width_frac > 0.f ? style.width_frac * candle_right
                                                    : candle_right;
    const float px_reach = style.width_px > 0.f ? style.width_px : candle_right;
    const float reach = std::min(px_reach, frac_reach);
    const float left_x = std::max(0.f, candle_right - reach);

    // Volume that maps to peak opacity: explicit, or the largest band's volume.
    double max_vol = style.max_volume;
    if (max_vol <= 0.0) {
        for (const VroomBand& b : chart.bands) max_vol = std::max(max_vol, b.volume);
    }

    const float min_op = std::clamp(style.min_opacity, 0.f, 1.f);
    const float max_op = std::clamp(style.max_opacity, 0.f, 1.f);

    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(0.f, 0.f, candle_right, candle_area_h));

    const SkPoint pts[2] = {SkPoint{left_x, 0.f}, SkPoint{candle_right, 0.f}};

    for (const VroomBand& b : chart.bands) {
        // Higher price -> smaller y, so max_price is the top edge.
        const float y_top = vroom::price_to_y(lay, bounds, b.max_price);
        const float y_bot = vroom::price_to_y(lay, bounds, b.min_price);
        if (y_bot < 0.f || y_top > candle_area_h) continue;  // fully off-pane

        const SkColor base =
            static_cast<SkColor>(b.side == 0 ? style.buy_color : style.sell_color);

        const double t = max_vol > 0.0
            ? std::clamp(b.volume / max_vol, 0.0, 1.0)
            : 0.0;
        const float alpha = min_op + (max_op - min_op) * static_cast<float>(t);

        SkPaint paint;
        paint.setAntiAlias(true);
        // Transparent at the left of the band's reach, full `alpha` at the axis.
        paint.setShader(vroom::linear_alpha_ramp(pts, base, 0.f, alpha));
        canvas->drawRect(SkRect::MakeLTRB(left_x, y_top, candle_right, y_bot), paint);
    }

    canvas->restore();
}

}  // namespace vroom::liquidity
