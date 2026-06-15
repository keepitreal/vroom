#include "doctest.h"

#include <cmath>
#include <vector>

#include "ma.h"

namespace {
std::vector<VroomCandle> from_close(std::initializer_list<double> cs) {
    std::vector<VroomCandle> out;
    int64_t t = 0;
    for (double c : cs) {
        out.push_back(VroomCandle{t, c, c, c, c, 0.0});
        t += 60'000;
    }
    return out;
}
}  // namespace

TEST_CASE("ma::compute SMA") {
    std::vector<double> out;
    auto c = from_close({1, 2, 3, 4, 10});
    vroom::ma::compute(c.data(), c.size(), vroom::ma::KIND_SMA, 3,
                       vroom::ma::SOURCE_CLOSE, out);
    REQUIRE(out.size() == 5);
    CHECK(std::isnan(out[0]));
    CHECK(std::isnan(out[1]));  // NaN before period-1
    CHECK(out[2] == doctest::Approx(2.0));
    CHECK(out[3] == doctest::Approx(3.0));
    CHECK(out[4] == doctest::Approx(17.0 / 3.0));
}

TEST_CASE("ma::compute EMA reacts faster than SMA (SMA-seeded)") {
    std::vector<double> out;
    auto c = from_close({1, 2, 3, 4, 10});
    vroom::ma::compute(c.data(), c.size(), vroom::ma::KIND_EMA, 3,
                       vroom::ma::SOURCE_CLOSE, out);
    CHECK(std::isnan(out[1]));
    CHECK(out[2] == doctest::Approx(2.0));  // seed = SMA(1,2,3)
    CHECK(out[3] == doctest::Approx(3.0));  // 0.5*4 + 0.5*2
    CHECK(out[4] == doctest::Approx(6.5));  // 0.5*10 + 0.5*3 (> SMA's 5.667)
}

TEST_CASE("ma::compute source selection + warmup") {
    // hl2 source, period 1 -> the per-candle (high+low)/2.
    std::vector<VroomCandle> c;
    c.push_back(VroomCandle{0, 5.0, 10.0, 4.0, 6.0, 0.0});  // hl2 = 7
    std::vector<double> out;
    vroom::ma::compute(c.data(), c.size(), vroom::ma::KIND_SMA, 1,
                       vroom::ma::SOURCE_HL2, out);
    CHECK(out[0] == doctest::Approx(7.0));

    // n < period -> all NaN.
    auto c2 = from_close({1, 2});
    vroom::ma::compute(c2.data(), c2.size(), vroom::ma::KIND_SMA, 3,
                       vroom::ma::SOURCE_CLOSE, out);
    REQUIRE(out.size() == 2);
    CHECK(std::isnan(out[0]));
    CHECK(std::isnan(out[1]));
}
