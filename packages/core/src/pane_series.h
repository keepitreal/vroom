// Polyline plotting shared by the indicator panes (RSI, MACD, ATR), including
// the interval morph.
//
// Each pane maps values into its own band with its own auto-fit, so it hands
// over its y mapping and everything from the slot pairing down is identical
// across the three. The capture holds band *fractions* rather than values,
// which is what lets a pane re-fit across a timeframe switch without its
// outgoing shape moving.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#pragma clang diagnostic pop

#include "line_morph.h"
#include "viewport.h"
#include "vroom/vroom_chart.h"

namespace vroom::pane_series {

// Slot `k` of a pane series in screen pixels, blended from the capture toward
// the new value. Slot 0 is the rightmost vertex on both sides — the pairing a
// timeframe switch preserves, the same one candles::draw uses.
template <typename YFor>
MorphVertex vertex(const Layout& lay,
                   const ::VroomCandle* visible,
                   std::size_t n,
                   const double* values,
                   int64_t window_ms,
                   int64_t visible_start_ms,
                   int64_t candle_duration_ms,
                   float pane_top,
                   float pane_bottom,
                   const LineMorph* from,
                   std::size_t from_count,
                   float morph_t,
                   std::size_t k,
                   YFor&& y_for) {
    MorphVertex to;
    if (values && k < n) {
        const std::size_t i = n - 1 - k;
        const double v = values[i];
        if (std::isfinite(v)) {
            to = MorphVertex{
                vroom::candle_center_x(lay, visible[i].time_ms,
                                       candle_duration_ms, visible_start_ms,
                                       window_ms),
                y_for(v), true};
        }
    }
    MorphVertex frm;
    if (k < from_count) {
        const LineSnapshot& s = from->pts[k];
        if (s.valid) {
            frm = MorphVertex{s.x * vroom::candle_area_width(lay),
                              pane_bottom - s.y * (pane_bottom - pane_top),
                              true};
        }
    }
    return vroom::morph_vertex(to, frm, morph_t);
}

// How many vertices a morphing series spans: the new slice and the capture can
// disagree on length, and every slot either defines has to be walked.
inline std::size_t slots(std::size_t n, const LineMorph* from, float morph_t) {
    return std::max(n, vroom::morph_line_count(from, morph_t));
}

// The series as a polyline, walked left to right. A slot neither side defines
// lifts the pen, which is what leaves the warmup gap open.
template <typename YFor>
SkPath build_path(const Layout& lay,
                  const ::VroomCandle* visible,
                  std::size_t n,
                  const double* values,
                  int64_t window_ms,
                  int64_t visible_start_ms,
                  int64_t candle_duration_ms,
                  float pane_top,
                  float pane_bottom,
                  const LineMorph* from,
                  float morph_t,
                  YFor&& y_for) {
    const std::size_t from_count = vroom::morph_line_count(from, morph_t);
    const std::size_t count = std::max(n, from_count);
    SkPathBuilder path;
    bool pen_down = false;
    for (std::size_t j = 0; j < count; ++j) {
        const MorphVertex p =
            vertex(lay, visible, n, values, window_ms, visible_start_ms,
                   candle_duration_ms, pane_top, pane_bottom, from, from_count,
                   morph_t, count - 1 - j, y_for);
        if (!p.valid) {
            pen_down = false;
            continue;
        }
        if (pen_down) {
            path.lineTo(p.x, p.y);
        } else {
            path.moveTo(p.x, p.y);
            pen_down = true;
        }
    }
    return path.detach();
}

// The region between the series and a horizontal rule at `y_band`, as the
// closed counterpart to build_path. Callers clip to one side of the rule to keep
// only the stretches that reach past it, which puts the crossings exactly where
// Skia cuts the geometry rather than anywhere this has to solve for.
//
// Each run of vertices gets its own contour, tied down to the rule at both
// ends. One contour spanning the whole series would close a warmup gap by
// running straight through it, filling a span the line never drew.
template <typename YFor>
SkPath build_band_area(const Layout& lay,
                       const ::VroomCandle* visible,
                       std::size_t n,
                       const double* values,
                       int64_t window_ms,
                       int64_t visible_start_ms,
                       int64_t candle_duration_ms,
                       float pane_top,
                       float pane_bottom,
                       const LineMorph* from,
                       float morph_t,
                       float y_band,
                       YFor&& y_for) {
    const std::size_t from_count = vroom::morph_line_count(from, morph_t);
    const std::size_t count = std::max(n, from_count);
    SkPathBuilder path;
    bool open = false;
    float last_x = 0.f;
    for (std::size_t j = 0; j < count; ++j) {
        const MorphVertex p =
            vertex(lay, visible, n, values, window_ms, visible_start_ms,
                   candle_duration_ms, pane_top, pane_bottom, from, from_count,
                   morph_t, count - 1 - j, y_for);
        if (!p.valid) {
            if (open) {
                path.lineTo(last_x, y_band);
                path.close();
                open = false;
            }
            continue;
        }
        if (!open) {
            path.moveTo(p.x, y_band);
            open = true;
        }
        path.lineTo(p.x, p.y);
        last_x = p.x;
    }
    if (open) {
        path.lineTo(last_x, y_band);
        path.close();
    }
    return path.detach();
}

}  // namespace vroom::pane_series
