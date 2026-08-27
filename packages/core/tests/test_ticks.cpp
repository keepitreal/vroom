#include "doctest.h"

#include "ticks.h"

#include <cstdlib>
#include <ctime>
#include <string>

namespace {
constexpr int64_t k1m = 60'000LL;
constexpr int64_t k5m = 5 * 60'000LL;
constexpr int64_t k15m = 15 * 60'000LL;
constexpr int64_t k1w = 7 * 24 * 60 * 60'000LL;

using vroom::TimeTick;
using vroom::TimeUnit;
}  // namespace

TEST_CASE("pick_time_tick fixed intervals") {
    SUBCASE("degenerate args fall back to 1m") {
        CHECK(vroom::pick_time_tick(0, 100.f).unit == TimeUnit::Fixed);
        CHECK(vroom::pick_time_tick(0, 100.f).step_ms == k1m);
        CHECK(vroom::pick_time_tick(-1, 100.f).step_ms == k1m);
        CHECK(vroom::pick_time_tick(600'000, 0.f).step_ms == k1m);
        CHECK(vroom::pick_time_tick(600'000, -5.f).step_ms == k1m);
    }

    SUBCASE("never returns below 1m for a very dense window") {
        const TimeTick t = vroom::pick_time_tick(1, 1000.f);
        CHECK(t.unit == TimeUnit::Fixed);
        CHECK(t.step_ms == k1m);
    }

    SUBCASE("the 50px spacing threshold selects the smallest fit") {
        // ms_per_px = 6000 -> 5m spans exactly 50px (>= threshold) but 1m only
        // 10px, so 5m wins.
        const TimeTick a = vroom::pick_time_tick(600'000, 100.f);
        CHECK(a.unit == TimeUnit::Fixed);
        CHECK(a.step_ms == k5m);
        // ms_per_px = 6600 -> 5m drops to ~45px (< threshold) so 15m is picked.
        const TimeTick b = vroom::pick_time_tick(660'000, 100.f);
        CHECK(b.unit == TimeUnit::Fixed);
        CHECK(b.step_ms == k15m);
    }

    SUBCASE("tops out at 1w before switching to calendar units") {
        // window 1w over 100px -> 1w spans exactly 100px, the coarsest fixed fit.
        const TimeTick t = vroom::pick_time_tick(k1w, 100.f);
        CHECK(t.unit == TimeUnit::Fixed);
        CHECK(t.step_ms == k1w);
    }
}

TEST_CASE("pick_time_tick calendar units") {
    // area fixed at 100px, so ms_per_px == window / 100.
    SUBCASE("months: 1mo / 3mo / 6mo as the window widens") {
        const TimeTick mo1 = vroom::pick_time_tick(3'000'000'000LL, 100.f);
        CHECK(mo1.unit == TimeUnit::Month);
        CHECK(mo1.step == 1);

        const TimeTick mo3 = vroom::pick_time_tick(10'000'000'000LL, 100.f);
        CHECK(mo3.unit == TimeUnit::Month);
        CHECK(mo3.step == 3);

        const TimeTick mo6 = vroom::pick_time_tick(20'000'000'000LL, 100.f);
        CHECK(mo6.unit == TimeUnit::Month);
        CHECK(mo6.step == 6);
    }

    SUBCASE("years: 1y then nice-number steps for very wide windows") {
        const TimeTick y1 = vroom::pick_time_tick(40'000'000'000LL, 100.f);
        CHECK(y1.unit == TimeUnit::Year);
        CHECK(y1.step == 1);

        const TimeTick y5 = vroom::pick_time_tick(200'000'000'000LL, 100.f);
        CHECK(y5.unit == TimeUnit::Year);
        CHECK(y5.step == 5);
    }
}

TEST_CASE("calendar tick stepping (UTC)") {
    // Pin the zone so mktime/localtime land on deterministic boundaries.
    setenv("TZ", "UTC", 1);
    tzset();

    constexpr int64_t kJan1_2025 = 1'735'689'600'000LL;  // 2025-01-01T00:00:00Z
    constexpr int64_t kFeb1_2025 = 1'738'368'000'000LL;  // +31 days
    constexpr int64_t kApr1_2025 = 1'743'465'600'000LL;  // +90 days
    constexpr int64_t kJan1_2026 = 1'767'225'600'000LL;  // +365 days
    constexpr int64_t kJan1_2027 = 1'798'761'600'000LL;  // +365 days

    const TimeTick month1{TimeUnit::Month, 0, 1};
    const TimeTick quarter{TimeUnit::Month, 0, 3};
    const TimeTick year1{TimeUnit::Year, 0, 1};

    SUBCASE("first_tick_at_or_after snaps up to a month boundary") {
        // Mid-January -> next month start is Feb 1.
        const int64_t mid_jan = kJan1_2025 + 14LL * 24 * 60 * 60'000;
        CHECK(vroom::first_tick_at_or_after(mid_jan, month1) == kFeb1_2025);
        // Exactly on Jan 1 stays on Jan 1.
        CHECK(vroom::first_tick_at_or_after(kJan1_2025, month1) == kJan1_2025);
    }

    SUBCASE("quarters align to Jan/Apr/Jul/Oct") {
        // Feb 10 -> next quarter start is Apr 1.
        const int64_t feb10 = kFeb1_2025 + 9LL * 24 * 60 * 60'000;
        CHECK(vroom::first_tick_at_or_after(feb10, quarter) == kApr1_2025);
    }

    SUBCASE("years snap up to Jan 1") {
        const int64_t mid_2025 = kJan1_2025 + 150LL * 24 * 60 * 60'000;
        CHECK(vroom::first_tick_at_or_after(mid_2025, year1) == kJan1_2026);
    }

    SUBCASE("next_tick advances by the calendar step") {
        CHECK(vroom::next_tick(kJan1_2025, month1) == kFeb1_2025);
        CHECK(vroom::next_tick(kJan1_2026, year1) == kJan1_2027);
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

// price_decimals and format_price now live in price_format.h; their coverage
// moved to test_price_format.cpp alongside the precision rule they feed.
