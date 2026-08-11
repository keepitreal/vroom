#include "volume.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#pragma clang diagnostic pop

#include <algorithm>

#include "theme.h"
#include "viewport.h"
#include "volume_anim.h"

namespace vroom::volume {

namespace {
// Ceiling for the tallest bar when the config inherits it.
constexpr float kDefaultHeightFrac = 0.20f;
}  // namespace

void draw(SkCanvas* canvas,
          const ::VroomCandle* visible,
          std::size_t n,
          const Layout& lay,
          const Theme& theme,
          const ::VroomVolume& cfg,
          float collapse_t,
          int32_t collapse_easing,
          int64_t window_ms,
          int64_t visible_start_ms,
          int64_t candle_duration_ms) {
    if (!canvas || n == 0) return;

    // Auto-fit bar heights to the loudest volume currently in view.
    double max_vol = 0.0;
    for (std::size_t i = 0; i < n; ++i) max_vol = std::max(max_vol, visible[i].volume);
    if (max_vol <= 0.0) return;

    // Resolve every inherit sentinel up front so the loop reads plain values.
    const float height_frac =
        cfg.height_frac >= 0.f ? cfg.height_frac : kDefaultHeightFrac;
    const float opacity = cfg.opacity >= 0.f
        ? cfg.opacity
        : theme.floats[VROOM_FLOAT_VOLUME_OPACITY];
    const float vol_r = cfg.radius_px >= 0.f
        ? cfg.radius_px
        : theme.floats[VROOM_FLOAT_VOLUME_RADIUS_PX];
    const SkColor up_color = cfg.up_color != 0
        ? static_cast<SkColor>(cfg.up_color)
        : theme.colors[VROOM_COLOR_ACCENT_BULL];
    const SkColor down_color = cfg.down_color != 0
        ? static_cast<SkColor>(cfg.down_color)
        : theme.colors[VROOM_COLOR_ACCENT_BEAR];

    const float candle_area_h = vroom::price_pane_bottom(lay);
    const float region_h = candle_area_h * height_frac;
    if (region_h <= 0.f) return;

    const float body_w = vroom::candle_body_width(
        lay, window_ms, candle_duration_ms);
    const float half_body = body_w * 0.5f;

    SkPaint bull_paint;
    bull_paint.setAntiAlias(true);
    bull_paint.setColor(up_color);
    bull_paint.setAlphaf(opacity);

    SkPaint bear_paint;
    bear_paint.setAntiAlias(true);
    bear_paint.setColor(down_color);
    bear_paint.setAlphaf(opacity);

    for (std::size_t i = 0; i < n; ++i) {
        const auto& c = visible[i];
        if (c.volume <= 0.0) continue;
        const bool bull = c.close >= c.open;

        const float cx = vroom::candle_center_x(
            lay, c.time_ms, candle_duration_ms,
            visible_start_ms, window_ms);
        // The 1px floor applies to the settled height, not the animated one, so
        // a collapsing bar can still reach zero.
        const float frac = static_cast<float>(c.volume / max_vol);
        const float full_h = std::max(1.f, frac * region_h);
        const float h = full_h * (1.f - vroom::volume_anim::bar_collapse(
                                            frac, collapse_t, collapse_easing));
        if (h < 0.5f) continue;  // collapsed past the point of drawing anything
        const SkRect rect =
            SkRect::MakeXYWH(cx - half_body, candle_area_h - h, body_w, h);
        const SkPaint& paint = bull ? bull_paint : bear_paint;
        // Round only the top corners; clamp so short/narrow bars don't over-round.
        const float r = std::min({vol_r, body_w * 0.5f, h});
        if (r > 0.f) {
            SkRRect rr;
            const SkVector radii[4] = {{r, r}, {r, r}, {0, 0}, {0, 0}};  // TL, TR, BR, BL
            rr.setRectRadii(rect, radii);
            canvas->drawRRect(rr, paint);
        } else {
            canvas->drawRect(rect, paint);
        }
    }
}

}  // namespace vroom::volume
