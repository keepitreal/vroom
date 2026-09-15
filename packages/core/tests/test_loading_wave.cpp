#include "doctest.h"

#include <cmath>

#include "loading_wave.h"

using vroom::loading_wave::at;
using vroom::loading_wave::Frame;
using vroom::loading_wave::kMaxAlpha;
using vroom::loading_wave::kMaxScale;
using vroom::loading_wave::kMinAlpha;
using vroom::loading_wave::kMinScale;
using vroom::loading_wave::kPeriodSeconds;
using vroom::loading_wave::kSpatialRadPerBar;
using vroom::loading_wave::kTemporalRadPerSecond;

TEST_CASE("both outputs stay inside their range for any time and index") {
    for (int i = 0; i < 120; ++i) {
        for (int step = 0; step <= 40; ++step) {
            const Frame f = at(static_cast<float>(step) * 0.05f, i);
            CHECK(f.alpha >= kMinAlpha);
            CHECK(f.alpha <= kMaxAlpha);
            CHECK(f.scale >= kMinScale);
            CHECK(f.scale <= kMaxScale);
        }
    }
}

TEST_CASE("alpha and scale share a phase") {
    // The coupling is the effect: a crest has to be the tallest bar *and* the
    // most opaque one, or it reads as two animations instead of one pulse.
    for (int i = 0; i < 40; ++i) {
        const Frame f = at(static_cast<float>(i) * 0.037f, i);
        const float alpha_u = (f.alpha - kMinAlpha) / (kMaxAlpha - kMinAlpha);
        const float scale_u = (f.scale - kMinScale) / (kMaxScale - kMinScale);
        CHECK(alpha_u == doctest::Approx(scale_u));
    }
}

TEST_CASE("a crest is the extreme of both") {
    // sin peaks at pi/2, so solve 6t = pi/2 for bar 0.
    const float crest_t = 1.5707963f / kTemporalRadPerSecond;
    const Frame f = at(crest_t, 0);
    CHECK(f.alpha == doctest::Approx(kMaxAlpha));
    CHECK(f.scale == doctest::Approx(kMaxScale));

    const Frame trough = at(crest_t + kPeriodSeconds * 0.5f, 0);
    CHECK(trough.alpha == doctest::Approx(kMinAlpha));
    CHECK(trough.scale == doctest::Approx(kMinScale));
}

TEST_CASE("the wave travels toward higher indices") {
    // Phase velocity is temporal/spatial = 15 bars per second. After dt, the
    // crest that was at bar 0 must be at bar 15*dt — that direction is what
    // makes the pulse sweep left to right rather than right to left.
    const float crest_t = 1.5707963f / kTemporalRadPerSecond;
    const float velocity = kTemporalRadPerSecond / kSpatialRadPerBar;
    CHECK(velocity == doctest::Approx(15.f));

    const float dt = 0.2f;
    const int moved = static_cast<int>(velocity * dt + 0.5f);  // 3 bars
    const Frame f = at(crest_t + dt, moved);
    CHECK(f.alpha == doctest::Approx(kMaxAlpha));
    CHECK(f.scale == doctest::Approx(kMaxScale));
}

TEST_CASE("time wraps by the period, so callers can pass raw elapsed seconds") {
    for (int i = 0; i < 10; ++i) {
        const Frame ref = at(0.31f, i);
        for (float k : {1.f, 5.f, 40.f}) {
            const Frame f = at(0.31f + kPeriodSeconds * k, i);
            CHECK(f.alpha == doctest::Approx(ref.alpha).epsilon(0.001));
            CHECK(f.scale == doctest::Approx(ref.scale).epsilon(0.001));
        }
    }
}

TEST_CASE("the wavelength puts about one crest on a screenful of bars") {
    // 2*pi / 0.4 ~= 15.7 bars. Much shorter would strobe; much longer and the
    // whole series would breathe in unison with no travel visible.
    const float wavelength = 6.2831853f / kSpatialRadPerBar;
    CHECK(wavelength == doctest::Approx(15.7f).epsilon(0.01));

    const float crest_t = 1.5707963f / kTemporalRadPerSecond;
    // One wavelength along the series is the same point in the cycle again.
    const Frame f = at(crest_t, 16);
    CHECK(f.alpha == doctest::Approx(kMaxAlpha).epsilon(0.01));
}

TEST_CASE("the scale range is symmetric about resting size") {
    // A wave that only grew bars would inflate the skeleton's average
    // silhouette relative to the real series it hands off to.
    CHECK(kMaxScale - 1.f == doctest::Approx(1.f - kMinScale));
}
