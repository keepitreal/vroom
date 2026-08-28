#include "doctest.h"

#include <cmath>
#include <string>

#include "price_format.h"

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
    // takes over.
    CHECK(render_for(0.000044094) == "0.000044094");
    CHECK(render_for(0.023397) == "0.023397");
    CHECK(render_for(80285.20) == "80,285.20");
    CHECK(render_for(2513.92) == "2,513.92");
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
}

TEST_CASE("a reference that says nothing falls back to the floor") {
    // A chart with no data, or bounds that haven't been measured yet.
    CHECK(significant_decimals(0.0) == kPriceMinDecimals);
    CHECK(significant_decimals(std::nan("")) == kPriceMinDecimals);
    CHECK(significant_decimals(HUGE_VAL) == kPriceMinDecimals);
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
