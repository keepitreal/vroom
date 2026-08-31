#include "doctest.h"

#include <cmath>
#include <string>

#include "price_format.h"

using vroom::CompactScale;
using vroom::format_price;
using vroom::kPriceMaxDecimals;
using vroom::kPriceMinDecimals;
using vroom::price_decimals;
using vroom::price_format_for;
using vroom::PriceFormat;
using vroom::significant_decimals;
using vroom::with_tick_guard;

namespace {

std::string render(double price, const PriceFormat& fmt) {
    char buf[48];
    format_price(buf, sizeof(buf), price, fmt);
    return std::string(buf);
}

std::string render_for(double price) {
    return render(price, price_format_for(price));
}

}  // namespace

TEST_CASE("a price renders at the precision its own scale calls for") {
    // The four cases the axis has to span, from a sub-cent token to a
    // five-figure one. Each keeps five significant digits until the floor
    // takes over. Compact suffixes start at a million and are covered below.
    CHECK(render_for(0.000044094) == "0.000044094");
    CHECK(render_for(0.023397) == "0.023397");
    CHECK(render_for(80285.20) == "80,285.20");
    CHECK(render_for(2513.92) == "2,513.92");
    CHECK(render_for(999999.0) == "999,999.00");
}

TEST_CASE("decimals track the leading digit's exponent") {
    CHECK(significant_decimals(4.4094e-5) == 9);
    CHECK(significant_decimals(0.023397) == 6);
    CHECK(significant_decimals(0.5) == 5);
    CHECK(significant_decimals(1.0) == 4);
    CHECK(significant_decimals(9.9999) == 4);
    CHECK(significant_decimals(10.0) == 3);
    CHECK(significant_decimals(100.0) == 2);
}

TEST_CASE("the floor holds for anything priced like currency") {
    // Past three integer digits the significant-digit count would ask for
    // fewer than two decimals, which reads as broken on a price axis.
    for (double p : {1000.0, 2513.92, 80285.2, 1.0e9}) {
        CHECK(significant_decimals(p) == kPriceMinDecimals);
    }
}

TEST_CASE("the cap holds for prices below the digits we'd trust") {
    CHECK(significant_decimals(1e-9) == kPriceMaxDecimals);
    CHECK(significant_decimals(1e-30) == kPriceMaxDecimals);
    CHECK(render(1e-9, price_format_for(1e-9)) == "0.000000001000");
}

TEST_CASE("magnitude is what matters, not sign") {
    CHECK(significant_decimals(-0.023397) == significant_decimals(0.023397));
    CHECK(render_for(-2513.92) == "-2,513.92");
    CHECK(render_for(-0.000044094) == "-0.000044094");
    CHECK(price_format_for(-1.59e6).compact == CompactScale::Million);
    CHECK(render_for(-1.59e6) == "-1.59M");
}

TEST_CASE("a reference that says nothing falls back to the floor") {
    // A chart with no data, or bounds that haven't been measured yet.
    CHECK(significant_decimals(0.0) == kPriceMinDecimals);
    CHECK(significant_decimals(std::nan("")) == kPriceMinDecimals);
    CHECK(significant_decimals(HUGE_VAL) == kPriceMinDecimals);
    CHECK(price_format_for(0.0).compact == CompactScale::None);
    CHECK(price_format_for(std::nan("")).compact == CompactScale::None);
    CHECK(price_format_for(HUGE_VAL).compact == CompactScale::None);
}

TEST_CASE("separators land every three digits and never in the fraction") {
    const PriceFormat two{2, true};
    CHECK(render(999.0, two) == "999.00");
    CHECK(render(1000.0, two) == "1,000.00");
    CHECK(render(999999.0, two) == "999,999.00");
    CHECK(render(1000000.0, two) == "1,000,000.00");
    CHECK(render(1234567890.0, two) == "1,234,567,890.00");

    // Nine decimals is a lot of trailing digits to walk past.
    CHECK(render(0.000044094, PriceFormat{9, true}) == "0.000044094");
}

TEST_CASE("grouping can be turned off") {
    CHECK(render(1234567.0, PriceFormat{2, false}) == "1234567.00");
    CHECK(render(1234567.0, PriceFormat{0, false}) == "1234567");
}

TEST_CASE("zero decimals leaves no trailing point") {
    CHECK(render(1000.0, PriceFormat{0, true}) == "1,000");
    CHECK(render(100.0, PriceFormat{0, true}) == "100");
}

