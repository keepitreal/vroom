#include "doctest.h"

#include <cmath>
#include <limits>
#include <vector>

#include "ma.h"
#include "rsi.h"

namespace {
// Build candles from a close series (open/high/low/volume don't affect RSI).
std::vector<VroomCandle> closes(std::initializer_list<double> cs) {
    std::vector<VroomCandle> out;
    int64_t t = 0;
    for (double c : cs) {
        out.push_back(VroomCandle{t, c, c, c, c, 0.0});
        t += 60'000;
    }
    return out;
}
}  // namespace

TEST_CASE("rsi::compute") {
    std::vector<double> out;

    SUBCASE("undefined before the first value and with too little data") {
        auto c = closes({10, 11, 10, 11});
        vroom::rsi::compute(c.data(), c.size(), 2, out);
        REQUIRE(out.size() == 4);
        CHECK(std::isnan(out[0]));  // i < period
        CHECK(std::isnan(out[1]));  // i < period
        CHECK_FALSE(std::isnan(out[2]));

        // n <= period → all NaN.
        std::vector<double> few;
        auto c2 = closes({10, 11});
        vroom::rsi::compute(c2.data(), c2.size(), 2, few);
        REQUIRE(few.size() == 2);
        CHECK(std::isnan(few[0]));
        CHECK(std::isnan(few[1]));
    }

    SUBCASE("hand-computed values (period 2)") {
        // deltas +1,-1,+1. seed@2: avgGain=0.5 avgLoss=0.5 → RS=1 → 50.
        // @3: avgGain=(0.5+1)/2=0.75 avgLoss=(0.5+0)/2=0.25 → RS=3 → 75.
        auto c = closes({10, 11, 10, 11});
        vroom::rsi::compute(c.data(), c.size(), 2, out);
        CHECK(out[2] == doctest::Approx(50.0));
        CHECK(out[3] == doctest::Approx(75.0));
    }

    SUBCASE("all gains → 100") {
        auto c = closes({1, 2, 3, 4, 5});
        vroom::rsi::compute(c.data(), c.size(), 2, out);
        CHECK(out[2] == doctest::Approx(100.0));
        CHECK(out[4] == doctest::Approx(100.0));
    }

    SUBCASE("all losses → 0") {
        auto c = closes({5, 4, 3, 2, 1});
        vroom::rsi::compute(c.data(), c.size(), 2, out);
        CHECK(out[2] == doctest::Approx(0.0));
        CHECK(out[4] == doctest::Approx(0.0));
    }

    SUBCASE("period must be >= 2") {
        auto c = closes({1, 2, 3});
        vroom::rsi::compute(c.data(), c.size(), 1, out);
        REQUIRE(out.size() == 3);
        for (double v : out) CHECK(std::isnan(v));
    }

    SUBCASE("a window that never moved is undefined, not 100") {
        auto c = closes({5, 5, 5, 5, 5});
        vroom::rsi::compute(c.data(), c.size(), 2, out);
        REQUIRE(out.size() == 5);
        for (double v : out) CHECK(std::isnan(v));
    }

    SUBCASE("a flat stretch becomes defined once a close moves") {
        // seed@2: deltas 0,0 → undefined. @3: avgGain=(0+1)/2 avgLoss=0 → 100.
        auto c = closes({5, 5, 5, 6});
        vroom::rsi::compute(c.data(), c.size(), 2, out);
        CHECK(std::isnan(out[2]));
        CHECK(out[3] == doctest::Approx(100.0));
    }

    SUBCASE("leading zero-price placeholders are skipped, not rallied from") {
        // The priced run starts at index 3 and matches the hand-computed case
        // shifted by 3: seed 50 at 3 + period, then 75.
        auto c = closes({0, 0, 0, 10, 11, 10, 11});
        vroom::rsi::compute(c.data(), c.size(), 2, out);
        REQUIRE(out.size() == 7);
        for (std::size_t i = 0; i < 5; ++i) CHECK(std::isnan(out[i]));
        CHECK(out[5] == doctest::Approx(50.0));
        CHECK(out[6] == doctest::Approx(75.0));
    }

    SUBCASE("a placeholder mid-series ends the run and the next one re-seeds") {
        auto c = closes({10, 11, 10, 0, 10, 11, 10, 11});
        vroom::rsi::compute(c.data(), c.size(), 2, out);
        REQUIRE(out.size() == 8);
        CHECK(out[2] == doctest::Approx(50.0));
        CHECK(std::isnan(out[3]));  // the placeholder itself
        CHECK(std::isnan(out[4]));  // warmup of the new run
        CHECK(std::isnan(out[5]));
        CHECK(out[6] == doctest::Approx(50.0));
        CHECK(out[7] == doctest::Approx(75.0));
    }

    SUBCASE("negative and non-finite closes count as missing") {
        const double nan = std::numeric_limits<double>::quiet_NaN();
        const double inf = std::numeric_limits<double>::infinity();
        for (double bad : {-1.0, nan, inf}) {
            auto c = closes({10, bad, 10, 11, 10, 11});
            vroom::rsi::compute(c.data(), c.size(), 2, out);
            REQUIRE(out.size() == 6);
            for (std::size_t i = 0; i < 4; ++i) CHECK(std::isnan(out[i]));
            CHECK(out[4] == doctest::Approx(50.0));
            CHECK(out[5] == doctest::Approx(75.0));
        }
    }
}

