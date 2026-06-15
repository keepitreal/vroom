#include "doctest.h"

#include <cmath>
#include <vector>

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
}  // namespace

TEST_CASE("macd::compute") {
    std::vector<double> macd;
    std::vector<double> sig;
    std::vector<double> hist;

    SUBCASE("hand-computed, SMA-seeded (fast 2, slow 3, signal 2)") {
        // closes 1..6: EMA2 seeds 1.5@1 -> 2.5,3.5,4.5,5.5; EMA3 seeds 2@2 ->
        // 3,4,5; macd = 0.5 from idx2; signal = 0.5 from idx3; hist = 0 from 3.
        auto c = closes({1, 2, 3, 4, 5, 6});
        vroom::macd::compute(c.data(), c.size(), 2, 3, 2, macd, sig, hist);
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
        vroom::macd::compute(c.data(), c.size(), 2, 3, 2, macd, sig, hist);
        CHECK(macd[2] < 0.0);
        CHECK(macd[3] > 0.0);
        CHECK(std::isnan(sig[2]));
        CHECK_FALSE(std::isnan(sig[3]));
        CHECK(std::isnan(hist[2]));
        CHECK(hist[3] == doctest::Approx(macd[3] - sig[3]));
    }

    SUBCASE("too little data (n < slow) -> all NaN") {
        auto c = closes({1, 2});  // n=2 < slow=3
        vroom::macd::compute(c.data(), c.size(), 2, 3, 2, macd, sig, hist);
        REQUIRE(macd.size() == 2);
        for (std::size_t i = 0; i < 2; ++i) {
            CHECK(std::isnan(macd[i]));
            CHECK(std::isnan(sig[i]));
            CHECK(std::isnan(hist[i]));
        }
    }
}
