#include "viewport.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "ticks.h"

namespace vroom {

double to_scale(bool log, double price) {
    if (!log) return price;
    return std::log10(std::max(price, kMinLogPrice));
}

double from_scale(bool log, double v) {
    return log ? std::pow(10.0, v) : v;
}

float candle_body_width(const Layout& layout,
                        int64_t window_ms,
                        int64_t candle_duration_ms) {
    if (window_ms <= 0 || candle_duration_ms <= 0) return 0.f;
    const float usable = candle_area_width(layout);
    const double slot = static_cast<double>(usable) *
        (static_cast<double>(candle_duration_ms) /
         static_cast<double>(window_ms));
    return static_cast<float>(slot * layout.candle_width_ratio);
}

int64_t window_for_body_width(const Layout& layout,
                              int64_t candle_duration_ms,
                              double body_px) {
    const double usable = candle_area_width(layout);
    const double ratio = static_cast<double>(layout.candle_width_ratio);
    const double dur = static_cast<double>(candle_duration_ms);
    if (usable <= 0.0 || ratio <= 0.0 || dur <= 0.0 || body_px <= 0.0) return 0;
    return static_cast<int64_t>((usable * dur * ratio) / body_px);
}

float candle_center_x(const Layout& layout,
                      int64_t time_ms,
                      int64_t candle_duration_ms,
                      int64_t visible_start_ms,
                      int64_t window_ms) {
    if (window_ms <= 0) return 0.f;
    const float usable = candle_area_width(layout);
    const double center_time =
        static_cast<double>(time_ms) +
        static_cast<double>(candle_duration_ms) * 0.5;
    const double frac =
        (center_time - static_cast<double>(visible_start_ms)) /
        static_cast<double>(window_ms);
    return static_cast<float>(static_cast<double>(usable) * frac);
}

int64_t time_at_x(const Layout& layout,
                  int64_t visible_start_ms,
                  int64_t window_ms,
                  float x_px) {
    const float usable =
        layout.width_px - layout.y_axis_width_px - layout.right_padding_px;
    if (usable <= 0.f || window_ms <= 0) return visible_start_ms;
    const double frac = static_cast<double>(x_px) / static_cast<double>(usable);
    return visible_start_ms +
           static_cast<int64_t>(std::llround(frac * static_cast<double>(window_ms)));
}

float x_at_time(const Layout& layout,
                int64_t visible_start_ms,
                int64_t window_ms,
                int64_t time_ms) {
    if (window_ms <= 0) return 0.f;
    const float usable =
        layout.width_px - layout.y_axis_width_px - layout.right_padding_px;
    const double frac =
        static_cast<double>(time_ms - visible_start_ms) /
        static_cast<double>(window_ms);
    return static_cast<float>(static_cast<double>(usable) * frac);
}

size_t snap_index_to_candle(const Layout& layout,
                            const ::VroomCandle* candles,
                            size_t count,
                            int64_t candle_duration_ms,
                            int64_t visible_start_ms,
                            int64_t window_ms,
                            float x_px) {
    const float usable =
        layout.width_px - layout.y_axis_width_px - layout.right_padding_px;
    if (usable <= 0.f) return 0;

    // Pixel x -> the period start time of the candle that would sit there.
    const double target_center =
        static_cast<double>(visible_start_ms) +
        (static_cast<double>(x_px) / static_cast<double>(usable)) *
            static_cast<double>(window_ms);
    const int64_t key =
        static_cast<int64_t>(target_center) - candle_duration_ms / 2;

    // First candle with time_ms >= key, then pick whichever of it / its
    // predecessor is closer in time.
    auto* it = std::lower_bound(
        candles, candles + count, key,
        [](const ::VroomCandle& c, int64_t t) { return c.time_ms < t; });
    if (it == candles) return 0;
    if (it == candles + count) return count - 1;
    const int64_t hi = it->time_ms;
    const int64_t lo = (it - 1)->time_ms;
    return (hi - key < key - lo) ? static_cast<size_t>(it - candles)
                                 : static_cast<size_t>(it - 1 - candles);
}

SnapResult snap_to_slot(const Layout& layout,
                        const ::VroomCandle* candles,
                        size_t count,
                        int64_t candle_duration_ms,
                        int64_t visible_start_ms,
                        int64_t window_ms,
                        float x_px) {
    if (count == 0) return {visible_start_ms, false, 0};

    const float usable =
        layout.width_px - layout.y_axis_width_px - layout.right_padding_px;
    if (usable <= 0.f || window_ms <= 0 || candle_duration_ms <= 0) {
        return {candles[0].time_ms, true, 0};
    }

    // Clamp to the usable candle width so snapping never runs past the visible
    // right edge into off-screen space.
    const float clamped_x = std::clamp(x_px, 0.f, usable);

    // Pixel x -> the period start time of the candle that would sit there.
    const double target_center =
        static_cast<double>(visible_start_ms) +
        (static_cast<double>(clamped_x) / static_cast<double>(usable)) *
            static_cast<double>(window_ms);
    const int64_t key =
        static_cast<int64_t>(target_center) - candle_duration_ms / 2;

    const int64_t last_time = candles[count - 1].time_ms;
    if (key > last_time) {
        // Past the last candle: snap to the nearest candle-aligned grid slot.
        // Within half a period of the last candle we still land on it (steps
        // 0); beyond that we snap to an empty future slot with no candle data.
        const double k = static_cast<double>(key - last_time) /
                         static_cast<double>(candle_duration_ms);
        const int64_t steps = std::llround(k);
        if (steps <= 0) return {last_time, true, count - 1};
        return {last_time + steps * candle_duration_ms, false, 0};
    }

    // Populated region: pick whichever of the lower_bound candle / its
    // predecessor is closer in time.
    auto* it = std::lower_bound(
        candles, candles + count, key,
        [](const ::VroomCandle& c, int64_t t) { return c.time_ms < t; });
    if (it == candles) return {candles[0].time_ms, true, 0};
    if (it == candles + count) {
        return {candles[count - 1].time_ms, true, count - 1};
    }
    const int64_t hi = it->time_ms;
    const int64_t lo = (it - 1)->time_ms;
    const size_t idx = (hi - key < key - lo)
                           ? static_cast<size_t>(it - candles)
                           : static_cast<size_t>(it - 1 - candles);
    return {candles[idx].time_ms, true, idx};
}

float snap_x_to_candle(const Layout& layout,
                       const ::VroomCandle* candles,
                       size_t count,
                       int64_t candle_duration_ms,
                       int64_t visible_start_ms,
                       int64_t window_ms,
                       float x_px) {
    if (count == 0 || window_ms <= 0) return x_px;

    const float usable =
        layout.width_px - layout.y_axis_width_px - layout.right_padding_px;
    if (usable <= 0.f) return x_px;

    const SnapResult snap = snap_to_slot(
        layout, candles, count, candle_duration_ms, visible_start_ms,
        window_ms, x_px);
    return candle_center_x(layout, snap.time_ms, candle_duration_ms,
                           visible_start_ms, window_ms);
}

IndexRange visible_indices(const ::VroomCandle* candles,
                           size_t count,
                           int64_t start_ms,
                           int64_t end_ms) {
    if (count == 0) return {0, 0};
    if (start_ms == 0 && end_ms == 0) return {0, count};

    // first index with time_ms >= start_ms
    auto* first = std::lower_bound(
        candles, candles + count, start_ms,
        [](const ::VroomCandle& c, int64_t t) { return c.time_ms < t; });
    // first index with time_ms > end_ms
    auto* last = std::upper_bound(
        candles, candles + count, end_ms,
        [](int64_t t, const ::VroomCandle& c) { return t < c.time_ms; });
    return {
        static_cast<size_t>(first - candles),
        static_cast<size_t>(last - candles),
    };
}

PriceBounds price_bounds(const ::VroomCandle* candles, size_t count) {
    PriceBounds b{
        std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(),
    };
    for (size_t i = 0; i < count; ++i) {
        b.min = std::min(b.min, candles[i].low);
        b.max = std::max(b.max, candles[i].high);
    }
    if (count == 0) {
        b.min = 0.0;
        b.max = 1.0;
    }
    return b;
}

PriceBounds auto_price_bounds(const ::VroomCandle* candles, size_t count,
                              bool log) {
    PriceBounds b = price_bounds(candles, count);
    b.log = log;
    if (count == 0) return b;
    const double lo = to_scale(log, b.min);
    const double hi = to_scale(log, b.max);
    const double mid = (lo + hi) * 0.5;
    const double half = (hi - lo) * 0.5 * kAutoYZoom;
    return {from_scale(log, mid - half), from_scale(log, mid + half), log};
}

PriceBounds preserve_envelope_bounds(const PriceBounds& old_axis,
                                     const PriceBounds& old_env,
                                     const PriceBounds& new_env) {
    const bool log = old_axis.log;
    const double axis_lo = to_scale(log, old_axis.min);
    const double axis_range = to_scale(log, old_axis.max) - axis_lo;
    const double old_lo = to_scale(log, old_env.min);
    const double old_hi = to_scale(log, old_env.max);
    const double new_lo = to_scale(log, new_env.min);
    const double new_hi = to_scale(log, new_env.max);
    const double old_span = old_hi - old_lo;
    const double new_span = new_hi - new_lo;
    if (axis_range <= 0.0 || old_span <= 0.0 || new_span <= 0.0) return old_axis;

    // Same span/range ratio => same pixel height for the envelope.
    const double new_range = axis_range * (new_span / old_span);
    // Fraction of the axis (from the bottom) where the old envelope's midpoint
    // sat; putting the new midpoint at the same fraction pins the envelope box.
    const double t = ((old_lo + old_hi) * 0.5 - axis_lo) / axis_range;
    const double new_mid = (new_lo + new_hi) * 0.5;
    return {from_scale(log, new_mid - t * new_range),
            from_scale(log, new_mid + (1.0 - t) * new_range), log};
}

double price_fraction(const PriceBounds& bounds, double price) {
    const double lo = to_scale(bounds.log, bounds.min);
    const double range = to_scale(bounds.log, bounds.max) - lo;
    // Degenerate range: everything sits at the band midpoint, which keeps
    // price_to_y's flat-series fallback intact.
    if (range <= 0.0) return 0.5;
    return (to_scale(bounds.log, price) - lo) / range;  // 0 at min, 1 at max
}

double price_at_fraction(const PriceBounds& bounds, double frac) {
    const double lo = to_scale(bounds.log, bounds.min);
    const double hi = to_scale(bounds.log, bounds.max);
    return from_scale(bounds.log, lo + frac * (hi - lo));
}

PriceBounds shift_scaled(const PriceBounds& b, double frac) {
    const double lo = to_scale(b.log, b.min);
    const double hi = to_scale(b.log, b.max);
    const double d = frac * (hi - lo);
    return {from_scale(b.log, lo + d), from_scale(b.log, hi + d), b.log};
}

PriceBounds rescale_scaled(const PriceBounds& b, double scale, double anchor) {
    const double lo = to_scale(b.log, b.min);
    const double hi = to_scale(b.log, b.max);
    const double pivot = lo + anchor * (hi - lo);
    return {from_scale(b.log, pivot - anchor * (hi - lo) * scale),
            from_scale(b.log, pivot + (1.0 - anchor) * (hi - lo) * scale),
            b.log};
}

void price_ticks(const PriceBounds& bounds, float pane_h, int max_count,
                 std::vector<double>& out) {
    out.clear();
    if (!(bounds.max > bounds.min) || pane_h <= 0.f || max_count <= 0) return;

    if (!bounds.log) {
        const double interval =
            pick_price_interval(bounds.max - bounds.min, pane_h);
        if (interval <= 0.0) return;
        const double first = std::ceil(bounds.min / interval) * interval;
        int n = 0;
        for (double price = first; price <= bounds.max && n < max_count;
             price += interval, ++n) {
            out.push_back(price);
        }
        return;
    }

    const double lo = std::max(bounds.min, kMinLogPrice);
    const double hi = bounds.max;
    if (!(hi > lo)) return;
    // d(price)/d(band fraction) at price p is p × ln(hi/lo); feeding that to
    // the linear picker gives the interval a linear band of the same local
    // density would use.
    const double ln_span = std::log(hi / lo);
    const double px_per_ln = static_cast<double>(pane_h) / ln_span;
    const auto local_interval = [&](double p) {
        return pick_price_interval(p * ln_span, pane_h);
    };

    double interval = local_interval(hi);
    double price = std::floor(hi / interval) * interval;
    double prev_px = std::numeric_limits<double>::infinity();
    while (price >= lo && price > 0.0 &&
           static_cast<int>(out.size()) < max_count) {
        const double px = std::log(price / lo) * px_per_ln;
        if (prev_px - px >= kLogTickMinGapPx) {
            out.push_back(price);
            prev_px = px;
        }
        interval = local_interval(price);
        if (!(interval > 0.0)) break;
        // Largest multiple of the (possibly finer) interval strictly below.
        double k = std::floor(price / interval + 1e-9);
        if (k * interval >= price - interval * 1e-9) k -= 1.0;
        price = k * interval;
    }
}

double price_label_interval(const PriceBounds& bounds, float pane_h) {
    if (!bounds.log) return pick_price_interval(bounds.max - bounds.min, pane_h);
    const double lo = std::max(bounds.min, kMinLogPrice);
    if (!(bounds.max > lo)) return pick_price_interval(0.0, pane_h);
    return pick_price_interval(lo * std::log(bounds.max / lo), pane_h);
}

float y_at_fraction(const Layout& layout, double frac) {
    // The candle drawing area is the full height minus the x-axis strip and
    // any below-chart indicator pane.
    const float candle_area_h = price_pane_bottom(layout);
    const float top = candle_area_h * layout.top_padding_frac;
    const float bot = candle_area_h * (1.f - layout.bottom_padding_frac);
    return bot - static_cast<float>(frac) * (bot - top);  // invert: high → low y
}

float price_to_y(const Layout& layout,
                 const PriceBounds& bounds,
                 double price) {
    return y_at_fraction(layout, price_fraction(bounds, price));
}

double y_to_price(const Layout& layout,
                  const PriceBounds& bounds,
                  float y) {
    const float candle_area_h = price_pane_bottom(layout);
    const float top = candle_area_h * layout.top_padding_frac;
    const float bot = candle_area_h * (1.f - layout.bottom_padding_frac);
    const float draw_h = bot - top;
    if (draw_h <= 0.f || !(bounds.max > bounds.min)) return bounds.min;
    const double t = (bot - y) / draw_h;  // 0 at min (bottom), 1 at top
    return price_at_fraction(bounds, t);
}

TimeWindow clamp_shifted_time_window(int64_t start_ms, int64_t end_ms,
                                     int64_t first_time, int64_t last_time,
                                     int64_t candle_duration_ms,
                                     int64_t max_future) {
    const int64_t window = end_ms - start_ms;
    if (window <= 0) return {start_ms, end_ms};

    const int64_t dur = candle_duration_ms > 0 ? candle_duration_ms : 0;
    const int64_t last_slot_end = last_time + dur;
    const int64_t data_extent = last_slot_end - first_time;

    if (max_future >= 0 && end_ms > last_time + max_future) {
        end_ms = last_time + max_future;
        start_ms = end_ms - window;
    }

    // Window longer than the series: empty past is how width wins. Floor the
    // right edge at last_time so a pan into the past is a no-op when the
    // candles are already right-aligned. Do not pin start to first_time — that
    // throws the extra length into the future and the two caps latch.
    if (data_extent >= 0 && window > data_extent) {
        if (end_ms < last_time) {
            end_ms = last_time;
            start_ms = end_ms - window;
        }
        return {start_ms, end_ms};
    }

    if (start_ms < first_time) {
        start_ms = first_time;
        end_ms = start_ms + window;
    }
    return {start_ms, end_ms};
}

}  // namespace vroom
