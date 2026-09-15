#include "doctest.h"

#include "loading_wave.h"

using vroom::loading_wave::at;
using vroom::loading_wave::kAmplitudeFrac;
using vroom::loading_wave::kCyclesAcrossWidth;
using vroom::loading_wave::kPeriodSeconds;
using vroom::loading_wave::kTemporalRadPerSecond;
using vroom::loading_wave::kTwoPi;
using vroom::loading_wave::y_frac;

TEST_CASE("the offset stays within a unit sine for any time and position") {
    for (int xi = 0; xi <= 50; ++xi) {
        for (int ti = 0; ti <= 40; ++ti) {
            const float v = at(static_cast<float>(ti) * 0.1f,
                               static_cast<float>(xi) / 50.f);
            CHECK(v >= -1.f);
            CHECK(v <= 1.f);
        }
    }
}

TEST_CASE("the wave travels toward the right") {
    // A crest at x after dt must sit further right than it did at t. That
    // direction is the whole reason the phase is (k*x - w*t) and not a sum.
    const float dt = 0.4f;
    // Phase pi/2 is a crest. Solve k*x = pi/2 at t=0, then again at t=dt.
    const float k = kCyclesAcrossWidth * kTwoPi;
    const float x0 = (kTwoPi * 0.25f) / k;
    const float x1 = (kTwoPi * 0.25f + kTemporalRadPerSecond * dt) / k;
    CHECK(x1 > x0);
    CHECK(at(0.f, x0) == doctest::Approx(1.f));
    CHECK(at(dt, x1) == doctest::Approx(1.f));
}

TEST_CASE("a full period returns the wave to where it started") {
    for (int xi = 0; xi <= 20; ++xi) {
        const float x = static_cast<float>(xi) / 20.f;
        const float ref = at(0.37f, x);
        for (float k : {1.f, 4.f, 25.f}) {
            CHECK(at(0.37f + kPeriodSeconds * k, x) ==
                  doctest::Approx(ref).epsilon(0.001));
        }
    }
}

TEST_CASE("one period moves a crest exactly one wavelength") {
    // Phase velocity is w/k in x-fractions per second, so a period covers
    // 1/kCyclesAcrossWidth of the width — which is what makes the wave look
    // like it's sliding rather than flickering between two shapes.
    const float k = kCyclesAcrossWidth * kTwoPi;
    const float travelled = (kTemporalRadPerSecond / k) * kPeriodSeconds;
    CHECK(travelled == doctest::Approx(1.f / kCyclesAcrossWidth));
}

TEST_CASE("the requested number of crests fits across the plot") {
    // Counting sign changes of the derivative is fussy; counting how many times
    // the wave returns to its starting phase is equivalent and simpler.
    CHECK(at(0.f, 0.f) == doctest::Approx(at(0.f, 1.f / kCyclesAcrossWidth))
                              .epsilon(0.001));
    CHECK(kCyclesAcrossWidth >= 1.f);
    CHECK(kCyclesAcrossWidth <= 2.f);
}

TEST_CASE("y_frac centres the wave on the pane and stays inside it") {
    for (int xi = 0; xi <= 50; ++xi) {
        for (int ti = 0; ti <= 20; ++ti) {
            const float y = y_frac(static_cast<float>(ti) * 0.2f,
                                   static_cast<float>(xi) / 50.f);
            // Comfortably off both edges, so a crest never clips the pane.
            CHECK(y >= 0.5f - kAmplitudeFrac);
            CHECK(y <= 0.5f + kAmplitudeFrac);
            CHECK(y > 0.f);
            CHECK(y < 1.f);
        }
    }
}

TEST_CASE("y_frac tracks the offset it is built from") {
    for (int xi = 0; xi <= 10; ++xi) {
        const float x = static_cast<float>(xi) / 10.f;
        CHECK(y_frac(0.8f, x) ==
              doctest::Approx(0.5f + kAmplitudeFrac * at(0.8f, x)));
    }
}

TEST_CASE("the drift is slow enough to read as a placeholder") {
    // Regression guard on feel: an earlier revision ran ~4x this fast and read
    // as frantic. A crest should take a couple of seconds to cross the plot.
    const float k = kCyclesAcrossWidth * kTwoPi;
    const float cross_seconds = k / kTemporalRadPerSecond;
    CHECK(cross_seconds > 2.f);
    CHECK(cross_seconds < 10.f);
}
