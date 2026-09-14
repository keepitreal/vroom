#include "doctest.h"

#include <cmath>
#include <vector>

#include "ma.h"
#include "macd.h"

namespace {
std::vector<VroomCandle> closes(std::initializer_list<double> cs) {
    std::vector<VroomCandle> out;
    int64_t t = 0;
    for (double c : cs) {
        out.push_back(VroomCandle{t, c, c, c, c, 0.0});
        t += 60'000;
    }
    return out;
}

// Candles whose high sits `spread` above the close, so a non-close source
// produces a distinguishable series.
std::vector<VroomCandle> spread_closes(std::initializer_list<double> cs,
                                       double spread) {
    std::vector<VroomCandle> out;
    int64_t t = 0;
    for (double c : cs) {
        out.push_back(VroomCandle{t, c, c + spread, c, c, 0.0});
        t += 60'000;
    }
    return out;
}

constexpr int kEMA = vroom::ma::KIND_EMA;
constexpr int kSMA = vroom::ma::KIND_SMA;
constexpr int kClose = vroom::ma::SOURCE_CLOSE;
}  // namespace

TEST_CASE("macd::compute") {
    std::vector<double> macd;
    std::vector<double> sig;
    std::vector<double> hist;

    SUBCASE("hand-computed, SMA-seeded (fast 2, slow 3, signal 2)") {
        // closes 1..6: EMA2 seeds 1.5@1 -> 2.5,3.5,4.5,5.5; EMA3 seeds 2@2 ->
        // 3,4,5; macd = 0.5 from idx2; signal = 0.5 from idx3; hist = 0 from 3.
        auto c = closes({1, 2, 3, 4, 5, 6});
        vroom::macd::compute(c.data(), c.size(), 2, 3, 2, kClose, kEMA, kEMA,
                             macd, sig, hist);
        REQUIRE(macd.size() == 6);

        CHECK(std::isnan(macd[0]));
        CHECK(std::isnan(macd[1]));
        CHECK(macd[2] == doctest::Approx(0.5));
        CHECK(macd[5] == doctest::Approx(0.5));

        CHECK(std::isnan(sig[2]));  // signal defined from idx3
        CHECK(sig[3] == doctest::Approx(0.5));
        CHECK(sig[5] == doctest::Approx(0.5));

        CHECK(std::isnan(hist[2]));
        CHECK(hist[3] == doctest::Approx(0.0));
        CHECK(hist[5] == doctest::Approx(0.0));
    }

    SUBCASE("alignment + signs on a non-constant series") {
        // closes 1,2,1,2,1,2 -> macd[2] < 0, macd[3] > 0; signal/hist start @3.
        auto c = closes({1, 2, 1, 2, 1, 2});
        vroom::macd::compute(c.data(), c.size(), 2, 3, 2, kClose, kEMA, kEMA,
                             macd, sig, hist);
        CHECK(macd[2] < 0.0);
        CHECK(macd[3] > 0.0);
        CHECK(std::isnan(sig[2]));
        CHECK_FALSE(std::isnan(sig[3]));
        CHECK(std::isnan(hist[2]));
        CHECK(hist[3] == doctest::Approx(macd[3] - sig[3]));
    }

    SUBCASE("too little data (n < slow) -> all NaN") {
        auto c = closes({1, 2});  // n=2 < slow=3
        vroom::macd::compute(c.data(), c.size(), 2, 3, 2, kClose, kEMA, kEMA,
                             macd, sig, hist);
        REQUIRE(macd.size() == 2);
        for (std::size_t i = 0; i < 2; ++i) {
            CHECK(std::isnan(macd[i]));
            CHECK(std::isnan(sig[i]));
            CHECK(std::isnan(hist[i]));
        }
    }

    SUBCASE("SMA legs (fast 2, slow 3)") {
        // closes 1..6: SMA2 = 1.5,2.5,3.5,4.5,5.5 from idx1; SMA3 = 2,3,4,5
        // from idx2; macd = 0.5 from idx2, so the alignment matches the EMA
        // legs while the values come from simple averages.
        auto c = closes({1, 2, 3, 4, 5, 6});
        vroom::macd::compute(c.data(), c.size(), 2, 3, 2, kClose, kSMA, kEMA,
                             macd, sig, hist);
        CHECK(std::isnan(macd[1]));
        CHECK(macd[2] == doctest::Approx(0.5));
        CHECK(macd[5] == doctest::Approx(0.5));
    }

    SUBCASE("SMA signal line starts on the same index as the EMA one") {
        auto c = closes({1, 2, 1, 2, 1, 2});
        std::vector<double> ema_sig;
        vroom::macd::compute(c.data(), c.size(), 2, 3, 2, kClose, kEMA, kEMA,
                             macd, ema_sig, hist);
        vroom::macd::compute(c.data(), c.size(), 2, 3, 2, kClose, kEMA, kSMA,
                             macd, sig, hist);
        CHECK(std::isnan(sig[2]));
        CHECK_FALSE(std::isnan(sig[3]));
        // A 2-period average of the same two values agrees at the seed, then
        // the two smoothers diverge.
        CHECK(sig[3] == doctest::Approx(ema_sig[3]));
        CHECK(hist[3] == doctest::Approx(macd[3] - sig[3]));
    }

    SUBCASE("source selects the price series") {
        // A constant spread shifts every average equally, so the differences
        // cancel: high-sourced MACD matches the close-sourced one.
        auto c = spread_closes({1, 2, 1, 2, 1, 2}, 3.0);
        std::vector<double> close_macd;
        vroom::macd::compute(c.data(), c.size(), 2, 3, 2, kClose, kEMA, kEMA,
                             close_macd, sig, hist);
        vroom::macd::compute(c.data(), c.size(), 2, 3, 2,
                             vroom::ma::SOURCE_HIGH, kEMA, kEMA, macd, sig,
                             hist);
        CHECK(macd[3] == doctest::Approx(close_macd[3]));

        // hl2 averages a flat low with a rising high, so it tracks a different
        // series than close.
        auto uneven = std::vector<VroomCandle>{
            {0, 1, 4, 1, 1, 0.0},   {60'000, 2, 9, 1, 2, 0.0},
            {120'000, 1, 4, 1, 1, 0.0}, {180'000, 2, 9, 1, 2, 0.0},
        };
        vroom::macd::compute(uneven.data(), uneven.size(), 2, 3, 2, kClose,
                             kEMA, kEMA, close_macd, sig, hist);
        vroom::macd::compute(uneven.data(), uneven.size(), 2, 3, 2,
                             vroom::ma::SOURCE_HL2, kEMA, kEMA, macd, sig,
                             hist);
        CHECK(macd[3] != doctest::Approx(close_macd[3]));
    }
}

TEST_CASE("macd::autoscale fits the band symmetrically about zero") {
    const double line[] = {1.0, -6.0, 2.0};
    const double signal[] = {3.0, 1.0, 0.5};
    const double hist[] = {std::nan(""), 2.0, 1.5};

    SUBCASE("the largest magnitude across every series shown") {
        CHECK(vroom::macd::autoscale(line, signal, hist, 3) == doctest::Approx(6.0));
    }

    SUBCASE("a hidden series is passed as null so the rest fill the band") {
        CHECK(vroom::macd::autoscale(nullptr, signal, hist, 3) == doctest::Approx(3.0));
    }

    SUBCASE("nothing to plot fits to zero") {
        CHECK(vroom::macd::autoscale(nullptr, nullptr, nullptr, 3) == doctest::Approx(0.0));
    }
}

TEST_CASE("macd::band_fraction maps the amplitude about the pane center") {
    const double pad = vroom::macd::kBandPadFraction;

    SUBCASE("zero is the pane center, and the extremes are symmetric") {
        CHECK(vroom::macd::band_fraction(0.0, 4.0, 1.0) == doctest::Approx(0.5));
        CHECK(vroom::macd::band_fraction(4.0, 4.0, 1.0) == doctest::Approx(0.5 + 0.5 * pad));
        CHECK(vroom::macd::band_fraction(-4.0, 4.0, 1.0) == doctest::Approx(0.5 - 0.5 * pad));
    }

    SUBCASE("y-zoom stretches about zero, which stays put") {
        CHECK(vroom::macd::band_fraction(0.0, 4.0, 3.0) == doctest::Approx(0.5));
        CHECK(vroom::macd::band_fraction(2.0, 4.0, 2.0) == doctest::Approx(0.5 + 0.5 * pad));
    }

    SUBCASE("a degenerate fit collapses onto the zero line") {
        CHECK(vroom::macd::band_fraction(1.0, 0.0, 1.0) == doctest::Approx(0.5));
    }
}
