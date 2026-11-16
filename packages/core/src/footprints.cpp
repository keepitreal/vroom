#include "footprints.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#pragma clang diagnostic pop

#include <algorithm>
#include <cmath>

#include "chart.h"
#include "footprints_layout.h"
#include "theme.h"
#include "viewport.h"

namespace vroom::footprints {

namespace {

// The glyph is a bar (minus) or a crossed pair of bars (plus) sized as a fraction
// of the badge, so restyling the radius scales the whole mark.
constexpr float kGlyphFrac = 0.52f;   // glyph length / diameter
constexpr float kStrokeFrac = 0.20f;  // stroke width / radius

// The hovered badge gets a soft ring outside its edge — the "lit up" state in the
// reference, and the only affordance that reads at a 9px radius.
constexpr float kHaloWidthFrac = 0.42f;  // ring stroke width / radius
constexpr float kHaloGapFrac = 0.30f;    // clear space between badge and ring
constexpr U8CPU kHaloAlpha = 0x66;

// The combined badge's two dots, sitting on the disc's diagonal. They overlap,
// the later trade's drawn in front, and both stay inside the disc — which is
// drawn larger than a one-sided badge to give them room.
constexpr float kCombinedScale = 1.15f;   // combined disc radius / radius
constexpr float kDotFrac = 0.52f;         // dot radius / combined disc radius
constexpr float kDotOffsetFrac = 0.235f;  // per-axis dot offset / combined disc radius

// Brightens `c`'s channels by `mul` (clamped at white), leaving alpha alone.
// Matches price_lines::boost so hover feels the same across both widgets.
SkColor boost(SkColor c, float mul) {
    if (mul <= 1.f) return c;
    const auto up = [mul](U8CPU v) {
        return static_cast<U8CPU>(
            std::min(255.f, static_cast<float>(v) * mul + 0.5f));
    };
    return SkColorSetARGB(SkColorGetA(c), up(SkColorGetR(c)), up(SkColorGetG(c)),
                          up(SkColorGetB(c)));
}

// One laid-out badge, in pixels.
struct Badge {
    float   cx = 0.f;
    float   cy = 0.f;
    float   radius = 0.f;
    int32_t side = -1;
    int32_t first_side = -1;  // upper-left dot of a BOTH badge
    int64_t candle_time_ms = 0;
};

// Walks every badge that could be on screen, in draw order, handing each to `fn`.
// Both passes go through here: the badge the user sees and the badge they can
// hover are the same object by construction.
template <typename Fn>
void for_each_badge(const VroomChart& chart, const Layout& lay,
                    const PriceBounds& bounds, int64_t window_ms, Fn&& fn) {
    if (chart.footprint_buckets.empty() || chart.candles.empty()) return;
    if (window_ms <= 0) return;

    const Metrics m = metrics_from(&chart.footprint_style);
    const float pane_bottom = vroom::price_pane_bottom(lay);
    const float pane_right = vroom::candle_area_width(lay);

    const ::VroomCandle* candles = chart.candles.data();
    const size_t candle_count = chart.candles.size();
    const float line_t = std::clamp(chart.morph_fade, 0.f, 1.f);

    // Only the buckets whose candles are in (or just off) the window matter. One
    // bar of slack each side keeps a badge from popping in at the edge.
    const int64_t slack = chart.candle_duration_ms;
    const auto& buckets = chart.footprint_buckets;
    auto it = std::lower_bound(buckets.begin(), buckets.end(),
                               chart.visible_start_ms - slack,
                               [](const Bucket& b, int64_t t) {
                                   return b.candle_time_ms < t;
                               });

    for (; it != buckets.end(); ++it) {
        if (it->candle_time_ms > chart.visible_end_ms + slack) break;

        // The bar this bucket belongs to, for its high and close.
        const ::VroomCandle* c =
            std::lower_bound(candles, candles + candle_count, it->candle_time_ms,
                             [](const ::VroomCandle& a, int64_t t) {
                                 return a.time_ms < t;
                             });
        if (c == candles + candle_count || c->time_ms != it->candle_time_ms) {
            continue;  // the bar went away; the bucket is about to be rebuilt
        }

        const float cx = vroom::candle_center_x(lay, it->candle_time_ms,
                                                chart.candle_duration_ms,
                                                chart.visible_start_ms, window_ms);
        const float radius = it->has_buys() && it->has_sells()
                                 ? m.radius * kCombinedScale
                                 : m.radius;
        // Cheap reject before laying the badge out.
        if (cx < -radius || cx > pane_right + radius) continue;

        const float high_y = vroom::price_to_y(lay, bounds, c->high);
        const float close_y = vroom::price_to_y(lay, bounds, c->close);
        const BadgeLayout bl = layout_badge(*it, high_y, close_y, line_t, 0.f,
                                            radius, m.margin);
        if (bl.side < 0) continue;
        // A bar below the pane (scrolled off the bottom) parks its badge out of
        // sight; skip rather than draw over the x-axis.
        if (bl.y - radius > pane_bottom) continue;
        Badge b;
        b.cx = cx;
        b.cy = bl.y;
        b.radius = radius;
        b.side = bl.side;
        b.first_side = bl.first_side;
        b.candle_time_ms = it->candle_time_ms;
        fn(b, m);
    }
}

// Buy = "+", sell = "-": one horizontal bar, plus a vertical one for an entry.
void draw_glyph(SkCanvas* canvas, const Badge& b, SkColor color) {
    SkPaint p;
    p.setAntiAlias(true);
    p.setColor(color);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(std::max(1.f, b.radius * kStrokeFrac));
    p.setStrokeCap(SkPaint::kRound_Cap);

    const float arm = b.radius * kGlyphFrac;
    canvas->drawLine(b.cx - arm, b.cy, b.cx + arm, b.cy, p);
    if (b.side == VROOM_FOOTPRINT_BUY) {
        canvas->drawLine(b.cx, b.cy - arm, b.cx, b.cy + arm, p);
    }
}

}  // namespace

void draw(SkCanvas* canvas,
          const VroomChart& chart,
          const Layout& lay,
          const PriceBounds& bounds,
          int64_t window_ms) {
    if (!canvas || chart.footprints.empty()) return;

    const float pane_bottom = vroom::price_pane_bottom(lay);
    const float pane_right = vroom::candle_area_width(lay);
    if (pane_right <= 0.f || pane_bottom <= 0.f) return;

    const SkColor bull = chart.theme.colors[VROOM_COLOR_BULL];
    const SkColor bear = chart.theme.colors[VROOM_COLOR_BEAR];
    const SkColor glyph = chart.theme.colors[VROOM_COLOR_BADGE_TEXT];
    const SkColor disc = SkColorSetA(chart.theme.colors[VROOM_COLOR_BACKGROUND], 0xFF);
    const SkColor disc_edge = chart.theme.colors[VROOM_COLOR_GRID];

    // Badges belong to the plot, not the axis strips: a bar at the right edge
    // must not spill its badge over the price labels.
    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(0.f, 0.f, pane_right, pane_bottom));

