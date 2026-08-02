#include "doctest.h"

#include <cmath>
#include <vector>

#include "bollinger.h"
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

TEST_CASE("bollinger::compute SMA basis, hand-computed") {
    std::vector<double> mid, up, lo;
    auto c = from_close({1, 2, 3, 4, 10});
    vroom::bollinger::compute(c.data(), c.size(), 3, 2.0, vroom::ma::SOURCE_CLOSE,
                              vroom::ma::KIND_SMA, mid, up, lo);
    REQUIRE(mid.size() == 5);
    REQUIRE(up.size() == 5);
    REQUIRE(lo.size() == 5);
    // NaN warmup before period-1.
    for (std::size_t i = 0; i < 2; ++i) {
        CHECK(std::isnan(mid[i]));
        CHECK(std::isnan(up[i]));
        CHECK(std::isnan(lo[i]));
    }
    // i=2: window {1,2,3}, mean 2, population var 2/3.
    const double sd2 = std::sqrt(2.0 / 3.0);
    CHECK(mid[2] == doctest::Approx(2.0));
    CHECK(up[2] == doctest::Approx(2.0 + 2.0 * sd2));
    CHECK(lo[2] == doctest::Approx(2.0 - 2.0 * sd2));
    // i=3: window {2,3,4}, same variance.
    CHECK(mid[3] == doctest::Approx(3.0));
    CHECK(up[3] == doctest::Approx(3.0 + 2.0 * sd2));
    // i=4: window {3,4,10}, mean 17/3, var = ((3-17/3)² + (4-17/3)² + (10-17/3)²)/3.
    const double m4 = 17.0 / 3.0;
    const double var4 =
        ((3 - m4) * (3 - m4) + (4 - m4) * (4 - m4) + (10 - m4) * (10 - m4)) / 3.0;
    CHECK(mid[4] == doctest::Approx(m4));
    CHECK(up[4] == doctest::Approx(m4 + 2.0 * std::sqrt(var4)));
    CHECK(lo[4] == doctest::Approx(m4 - 2.0 * std::sqrt(var4)));
}

TEST_CASE("bollinger::compute n < period -> all NaN") {
    std::vector<double> mid, up, lo;
    auto c = from_close({1, 2});
    vroom::bollinger::compute(c.data(), c.size(), 3, 2.0, vroom::ma::SOURCE_CLOSE,
                              vroom::ma::KIND_SMA, mid, up, lo);
    REQUIRE(up.size() == 2);
    for (std::size_t i = 0; i < 2; ++i) {
        CHECK(std::isnan(mid[i]));
        CHECK(std::isnan(up[i]));
        CHECK(std::isnan(lo[i]));
    }
}

TEST_CASE("bollinger::compute EMA basis: middle is the EMA, band offset unchanged") {
    auto c = from_close({1, 2, 3, 4, 10});
    std::vector<double> mid_e, up_e, lo_e, mid_s, up_s, lo_s, ema;
    vroom::bollinger::compute(c.data(), c.size(), 3, 2.0, vroom::ma::SOURCE_CLOSE,
                              vroom::ma::KIND_EMA, mid_e, up_e, lo_e);
    vroom::bollinger::compute(c.data(), c.size(), 3, 2.0, vroom::ma::SOURCE_CLOSE,
                              vroom::ma::KIND_SMA, mid_s, up_s, lo_s);
    vroom::ma::compute(c.data(), c.size(), vroom::ma::KIND_EMA, 3,
                       vroom::ma::SOURCE_CLOSE, ema);
    for (std::size_t i = 2; i < c.size(); ++i) {
        CHECK(mid_e[i] == doctest::Approx(ema[i]));
        // The stdev uses the window mean regardless of basis, so the half-width
        // (upper - middle) matches the SMA-basis run exactly.
        CHECK(up_e[i] - mid_e[i] == doctest::Approx(up_s[i] - mid_s[i]));
        CHECK(mid_e[i] - lo_e[i] == doctest::Approx(mid_s[i] - lo_s[i]));
    }
}

TEST_CASE("bollinger::compute source selection (hl2)") {
    // Candles with distinct h/l so hl2 differs from close.
    std::vector<VroomCandle> c;
    c.push_back(VroomCandle{0, 0.0, 10.0, 2.0, 9.0, 0.0});       // hl2 = 6
    c.push_back(VroomCandle{60'000, 0.0, 14.0, 2.0, 3.0, 0.0});  // hl2 = 8
    std::vector<double> mid, up, lo;
    vroom::bollinger::compute(c.data(), c.size(), 2, 1.0, vroom::ma::SOURCE_HL2,
                              vroom::ma::KIND_SMA, mid, up, lo);
    // Window {6,8}: mean 7, population sd 1.
    CHECK(mid[1] == doctest::Approx(7.0));
    CHECK(up[1] == doctest::Approx(8.0));
    CHECK(lo[1] == doctest::Approx(6.0));
}

TEST_CASE("bollinger::compute mult=0 collapses bands onto the middle") {
    std::vector<double> mid, up, lo;
    auto c = from_close({1, 2, 3, 4, 10});
    vroom::bollinger::compute(c.data(), c.size(), 3, 0.0, vroom::ma::SOURCE_CLOSE,
                              vroom::ma::KIND_SMA, mid, up, lo);
    for (std::size_t i = 2; i < c.size(); ++i) {
        CHECK(up[i] == doctest::Approx(mid[i]));
        CHECK(lo[i] == doctest::Approx(mid[i]));
    }
}

TEST_CASE("bollinger::compute 20/2 against a naive reference") {
    // Deterministic synthetic closes: a drifting wave.
    std::vector<double> closes;
    for (int i = 0; i < 25; ++i)
        closes.push_back(100.0 + 0.5 * i + 3.0 * ((i * 7) % 11) / 11.0);
    std::vector<VroomCandle> c;
    int64_t t = 0;
    for (double v : closes) {
        c.push_back(VroomCandle{t, v, v, v, v, 0.0});
        t += 60'000;
    }
    std::vector<double> mid, up, lo;
    vroom::bollinger::compute(c.data(), c.size(), 20, 2.0, vroom::ma::SOURCE_CLOSE,
                              vroom::ma::KIND_SMA, mid, up, lo);
    for (std::size_t i = 0; i < 19; ++i) CHECK(std::isnan(up[i]));
    for (std::size_t i = 19; i < closes.size(); ++i) {
        double mean = 0.0;
        for (std::size_t j = i + 1 - 20; j <= i; ++j) mean += closes[j];
        mean /= 20.0;
        double var = 0.0;
        for (std::size_t j = i + 1 - 20; j <= i; ++j)
            var += (closes[j] - mean) * (closes[j] - mean);
        var /= 20.0;
        CHECK(mid[i] == doctest::Approx(mean));
        CHECK(up[i] == doctest::Approx(mean + 2.0 * std::sqrt(var)));
        CHECK(lo[i] == doctest::Approx(mean - 2.0 * std::sqrt(var)));
    }
}
