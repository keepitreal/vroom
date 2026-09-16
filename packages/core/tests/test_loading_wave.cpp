#include "doctest.h"

#include <cmath>
#include <set>

#include "loading_wave.h"

using vroom::loading_wave::at;
using vroom::loading_wave::breath;
using vroom::loading_wave::kAmplitudeFrac;
using vroom::loading_wave::kBreathMax;
using vroom::loading_wave::kBreathMin;
using vroom::loading_wave::kDriftRadPerSecond;
using vroom::loading_wave::kHarmonicCount;
using vroom::loading_wave::kHarmonics;
using vroom::loading_wave::kPeriodSeconds;
using vroom::loading_wave::kTwoPi;
using vroom::loading_wave::sample_count;
using vroom::loading_wave::y_frac;

TEST_CASE("the weights sum to one so the offset stays in -1..1") {
    float total = 0.f;
    for (int i = 0; i < kHarmonicCount; ++i) total += kHarmonics[i].weight;
    CHECK(total == doctest::Approx(1.f));

    for (int xi = 0; xi <= 60; ++xi) {
        for (int ti = 0; ti <= 60; ++ti) {
            const float v = at(static_cast<float>(ti) * 0.43f,
                               static_cast<float>(xi) / 60.f);
            CHECK(v >= -1.f);
            CHECK(v <= 1.f);
        }
    }
}

TEST_CASE("no component is a harmonic of another") {
    // The whole point of three sines is that they never lock into one repeating
    // shape. Integer frequency ratios would do exactly that, so guard them.
    for (int i = 0; i < kHarmonicCount; ++i) {
        for (int j = i + 1; j < kHarmonicCount; ++j) {
            const float ratio = kHarmonics[j].freq / kHarmonics[i].freq;
            const float nearest = std::round(ratio);
            CHECK(std::fabs(ratio - nearest) > 0.05f);
        }
    }
}

TEST_CASE("the components drift at different rates") {
    // Equal drifts would slide one rigid shape across the plot, which is the
    // single-sine look this replaced.
    std::set<float> drifts;
    for (int i = 0; i < kHarmonicCount; ++i) drifts.insert(kHarmonics[i].drift);
    CHECK(drifts.size() == static_cast<std::size_t>(kHarmonicCount));
}

TEST_CASE("the curve travels toward the right") {
    // Each component's phase is (k*x - w*t), so a feature at x after dt sits
    // further right. Checking the carrier alone is enough to fix the direction.
    const auto& carrier = kHarmonics[0];
    const float dt = 0.5f;
    const float x0 = (kTwoPi * 0.25f) / carrier.freq;
    const float x1 = (kTwoPi * 0.25f +
                      carrier.drift * kDriftRadPerSecond * dt) / carrier.freq;
    CHECK(x1 > x0);
}

TEST_CASE("a full period returns the whole curve to where it started") {
    // This is what makes wrapping the phase clock seamless. It only holds
    // because the drifts are commensurate; if that changes, this catches it.
    for (int xi = 0; xi <= 24; ++xi) {
        const float x = static_cast<float>(xi) / 24.f;
        const float ref = at(0.31f, x);
        for (float k : {1.f, 3.f, 12.f}) {
            CHECK(at(0.31f + kPeriodSeconds * k, x) ==
                  doctest::Approx(ref).epsilon(0.002));
        }
    }
}

TEST_CASE("the curve has price-action texture, not a lone swell") {
    // Count local extremes across the plot. Too few and it reads as decorative
    // next to candlesticks; too many and it reads as noise.
    const auto extremes = [](float t) {
        int n = 0;
        float prev = 0.f;
        bool have_prev = false;
        for (int i = 1; i <= 600; ++i) {
            const float d = at(t, static_cast<float>(i) / 600.f) -
                            at(t, static_cast<float>(i - 1) / 600.f);
            if (have_prev && ((d > 0.f) != (prev > 0.f))) ++n;
            prev = d;
            have_prev = true;
        }
        return n;
    };
    for (int i = 0; i <= 10; ++i) {
        const int n = extremes(kPeriodSeconds * static_cast<float>(i) / 11.f);
        CHECK(n >= 6);
        CHECK(n <= 16);
    }
}

