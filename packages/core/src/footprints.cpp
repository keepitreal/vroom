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

        // The bar this bucket belongs to, for its high.
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
        // Cheap reject before laying the stack out.
        if (cx < -m.radius || cx > pane_right + m.radius) continue;

        const float high_y = vroom::price_to_y(lay, bounds, c->high);
        const Stack stack = layout_stack(*it, high_y, 0.f, m.radius, m.gap, m.margin);

        for (int32_t i = 0; i < stack.count; ++i) {
            // A bar whose high is below the pane (scrolled off the bottom) parks
            // its stack out of sight; skip rather than draw over the x-axis.
            if (stack.y[i] - m.radius > pane_bottom) continue;
            Badge b;
            b.cx = cx;
            b.cy = stack.y[i];
            b.radius = m.radius;
            b.side = stack.sides[i];
            b.candle_time_ms = it->candle_time_ms;
            fn(b, m);
        }
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

    // Badges belong to the plot, not the axis strips: a bar at the right edge
    // must not spill its badge over the price labels.
    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(0.f, 0.f, pane_right, pane_bottom));

    for_each_badge(chart, lay, bounds, window_ms,
                   [&](const Badge& b, const Metrics& m) {
        const bool hovered = chart.hovered_footprint_side == b.side &&
                             chart.hovered_footprint_time_ms == b.candle_time_ms;
        const SkColor base = b.side == VROOM_FOOTPRINT_BUY ? bull : bear;
        const SkColor fill = hovered ? boost(base, m.hover_boost) : base;

        if (hovered) {
            SkPaint halo;
            halo.setAntiAlias(true);
            halo.setStyle(SkPaint::kStroke_Style);
            halo.setStrokeWidth(b.radius * kHaloWidthFrac);
            halo.setColor(SkColorSetA(fill, kHaloAlpha));
            const float r =
                b.radius * (1.f + kHaloGapFrac + kHaloWidthFrac * 0.5f);
            canvas->drawCircle(b.cx, b.cy, r, halo);
        }

        SkPaint body;
        body.setAntiAlias(true);
        body.setColor(fill);
        canvas->drawCircle(b.cx, b.cy, b.radius, body);

        draw_glyph(canvas, b, glyph);
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
        // Stacked badges are drawn with a gap, but a generous hit tolerance can
        // still overlap; the nearest center is the one the user meant.
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