    for_each_badge(chart, lay, bounds, window_ms,
                   [&](const Badge& b, const Metrics& m) {
        const bool hovered = chart.hovered_footprint_side == b.side &&
                             chart.hovered_footprint_time_ms == b.candle_time_ms;
        const bool both = b.side == VROOM_FOOTPRINT_BOTH;
        const auto lit = [&](SkColor c) {
            return hovered ? boost(c, m.hover_boost) : c;
        };
        const SkColor fill =
            both ? disc : lit(b.side == VROOM_FOOTPRINT_BUY ? bull : bear);

        if (hovered) {
            SkPaint halo;
            halo.setAntiAlias(true);
            halo.setStyle(SkPaint::kStroke_Style);
            halo.setStrokeWidth(b.radius * kHaloWidthFrac);
            // The combined disc is the background color, which would leave its
            // ring invisible; ring it in its first dot's color instead.
            const SkColor ring =
                both ? lit(b.first_side == VROOM_FOOTPRINT_BUY ? bull : bear) : fill;
            halo.setColor(SkColorSetA(ring, kHaloAlpha));
            const float r =
                b.radius * (1.f + kHaloGapFrac + kHaloWidthFrac * 0.5f);
            canvas->drawCircle(b.cx, b.cy, r, halo);
        }

        SkPaint body;
        body.setAntiAlias(true);
        body.setColor(fill);
        canvas->drawCircle(b.cx, b.cy, b.radius, body);

        if (!both) {
            draw_glyph(canvas, b, glyph);
            return;
        }
        SkPaint edge;
        edge.setAntiAlias(true);
        edge.setStyle(SkPaint::kStroke_Style);
        edge.setStrokeWidth(1.f);
        edge.setColor(disc_edge);
        canvas->drawCircle(b.cx, b.cy, b.radius - 0.5f, edge);

        const bool buy_first = b.first_side == VROOM_FOOTPRINT_BUY;
        const float off = b.radius * kDotOffsetFrac;
        const float dot_r = b.radius * kDotFrac;
        SkPaint dot;
        dot.setAntiAlias(true);
        dot.setColor(lit(buy_first ? bull : bear));
        canvas->drawCircle(b.cx - off, b.cy - off, dot_r, dot);
        // The later trade's dot goes on top, cut out of the earlier one by a
        // disc-colored rim so the overlap reads as front and back.
        SkPaint rim;
        rim.setAntiAlias(true);
        rim.setColor(disc);
        canvas->drawCircle(b.cx + off, b.cy + off, dot_r + 1.f, rim);
        dot.setColor(lit(buy_first ? bear : bull));
        canvas->drawCircle(b.cx + off, b.cy + off, dot_r, dot);
    });

    canvas->restore();
}

::VroomFootprintHit hit_test(const VroomChart& chart,
                             const Layout& lay,
                             const PriceBounds& bounds,
                             float x,
                             float y) {
    ::VroomFootprintHit best{};
    best.side = -1;

    // The rect the badges are clipped to. Reported on every hit so the host can
    // place a tooltip against the real plot edge rather than the element's.
    const float pane_right = vroom::candle_area_width(lay);
    const float pane_bottom = vroom::price_pane_bottom(lay);
    best.pane_left = 0.f;
    best.pane_top = 0.f;
    best.pane_right = pane_right;
    best.pane_bottom = pane_bottom;

    const int64_t window_ms = chart.visible_end_ms - chart.visible_start_ms;
    // Outside the plot there is nothing to hit, matching the draw-time clip.
    if (x < 0.f || x > pane_right) return best;
    if (y < 0.f || y > pane_bottom) return best;

    float best_d2 = 0.f;
    for_each_badge(chart, lay, bounds, window_ms, [&](const Badge& b, const Metrics&) {
        if (!hits_badge(b.cx, b.cy, b.radius, x, y)) return;
        // Badges on neighbouring bars can overlap when zoomed out, and the hit
        // tolerance widens them further; the nearest center is the one meant.
        const float dx = x - b.cx;
        const float dy = y - b.cy;
        const float d2 = dx * dx + dy * dy;
        if (best.side >= 0 && d2 >= best_d2) return;
        best_d2 = d2;
        best.candle_time_ms = b.candle_time_ms;
        best.side = b.side;
        best.center_x = b.cx;
        best.center_y = b.cy;
        best.radius = b.radius;
    });
    return best;
}

}  // namespace vroom::footprints