TEST_CASE("the shape keeps rearranging within a period") {
    // A rigid travelling shape would reproduce its t=0 profile at some later
    // offset. Sample the whole curve at intervals through one period and check
    // no two profiles match.
    const auto profile_distance = [](float ta, float tb) {
        float worst = 0.f;
        for (int xi = 0; xi <= 40; ++xi) {
            const float x = static_cast<float>(xi) / 40.f;
            worst = std::max(worst, std::fabs(at(ta, x) - at(tb, x)));
        }
        return worst;
    };
    for (int i = 1; i <= 12; ++i) {
        const float t = kPeriodSeconds * static_cast<float>(i) / 13.f;
        CHECK(profile_distance(0.f, t) > 0.05f);
    }
}

TEST_CASE("y_frac centres the curve on the pane and stays well inside it") {
    for (int xi = 0; xi <= 40; ++xi) {
        for (int ti = 0; ti <= 30; ++ti) {
            const float y = y_frac(static_cast<float>(ti) * 0.83f,
                                   static_cast<float>(xi) / 40.f);
            CHECK(y >= 0.5f - kAmplitudeFrac);
            CHECK(y <= 0.5f + kAmplitudeFrac);
        }
    }
    // Restraint is the point: a crest should stay nowhere near the pane edges.
    CHECK(kAmplitudeFrac > 0.02f);
    CHECK(kAmplitudeFrac < 0.15f);
}

TEST_CASE("y_frac tracks the offset it is built from") {
    for (int xi = 0; xi <= 10; ++xi) {
        const float x = static_cast<float>(xi) / 10.f;
        CHECK(y_frac(1.7f, x) ==
              doctest::Approx(0.5f + kAmplitudeFrac * at(1.7f, x)));
    }
}

TEST_CASE("the breath stays faint and within its bounds") {
    // A pulse around full strength rather than a dimmer: the color decides how
    // quiet the line reads (it inherits the recessive gridline tone), so the
    // breath must not scale it down far enough to lose it against the
    // background.
    CHECK(kBreathMin > 0.5f);
    CHECK(kBreathMax <= 1.f);
    CHECK(kBreathMax > kBreathMin);
    for (int ti = 0; ti <= 200; ++ti) {
        const float b = breath(static_cast<float>(ti) * 0.17f);
        CHECK(b >= kBreathMin - 1e-5f);
        CHECK(b <= kBreathMax + 1e-5f);
    }
}

TEST_CASE("the breath wraps with the phase clock") {
    for (int ti = 0; ti <= 20; ++ti) {
        const float t = static_cast<float>(ti) * 0.61f;
        CHECK(breath(t + kPeriodSeconds) ==
              doctest::Approx(breath(t)).epsilon(0.002));
    }
}

TEST_CASE("the breath and the drift do not pulse together") {
    // Sharing a period would collapse the two into one visible beat.
    const float breath_period =
        kPeriodSeconds / vroom::loading_wave::kBreathCyclesPerPeriod;
    const float drift_period = kTwoPi / kDriftRadPerSecond;
    CHECK(std::fabs(breath_period - drift_period) > 0.5f);
}

TEST_CASE("a still curve sits mid-breath") {
    // Reduced motion pins elapsed time at 0, and the result has to be a
    // visible line rather than one caught at the bottom of its fade.
    CHECK(breath(0.f) == doctest::Approx((kBreathMin + kBreathMax) * 0.5f));
}

TEST_CASE("the drift is slow enough to read as waiting") {
    // Regression guard on feel: two earlier revisions ran several times this
    // fast and read as frantic.
    const float cross_seconds =
        kHarmonics[0].freq / (kHarmonics[0].drift * kDriftRadPerSecond);
    CHECK(cross_seconds > 4.f);
    CHECK(cross_seconds < 20.f);
}

TEST_CASE("sample_count resolves the finest component at any width") {
    // The floor is the point of this function: even a narrow chart has to
    // carry enough vertices that the tightest harmonic isn't aliased away.
    const float crests = vroom::loading_wave::max_freq() / kTwoPi;
    for (float w : {-100.f, 0.f, 120.f, 480.f, 1280.f, 4000.f, 100000.f}) {
        CHECK(static_cast<float>(sample_count(w)) >=
              crests * vroom::loading_wave::kSamplesPerCycle);
    }
}

TEST_CASE("sample_count grows with width but stays bounded") {
    CHECK(sample_count(1600.f) > sample_count(400.f));
    CHECK(sample_count(100000.f) == sample_count(200000.f));
    CHECK(sample_count(100000.f) < 400);  // cheap enough to spline every frame
}

TEST_CASE("max_freq finds the tightest component") {
    for (int i = 0; i < kHarmonicCount; ++i) {
        CHECK(kHarmonics[i].freq <= vroom::loading_wave::max_freq());
    }
}
