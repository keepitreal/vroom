#include "doctest.h"

#include <initializer_list>  // libstdc++ needs it for the braced-list loops below

#include "tip_pulse.h"

using vroom::tip_pulse::at;
using vroom::tip_pulse::Frame;
using vroom::tip_pulse::kDefaultStartAlpha;
using vroom::tip_pulse::kPeriodSeconds;
using vroom::tip_pulse::kRingSeconds;
using vroom::tip_pulse::kWidthMul;

// Where the ring has got to `seconds` into a cycle.
static Frame at_seconds(float seconds) { return at(seconds / kPeriodSeconds); }

TEST_CASE("the ring is five sixths of the dot radius wide") {
    // A 6pt dot gets a 5pt ring.
    CHECK(6.f * kWidthMul == doctest::Approx(5.f));
}

TEST_CASE("the cycle starts at 40% by default with the ring's inner edge on the dot") {
    const Frame f = at(0.f);
    CHECK(f.radius_mul - kWidthMul / 2.f == doctest::Approx(1.f));
    CHECK(f.alpha == doctest::Approx(0.4f));
    CHECK(kDefaultStartAlpha == doctest::Approx(0.4f));
}

TEST_CASE("a custom start opacity scales the whole fade") {
    CHECK(at(0.f, 0.8f).alpha == doctest::Approx(0.8f));
    CHECK(at(kRingSeconds / 2.f / kPeriodSeconds, 0.8f).alpha == doctest::Approx(0.4f));
    CHECK(at(0.f, 0.8f).radius_mul == doctest::Approx(at(0.f).radius_mul));

    SUBCASE("out-of-range values clamp to [0,1]") {
        CHECK(at(0.f, 1.5f).alpha == doctest::Approx(1.f));
        CHECK(at(0.f, -0.5f).alpha == doctest::Approx(0.f));
    }
}

TEST_CASE("the ring grows by twice its width as it fades out") {
    const Frame mid = at_seconds(kRingSeconds / 2.f);
    CHECK(mid.radius_mul - at(0.f).radius_mul == doctest::Approx(kWidthMul));
    CHECK(mid.alpha == doctest::Approx(kDefaultStartAlpha / 2.f));

    const Frame end = at_seconds(kRingSeconds);
    CHECK(end.radius_mul - at(0.f).radius_mul == doctest::Approx(2.f * kWidthMul));
    CHECK(end.alpha == doctest::Approx(0.f));
}

TEST_CASE("a 0.9s ring every 1.5s, as Liveline's live dot does") {
    CHECK(kPeriodSeconds == doctest::Approx(1.5f));
    CHECK(kRingSeconds == doctest::Approx(0.9f));
}

TEST_CASE("the rest draws nothing") {
    // That gap is the whole reason this reads as a heartbeat, so it has to stay
    // fully transparent.
    for (float s : {0.9f, 1.1f, 1.3f, 1.49f}) {
        CHECK(at_seconds(s).alpha == doctest::Approx(0.f));
    }
}

TEST_CASE("the ring only ever grows within a cycle") {
    // Open interval: phase 1 is the next cycle's start, where it resets.
    float prev = 0.f;
    for (int i = 0; i < 100; ++i) {
        const Frame f = at(static_cast<float>(i) / 100.f);
        CHECK(f.radius_mul >= prev);
        prev = f.radius_mul;
    }
    CHECK(at(1.f).radius_mul == doctest::Approx(at(0.f).radius_mul));
}

TEST_CASE("a 6pt dot gets a 5pt ring reaching 21pt") {
    constexpr float kDotR = 6.f;
    CHECK(kDotR * kWidthMul == doctest::Approx(5.f));
    CHECK(kDotR * (at(0.999f).radius_mul + kWidthMul / 2.f) == doctest::Approx(21.f));
}

TEST_CASE("at its widest the ring reaches 3.5x the dot radius") {
    // tip_geometry.h leaves the ring out of the gutter on the strength of this.
    CHECK(at(0.999f).radius_mul + kWidthMul / 2.f == doctest::Approx(3.5f));
}

TEST_CASE("alpha stays in range across the whole cycle") {
    for (int i = 0; i <= 200; ++i) {
        const Frame f = at(static_cast<float>(i) / 200.f);
        CHECK(f.alpha >= 0.f);
        CHECK(f.alpha <= 1.f);
    }
}

TEST_CASE("phase wraps, so callers can pass raw elapsed cycles") {
    const Frame ref = at(0.3f);
    for (float p : {1.3f, 7.3f, -0.7f, -3.7f}) {
        const Frame f = at(p);
        CHECK(f.radius_mul == doctest::Approx(ref.radius_mul));
        CHECK(f.alpha == doctest::Approx(ref.alpha));
    }
}
