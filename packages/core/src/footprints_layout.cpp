#include "footprints_layout.h"

#include <algorithm>
#include <cmath>

namespace vroom::footprints {

int32_t bucket_index(const ::VroomCandle* candles, size_t count,
                     int64_t duration_ms, int64_t time_ms) {
    if (!candles || count == 0 || duration_ms <= 0) return -1;

    // Last candle that opened at or before the trade.
    const ::VroomCandle* it =
        std::upper_bound(candles, candles + count, time_ms,
                         [](int64_t t, const ::VroomCandle& c) { return t < c.time_ms; });
    if (it == candles) return -1;  // before the first bar
    const int32_t i = static_cast<int32_t>((it - 1) - candles);

    // Inside that bar's window, or in a gap where no bar exists.
    if (time_ms >= candles[i].time_ms + duration_ms) return -1;
    return i;
}

std::vector<Bucket> build_buckets(const ::VroomCandle* candles, size_t count,
                                  int64_t duration_ms,
                                  const ::VroomFootprint* prints, size_t print_count) {
    std::vector<Bucket> out;
    if (!candles || count == 0 || !prints || print_count == 0) return out;

    // Bucket per candle index, then compact. Candle counts stay in the low
    // thousands, so a flat scratch vector beats a map on both sides of the ledger.
    std::vector<int32_t> slot(count, -1);

    for (size_t i = 0; i < print_count; ++i) {
        const ::VroomFootprint& p = prints[i];
        if (p.side != VROOM_FOOTPRINT_BUY && p.side != VROOM_FOOTPRINT_SELL) continue;

        const int32_t ci = bucket_index(candles, count, duration_ms, p.time_ms);
        if (ci < 0) continue;

        if (slot[static_cast<size_t>(ci)] < 0) {
            slot[static_cast<size_t>(ci)] = static_cast<int32_t>(out.size());
            Bucket b;
            b.candle_time_ms = candles[ci].time_ms;
            out.push_back(std::move(b));
        }
        Bucket& b = out[static_cast<size_t>(slot[static_cast<size_t>(ci)])];
        if (p.side == VROOM_FOOTPRINT_BUY) {
            b.buys.push_back(static_cast<int32_t>(i));
        } else {
            b.sells.push_back(static_cast<int32_t>(i));
        }
    }

    // The host's array can arrive in any order, so sort each side by time and read
    // the latest off the end.
    const auto by_time = [prints](int32_t a, int32_t b) {
        const int64_t ta = prints[static_cast<size_t>(a)].time_ms;
        const int64_t tb = prints[static_cast<size_t>(b)].time_ms;
        // Fall back to the host's order so equal timestamps stay stable.
        return ta != tb ? ta < tb : a < b;
    };
    for (Bucket& b : out) {
        std::sort(b.buys.begin(), b.buys.end(), by_time);
        std::sort(b.sells.begin(), b.sells.end(), by_time);
        if (!b.buys.empty()) {
            b.buy_latest_ms = prints[static_cast<size_t>(b.buys.back())].time_ms;
        }
        if (!b.sells.empty()) {
            b.sell_latest_ms = prints[static_cast<size_t>(b.sells.back())].time_ms;
        }
    }

    // Buckets were created in first-seen order; callers want them by candle.
    std::sort(out.begin(), out.end(), [](const Bucket& a, const Bucket& b) {
        return a.candle_time_ms < b.candle_time_ms;
    });
    return out;
}

Metrics metrics_from(const ::VroomFootprintStyle* style) {
    Metrics m;
    if (!style) return m;
    if (style->radius_px > 0.f) m.radius = style->radius_px;
    if (style->gap_px > 0.f) m.gap = style->gap_px;
    if (style->margin_px > 0.f) m.margin = style->margin_px;
    if (style->hover_boost > 0.f) m.hover_boost = style->hover_boost;
    return m;
}

Stack layout_stack(const Bucket& bucket, float high_y, float pane_top,
                   float radius, float gap, float margin) {
    Stack out;
    if (radius <= 0.f) radius = kRadius;
    if (gap <= 0.f) gap = kGap;
    if (margin <= 0.f) margin = kMargin;

    const bool buys = bucket.has_buys();
    const bool sells = bucket.has_sells();
    if (!buys && !sells) return out;

    if (buys && sells) {
        // Bottom-to-top replays the order the trades happened in. Ties go to the
        // buy, so an entry and exit stamped the same ms still lay out predictably.
        const bool buy_first = bucket.buy_latest_ms <= bucket.sell_latest_ms;
        out.sides[0] = buy_first ? VROOM_FOOTPRINT_BUY : VROOM_FOOTPRINT_SELL;
        out.sides[1] = buy_first ? VROOM_FOOTPRINT_SELL : VROOM_FOOTPRINT_BUY;
        out.count = 2;
    } else {
        out.sides[0] = buys ? VROOM_FOOTPRINT_BUY : VROOM_FOOTPRINT_SELL;
        out.count = 1;
    }

    // Slot 0 clears the high by `margin`; each further slot clears the one below
    // it by `gap`, which is what keeps them separately hoverable.
    const float pitch = 2.f * radius + gap;
    for (int32_t i = 0; i < out.count; ++i) {
        out.y[i] = high_y - margin - radius - static_cast<float>(i) * pitch;
    }

    // A bar near the top of the view would push its stack off-screen; slide the
    // whole stack down so the topmost badge stays inside the pane. Moving both
    // together preserves the order the stack encodes.
    const float top_edge = out.y[out.count - 1] - radius;
    if (top_edge < pane_top) {
        const float shift = pane_top - top_edge;
        for (int32_t i = 0; i < out.count; ++i) out.y[i] += shift;
    }
    return out;
}

bool hits_badge(float cx, float cy, float radius, float x, float y) {
    if (radius <= 0.f) return false;
    const float dx = x - cx;
    const float dy = y - cy;
    const float r = radius + kHitTolerance;
    return dx * dx + dy * dy <= r * r;
}

}  // namespace vroom::footprints
