#include "doctest.h"

#include <cmath>
#include <vector>

#include "loading_series.h"

using vroom::loading_series::generate;
using vroom::loading_series::kLength;
using vroom::loading_series::kMaxMove;
using vroom::loading_series::kMinMove;
using vroom::loading_series::kStepMs;
using vroom::loading_series::kWickRange;
using vroom::loading_series::tail;

namespace {
constexpr int64_t kEnd = 1700000000000;
}

TEST_CASE("the walk is deterministic") {
    // The skeleton has to look identical on every platform and every mount —
    // a series that re-randomised would shimmer on any re-layout.
    const auto a = generate(kEnd);
    const auto b = generate(kEnd);
    REQUIRE(a.size() == b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        CHECK(a[i].open == doctest::Approx(b[i].open));
        CHECK(a[i].high == doctest::Approx(b[i].high));
        CHECK(a[i].low == doctest::Approx(b[i].low));
        CHECK(a[i].close == doctest::Approx(b[i].close));
        CHECK(a[i].volume == doctest::Approx(b[i].volume));
    }
}

TEST_CASE("bars are ordered, evenly spaced, and end where asked") {
    const auto s = generate(kEnd);
    REQUIRE(s.size() == kLength);
    CHECK(s.back().time_ms == kEnd);
    for (std::size_t i = 1; i < s.size(); ++i) {
        CHECK(s[i].time_ms - s[i - 1].time_ms == kStepMs);
    }
}

TEST_CASE("each bar opens where the last one closed") {
    // This is what makes it a walk rather than a row of unrelated bars, and
    // it's what gives the skeleton a believable trend line.
    const auto s = generate(kEnd);
    for (std::size_t i = 1; i < s.size(); ++i) {
        CHECK(s[i].open == doctest::Approx(s[i - 1].close));
    }
}

TEST_CASE("OHLC is internally consistent") {
    const auto s = generate(kEnd);
    for (const auto& c : s) {
        CHECK(c.high >= c.open);
        CHECK(c.high >= c.close);
        CHECK(c.low <= c.open);
        CHECK(c.low <= c.close);
        CHECK(c.low > 0.0);
        CHECK(c.volume > 0.0);
    }
}

TEST_CASE("moves and wicks stay inside their configured bands") {
    const auto s = generate(kEnd);
    for (const auto& c : s) {
        // Rounding to 2dp perturbs the ratio slightly, hence the tolerance.
        const double move = std::abs(c.close / c.open - 1.0);
        CHECK(move >= kMinMove - 0.001);
        CHECK(move <= kMaxMove + 0.001);

        const double body_top = c.open > c.close ? c.open : c.close;
        const double body_bottom = c.open < c.close ? c.open : c.close;
        CHECK(c.high / body_top - 1.0 <= kWickRange + 0.001);
        CHECK(1.0 - c.low / body_bottom <= kWickRange + 0.001);
    }
}

TEST_CASE("the walk goes both ways") {
    // A drifting generator would leave the skeleton as one long ramp, which
    // reads as data rather than as a placeholder.
    const auto s = generate(kEnd);
    int up = 0;
    for (const auto& c : s) {
        if (c.close >= c.open) ++up;
    }
    CHECK(up > static_cast<int>(kLength) / 5);
    CHECK(up < static_cast<int>(kLength) * 4 / 5);
}

TEST_CASE("tail windows the same walk instead of resampling it") {
    // A resize has to reframe the series, not rewrite it: the bars that stay
    // on screen must be the same bars, still anchored at the right edge.
    const auto s = generate(kEnd);
    const ::VroomCandle* wide = tail(s, 80);
    const ::VroomCandle* narrow = tail(s, 30);
    REQUIRE(wide != nullptr);
    REQUIRE(narrow != nullptr);

    // Both windows end on the same bar.
    CHECK(wide[79].time_ms == narrow[29].time_ms);
    CHECK(wide[79].close == doctest::Approx(narrow[29].close));
    // And the narrow one is a suffix of the wide one.
    for (std::size_t i = 0; i < 30; ++i) {
        CHECK(wide[50 + i].close == doctest::Approx(narrow[i].close));
    }
}

TEST_CASE("tail clamps to the available length") {
    const auto s = generate(kEnd);
    CHECK(tail(s, kLength * 2) == s.data());
    CHECK(tail(s, 0) == s.data() + s.size());

    const std::vector<::VroomCandle> empty;
    CHECK(tail(empty, 10) == nullptr);
}
