// Footprint bucketing + badge geometry — pure functions, no Skia, no state.
//
// Split out from footprints.cpp for the same reason as price_line_layout.h: the
// render pass and the hit-test pass must agree on where every badge sits, or the
// user ends up hovering nothing where a badge clearly is. Being Skia-free it also
// unit-tests without a Skia checkout.

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "vroom/vroom_chart.h"

namespace vroom::footprints {

// Badge radius, the gap between two stacked badges, and the gap between the
// candle's high and the first badge. Defaults for a zero/negative style field.
constexpr float kRadius = 9.f;
constexpr float kGap = 4.f;
constexpr float kMargin = 8.f;
constexpr float kHoverBoost = 1.25f;

// Slop around the circle so a 9px badge is still a comfortable target, on a
// mouse and under a thumb alike.
constexpr float kHitTolerance = 3.f;

// One candle's footprints, split by side. Both lists hold indices into the array
// the host passed to vroom_chart_set_footprints — its order, so a caller can map
// them straight back to its own trade objects — sorted ascending by time.
struct Bucket {
    int64_t              candle_time_ms = 0;
    std::vector<int32_t> buys;
    std::vector<int32_t> sells;
    // Time of the last trade on each side, which decides the stack order. Only
    // meaningful when the matching list is non-empty.
    int64_t buy_latest_ms = 0;
    int64_t sell_latest_ms = 0;

    bool has_buys() const { return !buys.empty(); }
    bool has_sells() const { return !sells.empty(); }
    // Every candle that made it into a bucket has at least one badge to draw.
    int32_t badge_count() const { return (has_buys() ? 1 : 0) + (has_sells() ? 1 : 0); }
};

// Index of the candle whose [time_ms, time_ms + duration_ms) window contains
// `time_ms`, or -1 when it falls in no window — before the first candle, or in a
// gap (a weekend, a halt) where no bar exists. Candles must be ascending.
int32_t bucket_index(const ::VroomCandle* candles, size_t count,
                     int64_t duration_ms, int64_t time_ms);

// Groups `prints` onto candles. The result is ascending by candle time and holds
// only candles that got at least one footprint, so it is also the dedupe: however
// many trades a bar collected, it comes back as one bucket with at most two sides.
// Footprints with an unknown side, or that land in no candle's window, are dropped.
std::vector<Bucket> build_buckets(const ::VroomCandle* candles, size_t count,
                                  int64_t duration_ms,
                                  const ::VroomFootprint* prints, size_t print_count);

// The laid-out badge stack for one candle: one or two centers, going up from the
// candle's high. Slot 0 is the one nearest the bar.
struct Stack {
    int32_t count = 0;
    int32_t sides[2]{};  // VroomFootprintSide per slot
    float   y[2]{};      // center y per slot
};

// Stacks `bucket`'s badges above `high_y` (the pixel y of the candle's high).
//
// The side whose latest trade came *first* takes the lower slot, so reading the
// stack bottom-to-top replays the order the trades happened in. The whole stack
// shifts down if it would poke out the top of the pane, keeping badges on a bar
// near the high of the view visible and hoverable rather than clipped away.
//
// Style fields at or below zero fall back to kRadius / kGap / kMargin.
Stack layout_stack(const Bucket& bucket, float high_y, float pane_top,
                   float radius, float gap, float margin);

// True when (x, y) is within a badge of `radius` centered at (cx, cy), plus
// kHitTolerance.
bool hits_badge(float cx, float cy, float radius, float x, float y);

// Resolves the style struct's raw fields to the values the layout should use.
// A null `style`, or any field left at zero, takes the default.
struct Metrics {
    float radius = kRadius;
    float gap = kGap;
    float margin = kMargin;
    float hover_boost = kHoverBoost;
};
Metrics metrics_from(const ::VroomFootprintStyle* style);

}  // namespace vroom::footprints