TEST_CASE("the tick guard only ever raises the precision") {
    const PriceFormat asset{2, true};  // a five-figure asset

    SUBCASE("a coarse interval leaves the asset's own precision alone") {
        // The old interval-derived rule gave 0 decimals here, which is exactly
        // the "84000" bug.
        CHECK(with_tick_guard(asset, 2000.0).decimals == 2);
        CHECK(with_tick_guard(asset, 25.0).decimals == 2);
    }

    SUBCASE("zooming past the asset's precision adds decimals") {
        CHECK(with_tick_guard(asset, 0.001).decimals == 3);
        CHECK(with_tick_guard(asset, 1e-5).decimals == 5);
    }

    SUBCASE("grouping carries through") {
        CHECK(with_tick_guard(asset, 0.001).group == true);
        CHECK(with_tick_guard(PriceFormat{2, false}, 0.001).group == false);
    }

    SUBCASE("compact scale carries through") {
        const PriceFormat millions{2, true, CompactScale::Million};
        CHECK(with_tick_guard(millions, 5e6).compact == CompactScale::Million);
    }
}

TEST_CASE("millions and up render with a compact suffix") {
    CHECK(price_format_for(1.59e6).compact == CompactScale::Million);
    CHECK(price_format_for(3.219e7).compact == CompactScale::Million);
    CHECK(price_format_for(3.454e10).compact == CompactScale::Billion);
    CHECK(price_format_for(1.2e12).compact == CompactScale::Trillion);

    CHECK(render_for(1.59e6) == "1.59M");
    CHECK(render_for(3.219238138e7) == "32.19M");
    CHECK(render_for(3.454e10) == "34.54B");
    CHECK(render_for(3.4554e11) == "345.54B");
    CHECK(render_for(1.2e12) == "1.20T");

    // The suffix is keyed off the reference, so a tick below a million on a
    // market-cap chart still prints in millions and the column stays uniform.
    const PriceFormat millions = price_format_for(3.2e7);
    CHECK(render(8e5, millions) == "0.80M");

    SUBCASE("an explicit unscaled format still prints the full number") {
        // Call sites that build PriceFormat by hand (and the grouping tests
        // above) opt out of compact by leaving the scale at None.
        CHECK(render(1e6, PriceFormat{2, true}) == "1,000,000.00");
        CHECK(render(1.23456789e9, PriceFormat{2, true}) == "1,234,567,890.00");
    }
}

TEST_CASE("the tick guard runs in compact display units") {
    const PriceFormat millions{2, true, CompactScale::Million};

    SUBCASE("a coarse interval leaves the two-decimal floor alone") {
        // 5M in data units is 5.0 in millions — no extra decimals.
        CHECK(with_tick_guard(millions, 5e6).decimals == 2);
        CHECK(render(3.0e7, with_tick_guard(millions, 5e6)) == "30.00M");
    }

    SUBCASE("a 1,000-wide step on a 32M chart needs three decimals") {
        // 1000 / 1e6 = 0.001, so 32.192M vs 32.193M stay distinct. Without
        // converting into display units this would keep 2 decimals and every
        // nearby tick would collapse to 32.19M.
        CHECK(with_tick_guard(millions, 1000.0).decimals == 3);
        CHECK(render(3.2192e7, with_tick_guard(millions, 1000.0)) == "32.192M");
    }

    SUBCASE("an unscaled format still measures the interval in data units") {
        CHECK(with_tick_guard(PriceFormat{2, true}, 1000.0).decimals == 2);
    }
}

TEST_CASE("price_decimals keeps adjacent ticks distinct") {
    CHECK(price_decimals(1.0) == 0);
    CHECK(price_decimals(5.0) == 0);
    CHECK(price_decimals(0.01) == 2);
    CHECK(price_decimals(0.005) == 3);
    CHECK(price_decimals(1e-8) == 8);

    SUBCASE("a degenerate interval falls back to the floor") {
        CHECK(price_decimals(0.0) == kPriceMinDecimals);
        CHECK(price_decimals(-1.0) == kPriceMinDecimals);
    }
}

TEST_CASE("a buffer too small truncates instead of overrunning") {
    char buf[8];
    buf[7] = '\xff';
    format_price(buf, sizeof(buf), 1234567890.0, PriceFormat{2, true});
    CHECK(std::string(buf).size() < sizeof(buf));
    CHECK(buf[7] == '\0');
}