TEST_CASE("rsi::compute_ma") {
    // closes [10,11,10,11,10], period 2 → rsi = [_,_,50,75,37.5].
    std::vector<double> rsi;
    auto c = closes({10, 11, 10, 11, 10});
    vroom::rsi::compute(c.data(), c.size(), 2, rsi);
    REQUIRE(rsi.size() == 5);
    CHECK(rsi[2] == doctest::Approx(50.0));
    CHECK(rsi[3] == doctest::Approx(75.0));
    CHECK(rsi[4] == doctest::Approx(37.5));

    std::vector<double> ma;
    SUBCASE("SMA of RSI, NaN until the window is full of valid values") {
        vroom::rsi::compute_ma(rsi, 2, vroom::ma::KIND_SMA, ma);
        REQUIRE(ma.size() == 5);
        CHECK(std::isnan(ma[0]));
        CHECK(std::isnan(ma[1]));
        CHECK(std::isnan(ma[2]));  // window {rsi[1]=NaN, rsi[2]} → NaN
        CHECK(ma[3] == doctest::Approx(62.5));   // mean(50,75)
        CHECK(ma[4] == doctest::Approx(56.25));  // mean(75,37.5)
    }

    SUBCASE("ma_period 1 is the identity") {
        vroom::rsi::compute_ma(rsi, 1, vroom::ma::KIND_SMA, ma);
        CHECK(std::isnan(ma[1]));
        CHECK(ma[2] == doctest::Approx(50.0));
        CHECK(ma[4] == doctest::Approx(37.5));
    }

    SUBCASE("EMA trendline seeds on the same index as the SMA") {
        // Seed@3 = mean(50,75) = 62.5, matching the SMA; then
        // @4 = (2/3)*37.5 + (1/3)*62.5 = 45.833…, where the SMA gives 56.25.
        vroom::rsi::compute_ma(rsi, 2, vroom::ma::KIND_EMA, ma);
        REQUIRE(ma.size() == 5);
        CHECK(std::isnan(ma[2]));
        CHECK(ma[3] == doctest::Approx(62.5));
        CHECK(ma[4] == doctest::Approx(45.8333333));
    }
}

TEST_CASE("rsi::compute_ma re-seeds after a gap in the RSI series") {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const std::vector<double> rsi = {nan, 50, 70, nan, 40, 60, 90};
    std::vector<double> ma;

    SUBCASE("SMA") {
        vroom::rsi::compute_ma(rsi, 2, vroom::ma::KIND_SMA, ma);
        REQUIRE(ma.size() == 7);
        CHECK(ma[2] == doctest::Approx(60.0));  // mean(50,70)
        CHECK(std::isnan(ma[3]));               // the gap
        CHECK(std::isnan(ma[4]));               // window straddles the gap
        CHECK(ma[5] == doctest::Approx(50.0));  // mean(40,60)
        CHECK(ma[6] == doctest::Approx(75.0));  // mean(60,90)
    }

    SUBCASE("EMA") {
        // Run two seeds at 5 = mean(40,60) = 50, then
        // @6 = (2/3)*90 + (1/3)*50 = 76.666…, where the SMA gives 75.
        vroom::rsi::compute_ma(rsi, 2, vroom::ma::KIND_EMA, ma);
        REQUIRE(ma.size() == 7);
        CHECK(ma[2] == doctest::Approx(60.0));
        CHECK(std::isnan(ma[3]));
        CHECK(std::isnan(ma[4]));
        CHECK(ma[5] == doctest::Approx(50.0));
        CHECK(ma[6] == doctest::Approx(76.6666667));
    }
}

TEST_CASE("rsi::band_fraction maps the fixed 0..100 domain about the pane center") {
    SUBCASE("50 is the center and the domain fills the band") {
        CHECK(vroom::rsi::band_fraction(50.0, 1.0) == doctest::Approx(0.5));
        CHECK(vroom::rsi::band_fraction(100.0, 1.0) == doctest::Approx(1.0));
        CHECK(vroom::rsi::band_fraction(0.0, 1.0) == doctest::Approx(0.0));
        CHECK(vroom::rsi::band_fraction(70.0, 1.0) == doctest::Approx(0.7));
    }

    SUBCASE("y-zoom stretches about 50, which stays put") {
        CHECK(vroom::rsi::band_fraction(50.0, 2.0) == doctest::Approx(0.5));
        CHECK(vroom::rsi::band_fraction(75.0, 2.0) == doctest::Approx(1.0));
        // Zoomed in, the domain's ends run past the band and the pane clips them.
        CHECK(vroom::rsi::band_fraction(100.0, 2.0) == doctest::Approx(1.5));
    }
}
