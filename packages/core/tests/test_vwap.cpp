#include "doctest.h"

#include <cmath>
#include <vector>

#include "vwap.h"

namespace {
constexpr int64_t kHour = 3'600'000;
constexpr int64_t kDay = 86'400'000;

// Candle with explicit OHLCV; time set by the caller.
VroomCandle bar(int64_t t, double h, double l, double c, double vol) {
    return VroomCandle{t, c, h, l, c, vol};
}
}  // namespace

TEST_CASE("vwap::compute session accumulation + reset") {
    std::vector<double> v;
    std::vector<unsigned char> brk;

    SUBCASE("accumulates within a session, resets at the UTC day boundary") {
        std::vector<VroomCandle> c = {
            bar(0, 10, 10, 10, 100),       // day 0, hlc3=10
            bar(kHour, 20, 20, 20, 100),   // day 0, hlc3=20
            bar(kDay, 30, 30, 30, 100),    // day 1 -> reset
        };
        vroom::vwap::compute(c.data(), c.size(), 0, v, brk);
        REQUIRE(v.size() == 3);
        CHECK(v[0] == doctest::Approx(10.0));
        CHECK(v[1] == doctest::Approx(15.0));  // (10*100 + 20*100)/200
        CHECK(v[2] == doctest::Approx(30.0));  // new session -> just this candle
        CHECK(brk[0] == 0);
        CHECK(brk[1] == 0);
        CHECK(brk[2] == 1);  // session break
    }

    SUBCASE("configurable reset offset shifts the boundary to 01:00") {
        std::vector<VroomCandle> c = {
            bar(0, 10, 10, 10, 100),      // 00:00 -> previous session
            bar(kHour, 20, 20, 20, 100),  // 01:00 -> new session (offset 60m)
        };
        vroom::vwap::compute(c.data(), c.size(), 60, v, brk);
        CHECK(brk[0] == 0);
        CHECK(brk[1] == 1);
        CHECK(v[1] == doctest::Approx(20.0));  // reset, only this candle
    }

    SUBCASE("hlc3 typical price") {
        std::vector<VroomCandle> c = {bar(0, 12, 6, 9, 10)};  // hlc3 = 9
        vroom::vwap::compute(c.data(), c.size(), 0, v, brk);
        CHECK(v[0] == doctest::Approx(9.0));
    }

    SUBCASE("zero volume -> NaN") {
        std::vector<VroomCandle> c = {bar(0, 10, 10, 10, 0)};
        vroom::vwap::compute(c.data(), c.size(), 0, v, brk);
        CHECK(std::isnan(v[0]));
    }
}
