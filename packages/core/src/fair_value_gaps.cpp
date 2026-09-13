#include "fair_value_gaps.h"

#include <algorithm>

namespace vroom::fvg {

namespace {

// True once `c` has traded back through the far edge of the gap — the bottom of
// a bullish gap, the top of a bearish one. Under kFillClose the candle has to
// close past it; under kFillWick the wick reaching it is enough.
bool fills(const ::VroomCandle& c, const Gap& g, int fill_type) {
    if (fill_type == kFillWick) {
        return g.bullish ? c.low <= g.bottom : c.high >= g.top;
    }
    return g.bullish ? c.close <= g.bottom : c.close >= g.top;
}

}  // namespace

void compute(const ::VroomCandle* candles, std::size_t n, int max_bars_back,
             bool wait_for_close, int fill_type, std::vector<Gap>& out) {
    out.clear();
    if (!candles || n < 3 || max_bars_back <= 0) return;

    // The middle bar of the three. It needs a bar on each side, and under
    // wait_for_close the third one must not be the still-forming newest bar.
    const std::size_t last = wait_for_close ? n - 3 : n - 2;
    const std::size_t window = static_cast<std::size_t>(max_bars_back);
    const std::size_t first = std::max<std::size_t>(1, n > window ? n - window : 0);

    for (std::size_t i = first; i <= last; ++i) {
        const ::VroomCandle& prev = candles[i - 1];
        const ::VroomCandle& next = candles[i + 1];

        Gap g;
        if (prev.high < next.low) {
            g.bullish = true;
            g.bottom = prev.high;
            g.top = next.low;
        } else if (prev.low > next.high) {
            g.bullish = false;
            g.bottom = next.high;
            g.top = prev.low;
        } else {
            continue;  // the wicks overlap — price left nothing behind
        }
        g.time_ms = candles[i].time_ms;

        for (std::size_t j = i + 2; j < n; ++j) {
            if (fills(candles[j], g, fill_type)) {
                g.filled_ms = candles[j].time_ms;
                break;
            }
        }
        out.push_back(g);
    }
}

}  // namespace vroom::fvg
