#include "doctest.h"

#include "ticks.h"

namespace {
constexpr int64_t k1m = 60'000LL;
constexpr int64_t k5m = 5 * 60'000LL;
constexpr int64_t k15m = 15 * 60'000LL;
constexpr int64_t k1w = 7 * 24 * 60 * 60'000LL;
}  // namespace

TEST_CASE("pick_time_interval") {
    SUBCASE("degenerate args fall back to 1m") {
        CHECK(vroom::pick_time_interval(0, 100.f) == k1m);
        CHECK(vroom::pick_time_interval(-1, 100.f) == k1m);
        CHECK(vroom::pick_time_interval(600'000, 0.f) == k1m);
        CHECK(vroom::pick_time_interval(600'000, -5.f) == k1m);
    }

    SUBCASE("never returns below 1m for a very dense window") {
        // window 1ms across 1000px -> every interval is hugely over-spaced.
        CHECK(vroom::pick_time_interval(1, 1000.f) == k1m);
    }

    SUBCASE("never returns above 1w for a very wide window") {
        CHECK(vroom::pick_time_interval(1'000'000'000'000'000LL, 1.f) == k1w);
    }

    SUBCASE("wide window over narrow width picks a coarse interval") {
        CHECK(vroom::pick_time_interval(k1w, 100.f) == k1w);
    }

    SUBCASE("the 50px spacing threshold selects the smallest fit") {
        // ms_per_px = 6000 -> 5m spans exactly 50px (>= threshold) but 1m only
        // 10px, so 5m wins.
        CHECK(vroom::pick_time_interval(600'000, 100.f) == k5m);
        // ms_per_px = 6600 -> 5m drops to ~45px (< threshold) so 15m is picked.
        CHECK(vroom::pick_time_interval(660'000, 100.f) == k15m);
    }
}

TEST_CASE("pick_price_interval") {
    // area 550 -> target_count = max(2, 550/55) = 10, so rough = range / 10.
    constexpr float kArea = 550.f;

    SUBCASE("degenerate args fall back to 1.0") {
        CHECK(vroom::pick_price_interval(0.0, kArea) == doctest::Approx(1.0));
        CHECK(vroom::pick_price_interval(-1.0, kArea) == doctest::Approx(1.0));
        CHECK(vroom::pick_price_interval(1000.0, 0.f) == doctest::Approx(1.0));
        CHECK(vroom::pick_price_interval(1000.0, -5.f) == doctest::Approx(1.0));
    }

    SUBCASE("snaps to 1 / 2 / 5 / 10 x 10^n") {
        CHECK(vroom::pick_price_interval(1000.0, kArea) ==
              doctest::Approx(100.0));  // rough 100 -> norm 1 -> 1
        CHECK(vroom::pick_price_interval(2000.0, kArea) ==
              doctest::Approx(200.0));  // rough 200 -> norm 2 -> 2
        CHECK(vroom::pick_price_interval(5000.0, kArea) ==
              doctest::Approx(500.0));  // rough 500 -> norm 5 -> 5
        CHECK(vroom::pick_price_interval(8000.0, kArea) ==
              doctest::Approx(1000.0));  // rough 800 -> norm 8 -> 10
    }

    SUBCASE("normalized boundaries round up at 1.5 / 3.5 / 7.5") {
        // rough 1.4 -> norm 1.4 (< 1.5) -> 1.0
        CHECK(vroom::pick_price_interval(14.0, kArea) == doctest::Approx(1.0));
        // rough 1.5 -> norm 1.5 (>= 1.5) -> 2.0
        CHECK(vroom::pick_price_interval(15.0, kArea) == doctest::Approx(2.0));
    }

    SUBCASE("scales with magnitude for tiny ranges") {
        // rough 0.001 -> magnitude 1e-3, norm 1 -> 0.001
        CHECK(vroom::pick_price_interval(0.01, kArea) ==
              doctest::Approx(0.001));
    }
}
