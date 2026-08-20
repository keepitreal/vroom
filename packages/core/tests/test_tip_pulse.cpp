#include "doctest.h"

#include "tip_pulse.h"

using vroom::tip_pulse::at;
using vroom::tip_pulse::Frame;
using vroom::tip_pulse::kPeriodSeconds;

TEST_CASE("the cycle starts as a tight, washed-in bloom") {
    const Frame f = at(0.f);
    CHECK(f.radius_mul == doctest::Approx(1.f));
    CHECK(f.fill_alpha == doctest::Approx(0.25f));
    CHECK(f.stroke_alpha == doctest::Approx(0.40f));
}

TEST_CASE("stage boundaries hand off without a jump") {
    SUBCASE("expand ends with the fill gone and the edge at full strength") {
        const Frame f = at(0.25f);
        CHECK(f.radius_mul == doctest::Approx(2.5f));
        CHECK(f.fill_alpha == doctest::Approx(0.f));
        CHECK(f.stroke_alpha == doctest::Approx(0.80f));
    }

    SUBCASE("the fade ends fully transparent at full radius") {
        const Frame f = at(0.525f);
        CHECK(f.radius_mul == doctest::Approx(3.5f));
        CHECK(f.stroke_alpha == doctest::Approx(0.f));
    }

    SUBCASE("approaching a boundary matches landing on it") {
        const Frame before = at(0.25f - 1e-4f);
        const Frame after = at(0.25f);
        CHECK(before.radius_mul == doctest::Approx(after.radius_mul).epsilon(0.01));
        CHECK(before.stroke_alpha == doctest::Approx(after.stroke_alpha).epsilon(0.01));
    }
}

TEST_CASE("the rest stage draws nothing") {
    // Nearly half the period is dead time — that gap is the whole reason this
    // reads as a heartbeat, so it has to stay fully transparent.
    for (float p : {0.53f, 0.7f, 0.9f, 0.999f}) {
        const Frame f = at(p);
        CHECK(f.fill_alpha == doctest::Approx(0.f));
        CHECK(f.stroke_alpha == doctest::Approx(0.f));
    }
}

TEST_CASE("the ring only ever grows within a cycle") {
    // Open interval: phase 1 is the next cycle's start, where it resets to 1x.
    float prev = 0.f;
    for (int i = 0; i < 100; ++i) {
        const Frame f = at(static_cast<float>(i) / 100.f);
        CHECK(f.radius_mul >= prev);
        prev = f.radius_mul;
    }
    CHECK(at(0.999f).radius_mul == doctest::Approx(3.5f));
    CHECK(at(1.f).radius_mul == doctest::Approx(1.f));
}

TEST_CASE("alphas stay in range across the whole cycle") {
    for (int i = 0; i <= 200; ++i) {
        const Frame f = at(static_cast<float>(i) / 200.f);
        CHECK(f.fill_alpha >= 0.f);
        CHECK(f.fill_alpha <= 1.f);
        CHECK(f.stroke_alpha >= 0.f);
        CHECK(f.stroke_alpha <= 1.f);
        CHECK(f.radius_mul >= 1.f);
    }
}

TEST_CASE("phase wraps, so callers can pass raw elapsed cycles") {
    const Frame ref = at(0.3f);
    for (float p : {1.3f, 7.3f, -0.7f, -3.7f}) {
        const Frame f = at(p);
        CHECK(f.radius_mul == doctest::Approx(ref.radius_mul));
        CHECK(f.fill_alpha == doctest::Approx(ref.fill_alpha));
        CHECK(f.stroke_alpha == doctest::Approx(ref.stroke_alpha));
    }
}

TEST_CASE("the period leaves room for the rest stage") {
    CHECK(kPeriodSeconds == doctest::Approx(2.6f));
}
