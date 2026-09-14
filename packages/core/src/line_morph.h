// Interval-morph capture for indicator series — the line-shaped counterpart to
// viewport.h's CandleSnapshot. Pure geometry, no Skia, so it builds into the
// unit-test target.
//
// A timeframe switch replaces every indicator cache at once (see
// vroom_chart_set_candles), so an outgoing shape has to be captured before the
// swap if it is going to reshape into the new one. Captures normalize the same
// way the candle one does — x as a fraction of the candle-area width, y as a
// fraction of the band the series was drawn in — which keeps them correct
// across a mid-morph resize, across the y-axis rescale the switch brings, and,
// for the indicator panes, across a change in the pane's own auto-fit.

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "vroom/vroom_chart.h"  // ::VroomOverlay

namespace vroom {

// Which series a capture belongs to. Pairing is by identity rather than
// position so an indicator toggled in the same commit as the timeframe can't
// reshape one line out of another's geometry.
enum class LineKind : uint8_t {
    Overlay,
    Vwap,
    BollingerUpper,
    BollingerMiddle,
    BollingerLower,
    IchimokuTenkan,
    IchimokuKijun,
    IchimokuSenkouA,
    IchimokuSenkouB,
    IchimokuChikou,
    Rsi,
    RsiMa,
    Macd,
    MacdSignal,
    MacdHistogram,
    Atr,
};

// Identifies one captured series. The moving-average overlays are a
// user-ordered vector rather than a fixed slot, so `index` alone would pair two
// different MAs if one were added or removed during the switch; `tag` carries
// the overlay's kind, period and source, so a mismatch finds no capture and the
// line simply snaps. Both are 0 for every other series.
struct LineKey {
    LineKind kind;
    int32_t  index = 0;
    int32_t  tag = 0;
};

inline bool operator==(const LineKey& a, const LineKey& b) {
    return a.kind == b.kind && a.index == b.index && a.tag == b.tag;
}

// The key for the moving-average overlay at `index`, packing its kind, source
// and period into the tag. Periods above 65535 alias, which no usable lookback
// reaches.
inline LineKey overlay_line_key(const ::VroomOverlay& ov, std::size_t index) {
    const int32_t tag =
        (ov.kind << 24) ^ (ov.source << 16) ^ (ov.period & 0xffff);
    return LineKey{LineKind::Overlay, static_cast<int32_t>(index), tag};
}

// One captured vertex. `x` is a fraction of the candle-area width and `y` a
// fraction of the band the series was drawn in (0 = bottom edge, 1 = top).
// `valid` is false where the source value was NaN, which is how a warmup gap
// survives the capture.
struct LineSnapshot {
    float x = 0.f;
    float y = 0.f;
    bool  valid = false;
};

// One captured series. Slot 0 is the newest vertex, matching the right-edge
// indexing candles::draw pairs on. `scale` is the auto-fit its pane band was
// sized to at capture time, which the pane's axis label reads back mid-morph;
// 0 for series on the price scale, whose band is the price bounds.
struct LineMorph {
    LineKey key{LineKind::Overlay};
    double  scale = 0.0;
    std::vector<LineSnapshot> pts;
};

// The capture for `key`, or null when nothing matching was captured — which is
// what an indicator enabled mid-morph gets, so it draws its new shape directly
// instead of reshaping out of geometry that was never on screen.
inline const LineMorph* find_line_morph(const std::vector<LineMorph>& lines,
                                        const LineKey& key) {
    for (const LineMorph& line : lines) {
        if (line.key == key) return &line;
    }
    return nullptr;
}

// How many captured slots still contribute to a frame, mirroring
// morph_from_count: 0 once the morph lands, which collapses the draw path back
// to the new series alone. Drawing routines take max(n, this) as their slot
// count, pairing the new vertex at slot k with the captured one at pts[k].
inline std::size_t morph_line_count(const LineMorph* from, float morph_t) {
    return (from && morph_t < 1.f) ? from->pts.size() : 0;
}

// One slot resolved to screen pixels. `valid` false means neither side defined
// the slot, and the polyline lifts its pen there.
struct MorphVertex {
    float x = 0.f;
    float y = 0.f;
    bool  valid = false;
};

// Blends a slot's captured position toward its new one. Both sides arrive in
// pixels: the caller converts the capture's fractions through whichever band
// the series lives in, which is the only part that differs between a price-pane
// overlay and an indicator pane.
//
// A slot only one side defines holds that side's position rather than lifting
// the pen. The warmup gap at a line's left end spans a different amount of time
// at each resolution, and sliding that end reads far better than blinking it.
inline MorphVertex morph_vertex(const MorphVertex& to, const MorphVertex& from,
                                float morph_t) {
    if (!to.valid) return from;
    if (!from.valid) return to;
    return MorphVertex{from.x + (to.x - from.x) * morph_t,
                       from.y + (to.y - from.y) * morph_t, true};
}

// The line counterpart to viewport.h's blend_candle_snapshots: rewrites a fresh
// capture to start from the shape on screen, so a live tick restarting the
// morph doesn't snap the indicators back off the candles they sit on.
//
// Series are matched by key, so one enabled or removed between two ticks simply
// finds no counterpart and keeps its fresh capture. A slot only one side
// defines keeps that side, mirroring morph_vertex — the warmup gap can move by
// a slot as bars arrive, and sliding that end reads better than blinking it.
inline void blend_line_morphs(std::vector<LineMorph>& dst,
                              const std::vector<LineMorph>& interrupted,
                              float morph_t) {
    for (LineMorph& to : dst) {
        const LineMorph* from = find_line_morph(interrupted, to.key);
        if (!from) continue;
        to.scale = from->scale + (to.scale - from->scale) * morph_t;
        const std::size_t n =
            to.pts.size() < from->pts.size() ? to.pts.size() : from->pts.size();
        for (std::size_t k = 0; k < n; ++k) {
            const LineSnapshot& a = from->pts[k];
            LineSnapshot& b = to.pts[k];
            if (!a.valid || !b.valid) continue;
            b.x = a.x + (b.x - a.x) * morph_t;
            b.y = a.y + (b.y - a.y) * morph_t;
        }
    }
}

}  // namespace vroom
