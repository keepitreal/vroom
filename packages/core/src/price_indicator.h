// Current-price indicator — a dotted horizontal line at the latest candle's
// close, spanning the candle area and ending in a rounded price tag in the
// y-axis strip. Red/green by whether the latest period closed up or down. Kept
// in its own module alongside candles / labels / crosshair so the chart
// orchestrator stays thin.

#pragma once

#include <algorithm>
#include <cstdint>

#include "color_lerp.h"
#include "theme.h"

struct VroomChart;

namespace vroom {
struct CandleSnapshot;
struct Layout;
struct PriceBounds;
}  // namespace vroom

namespace vroom::price_indicator {

// Draws the line at the latest close's y, from the left edge to the price
// badge when that badge is drawn, otherwise to `candle_right`. `candle_area_h`
// is the y of the x-axis separator; the indicator is skipped if the close maps
// outside [0, candle_area_h].
//
// `morph_from` is the newest candle's outgoing capture (slot 0 of
// VroomChart::morph_from), or null to draw the settled close. When present the
// level eases across `morph_t` on the same clock as the candle it marks, so the
// badge tracks the bar instead of jumping ahead of it — see
// price_indicator_anim.h. Callers must only pass a capture that pairs with the
// newest candle (tip_anchor.h).
//
// Returns the accent it resolved from the theme, before any chart-type blend,
// so the next vroom_chart_begin_price_indicator_morph can start from it. With
// no candles that is the bull accent, so a series that lands mid-morph doesn't
// fade in from transparent.
uint32_t draw(SkCanvas* canvas,
              const VroomChart& chart,
              const Layout& lay,
              const PriceBounds& bounds,
              float candle_right,
              float candle_area_h,
              const CandleSnapshot* morph_from = nullptr,
              float morph_t = 1.f);

}  // namespace vroom::price_indicator

// Layout for that line's dash. The stroke is a SkDashPathEffect, the same
// effect the crosshair, price lines, fair-value gaps and RSI bands use; those
// pass a fixed pattern, and this is the visible-length adjustment they don't
// need. The theme's dash and gap are the lengths you see. A round cap adds
// half the stroke width to each end, so that much comes out of the on interval
// and goes into the off one, keeping the period at dash + gap. A dash shorter
// than the width bottoms out at a dot. Skia-free so the unit tests can cover
// it; see tests/test_price_indicator_anim.cpp.
namespace vroom::price_indicator_dash {

struct Intervals {
    float on;
    float off;  // 0 = draw solid, with no dash effect
};

inline Intervals intervals(float dash, float gap, float width, bool round_cap) {
    dash = std::max(dash, 0.f);
    if (gap <= 0.f) return {dash, 0.f};
    if (!round_cap) return {dash, gap};
    const float on = std::max(dash - width, 0.f);
    return {on, dash + gap - on};
}

// Where the stroke starts so the pattern hangs off `right`, the price pill's
// edge or wherever the line ends. The line draws from here to the left at phase
// 0, so a whole dash always ends flush with `right` however long the line is,
// and the pattern only moves when `right` does. A round cap reaches half the
// width past its stroke, so the stroke starts that much short of `right`.
inline float stroke_start(float right, float width, bool round_cap) {
    return round_cap ? right - width * 0.5f : right;
}

}  // namespace vroom::price_indicator_dash

// That line's stroke and color, and their blend across a candle↔line morph. A
// host that styles the line or picks its accents per chart type would otherwise
// have it snap while the chart around it morphs, so
// vroom_chart_begin_price_indicator_morph captures the outgoing look and the
// line and badge ease into the theme's on the morph's own fade. Skia-free for
// the same reason as the dash layout above.
namespace vroom::price_indicator_style {

struct Style {
    float dash = 0.f;
    float gap = 0.f;
    float width = 0.f;
    float opacity = 0.f;
    bool round_cap = false;
};

inline Style from_theme(const Theme& theme) {
    const float* f = theme.floats;
    Style s;
    s.dash = f[VROOM_FLOAT_PRICE_INDICATOR_DASH_PX];
    s.gap = f[VROOM_FLOAT_PRICE_INDICATOR_GAP_PX];
    s.width = f[VROOM_FLOAT_PRICE_INDICATOR_WIDTH_PX];
    s.opacity = f[VROOM_FLOAT_PRICE_INDICATOR_OPACITY];
    s.round_cap = f[VROOM_FLOAT_PRICE_INDICATOR_ROUND_CAP] > 0.5f;
    return s;
}

// Caps can't blend, so they switch halfway. Dash and gap are visible lengths
// either way (price_indicator_dash::intervals), so on a thin line the switch
// barely shows.
inline Style lerp(const Style& a, const Style& b, float t) {
    const auto mix = [t](float x, float y) { return x + (y - x) * t; };
    Style s;
    s.dash = mix(a.dash, b.dash);
    s.gap = mix(a.gap, b.gap);
    s.width = mix(a.width, b.width);
    s.opacity = mix(a.opacity, b.opacity);
    s.round_cap = t < 0.5f ? a.round_cap : b.round_cap;
    return s;
}

// The look captured at morph fade `from_fade`, headed for the theme's at
// `to_fade`. Equal fades mean settled: the theme draws as is.
struct Morph {
    Style from{};
    uint32_t from_color = 0;
    float from_fade = 0.f;
    float to_fade = 0.f;
};

inline float progress(const Morph& m, float fade) {
    if (m.to_fade == m.from_fade) return 1.f;
    return std::clamp((fade - m.from_fade) / (m.to_fade - m.from_fade), 0.f, 1.f);
}

inline Style current(const Morph& m, const Theme& theme, float fade) {
    const Style to = from_theme(theme);
    const float t = progress(m, fade);
    return t >= 1.f ? to : lerp(m.from, to, t);
}

// `accent` is the color price_indicator::draw resolved from the theme for the
// latest candle; this is the one the line and badge draw with.
inline uint32_t color(const Morph& m, uint32_t accent, float fade) {
    const float t = progress(m, fade);
    return t >= 1.f ? accent : lerp_argb(m.from_color, accent, t);
}

// Starts from whatever is on screen, so a switch that reverses mid-morph turns
// around where the line stands instead of jumping back to a settled look.
// `accent` is the one the last frame resolved.
inline Morph begin(const Morph& m, const Theme& theme, uint32_t accent,
                   float fade, float to_fade) {
    return {current(m, theme, fade), color(m, accent, fade), fade, to_fade};
}

}  // namespace vroom::price_indicator_style
