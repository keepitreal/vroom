#include "doctest.h"

#include <cmath>
#include <vector>

#include "atr.h"

namespace {
// Build candles from explicit high/low/close triples — ATR reads all three.
struct Bar {
    double high;
    double low;
    double close;
};

std::vector<VroomCandle> bars(std::initializer_list<Bar> bs) {
    std::vector<VroomCandle> out;
    int64_t t = 0;
    for (const Bar& b : bs) {
        out.push_back(VroomCandle{t, b.close, b.high, b.low, b.close, 0.0});
        t += 60'000;
    }
    return out;
}
}  // namespace

TEST_CASE("atr::compute") {
    std::vector<double> out;

    SUBCASE("bar 0 falls back to high - low") {
        auto c = bars({{12, 10, 11}, {13, 11, 12}});
        vroom::atr::compute(c.data(), c.size(), 1, vroom::atr::kRma, out);
        REQUIRE(out.size() == 2);
        CHECK(out[0] == doctest::Approx(2.0));  // 12 - 10, no previous close
    }

    SUBCASE("gaps outside the bar's own range drive the true range") {
        // Bar 1 gaps up: its 4-wide own range loses to the 6-point distance
        // from its high back down to the previous close.
        // Bar 2 gaps down: |low - prevClose| = |14 - 20| = 6 wins over 2.
        auto c = bars({{12, 10, 10}, {20, 16, 20}, {16, 14, 14}});
        vroom::atr::compute(c.data(), c.size(), 1, vroom::atr::kRma, out);
        CHECK(out[0] == doctest::Approx(2.0));
        CHECK(out[1] == doctest::Approx(10.0));  // |20 - 10|
        CHECK(out[2] == doctest::Approx(6.0));   // |14 - 20|
    }

    SUBCASE("NaN warmup up to period - 1") {
        auto c = bars({{12, 10, 11}, {13, 11, 12}, {14, 12, 13}});
        vroom::atr::compute(c.data(), c.size(), 3, vroom::atr::kRma, out);
        REQUIRE(out.size() == 3);
        CHECK(std::isnan(out[0]));
        CHECK(std::isnan(out[1]));
        CHECK_FALSE(std::isnan(out[2]));  // first value at period - 1
    }

    SUBCASE("too little data → all NaN") {
        auto c = bars({{12, 10, 11}, {13, 11, 12}});
        vroom::atr::compute(c.data(), c.size(), 3, vroom::atr::kRma, out);
        REQUIRE(out.size() == 2);
        CHECK(std::isnan(out[0]));
        CHECK(std::isnan(out[1]));
    }

    SUBCASE("Wilder smoothing") {
        // TR: 2, 3 (13-11 vs |13-11|=2 vs |11-11|=0), 4, 5.
        auto c = bars({{12, 10, 11}, {14, 11, 13}, {17, 13, 16}, {21, 16, 20}});
        vroom::atr::compute(c.data(), c.size(), 2, vroom::atr::kRma, out);
        CHECK(out[1] == doctest::Approx(2.5));   // (2 + 3) / 2
        CHECK(out[2] == doctest::Approx(3.25));  // (2.5 + 4) / 2
        CHECK(out[3] == doctest::Approx(4.125));
    }

    SUBCASE("simple smoothing") {
        auto c = bars({{12, 10, 11}, {14, 11, 13}, {17, 13, 16}, {21, 16, 20}});
        vroom::atr::compute(c.data(), c.size(), 2, vroom::atr::kSma, out);
        CHECK(out[1] == doctest::Approx(2.5));  // (2 + 3) / 2
        CHECK(out[2] == doctest::Approx(3.5));  // (3 + 4) / 2
        CHECK(out[3] == doctest::Approx(4.5));  // (4 + 5) / 2
    }

    SUBCASE("exponential smoothing") {
        // Seeded with the SMA of the first two TRs, then alpha = 2/(2+1).
        auto c = bars({{12, 10, 11}, {14, 11, 13}, {17, 13, 16}, {21, 16, 20}});
        vroom::atr::compute(c.data(), c.size(), 2, vroom::atr::kEma, out);
        CHECK(out[1] == doctest::Approx(2.5));
        CHECK(out[2] == doctest::Approx(2.0 / 3.0 * 4.0 + 1.0 / 3.0 * 2.5));
        CHECK(out[3] ==
              doctest::Approx(2.0 / 3.0 * 5.0 +
                              1.0 / 3.0 * (2.0 / 3.0 * 4.0 + 1.0 / 3.0 * 2.5)));
    }

    SUBCASE("flat series → zero volatility") {
        auto c = bars({{10, 10, 10}, {10, 10, 10}, {10, 10, 10}});
        for (int s : {vroom::atr::kRma, vroom::atr::kSma, vroom::atr::kEma}) {
            vroom::atr::compute(c.data(), c.size(), 2, s, out);
            CHECK(out[1] == doctest::Approx(0.0));
            CHECK(out[2] == doctest::Approx(0.0));
        }
    }

    SUBCASE("guards") {
        auto c = bars({{12, 10, 11}, {13, 11, 12}});
        vroom::atr::compute(nullptr, 4, 2, vroom::atr::kRma, out);
        CHECK(out.size() == 4);
        CHECK(std::isnan(out[0]));

        vroom::atr::compute(c.data(), c.size(), 0, vroom::atr::kRma, out);
        REQUIRE(out.size() == 2);
        CHECK(std::isnan(out[0]));
    }
}
