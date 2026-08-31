// How a price is turned into the string on the y-axis.
//
// Precision follows the asset's own scale rather than the tick interval: a
// sub-cent token needs nine decimals to say anything at all, while a
// five-figure one is unreadable past two. Deriving it from the interval — as
// the axis used to — gets both wrong, printing "84000" for the one and
// "0.000060" for the other.
//
// The rule is a fixed count of significant digits with a floor, which is what
// reproduces a conventional price scale across the whole range:
//
//   4.4094e-5 -> 9 decimals   0.000044094
//   0.023397  -> 6 decimals   0.023397
//   80285.20  -> 2 decimals   80,285.20   (floor)
//   2513.92   -> 2 decimals   2,513.92    (floor)
//   1.59e6    -> compact      1.59M
//   3.219e7   -> compact      32.19M
//   3.454e10  -> compact      34.54B
//   1.2e12    -> compact      1.20T
//
// Past a million the integer digits eat the axis, so the format switches to a
// suffix (M / B / T) with two decimal places. The suffix is keyed off the
// asset's own price, not each tick, so every label on a market-cap chart
// shares a column and a zoomed-in 32.1M–32.2M range does not mix "32.15M"
// with "32,150,000.00".
//
// The reference is the asset's own price, not the visible range, so the
// precision holds steady while the user pans and zooms. A range-derived count
// would change under the user's finger and drag the axis width along with it.
//
// Skia-free and separately compiled so the unit tests can cover it; see
// tests/test_price_format.cpp.

#pragma once

#include <cstddef>

namespace vroom {

// Significant digits a price is quoted to. Five is what conventional price
// scales show: it keeps 0.023397 intact and still fits 0.000044094.
inline constexpr int kPriceSigDigits = 5;

// Prices at or above ~1 unit read as currency, where fewer than two decimals
// looks broken (a "$84,000" axis next to a "$80,285.20" last price). Compact
// suffixes use the same floor: "32.19M", not "32.2M".
inline constexpr int kPriceMinDecimals = 2;

// Past this the digits are noise, and %.*f starts printing the binary
// representation's tail rather than anything the feed meant.
inline constexpr int kPriceMaxDecimals = 12;

// Compact suffix for assets priced in the millions and up. Keyed off the
// reference, then applied to every label so the column stays uniform. Nothing
// below a million is abbreviated — no K.
enum class CompactScale {
    None,
    Million,    // |ref| >= 1e6
    Billion,    // |ref| >= 1e9
    Trillion,   // |ref| >= 1e12
};

// How to render a price. `group` inserts thousands separators, which only ever
// affects the integer part of a non-compact format. `compact` scales the value
// and appends M / B / T; grouping is skipped in that path.
struct PriceFormat {
    int          decimals = kPriceMinDecimals;
    bool         group = true;
    CompactScale compact = CompactScale::None;
};

// Decimals giving kPriceSigDigits significant digits for an asset priced around
// |reference|, clamped to [kPriceMinDecimals, kPriceMaxDecimals]. A zero or
// non-finite reference falls back to the floor.
int significant_decimals(double reference);

// The format for an asset priced around `reference`, including the compact
// suffix when |reference| is at least a million.
PriceFormat price_format_for(double reference);

// Decimal places needed so adjacent ticks `interval` apart don't collapse to
// the same string. 0.01 → 2, 0.005 → 3, 1e-8 → 8. Clamped to [0, 12].
int price_decimals(double interval);

// `fmt` raised, if need be, so ticks `interval` apart stay distinct. The asset's
// precision is the floor; only a zoom deeper than that precision adds to it.
// `interval` is in the data's units; when `fmt` is compact the guard converts
// it into display units (interval / 1e6 for millions, etc.) so a 1,000-wide
// step on a 32M chart still gets enough decimals to print as 32.192M vs 32.193M.
PriceFormat with_tick_guard(const PriceFormat& fmt, double interval);

// Writes `price` into `buf` per `fmt`, truncating rather than overrunning.
void format_price(char* buf, size_t buf_size, double price,
                  const PriceFormat& fmt);

}  // namespace vroom
