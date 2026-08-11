#include "doctest.h"

#include "volume_anim.h"

#include <cstddef>
#include <iterator>

#include "vroom/vroom_chart.h"

using vroom::volume_anim::bar_collapse;
using vroom::volume_anim::ease;
using vroom::volume_anim::kMinWindow;

namespace {
constexpr int32_t kSmooth = VROOM_EASING_IN_OUT;
}  // namespace

TEST_CASE("ease matches the TS curves") {
    CHECK(ease(VROOM_EASING_LINEAR, 0.25f) == doctest::Approx(0.25f));
    CHECK(ease(VROOM_EASING_IN, 0.5f) == doctest::Approx(0.25f));
    CHECK(ease(VROOM_EASING_OUT, 0.5f) == doctest::Approx(0.75f));
    CHECK(ease(kSmooth, 0.5f) == doctest::Approx(0.5f));
    CHECK(ease(kSmooth, 0.25f) == doctest::Approx(0.15625f));

    SUBCASE("every curve pins both endpoints") {
        for (int32_t k = VROOM_EASING_LINEAR; k <= VROOM_EASING_IN_OUT; ++k) {
            CHECK(ease(k, 0.f) == doctest::Approx(0.f));
            CHECK(ease(k, 1.f) == doctest::Approx(1.f));
        }
    }

    SUBCASE("unknown kinds fall back to smoothstep, like the TS default") {
        CHECK(ease(99, 0.25f) == doctest::Approx(ease(kSmooth, 0.25f)));
        CHECK(ease(-1, 0.75f) == doctest::Approx(ease(kSmooth, 0.75f)));
    }

    SUBCASE("progress is clamped") {
        CHECK(ease(VROOM_EASING_LINEAR, -0.5f) == doctest::Approx(0.f));
        CHECK(ease(VROOM_EASING_LINEAR, 1.5f) == doctest::Approx(1.f));
    }
}

TEST_CASE("the tallest bar starts immediately") {
    CHECK(bar_collapse(1.f, 0.f, kSmooth) == doctest::Approx(0.f));
    CHECK(bar_collapse(1.f, 0.01f, kSmooth) > 0.f);
}

TEST_CASE("every bar lands together at t = 1") {
    for (const float frac : {1.f, 0.75f, 0.5f, 0.2f, 0.05f, 0.001f}) {
        CHECK(bar_collapse(frac, 1.f, kSmooth) == doctest::Approx(1.f));
        // ...and only at 1: a hair earlier it is still falling.
        CHECK(bar_collapse(frac, 0.999f, kSmooth) < 1.f);
    }
}

TEST_CASE("a bar holds full height until its window opens") {
    // frac 0.5 -> window [0.5, 1].
    CHECK(bar_collapse(0.5f, 0.49f, kSmooth) == doctest::Approx(0.f));
    CHECK(bar_collapse(0.5f, 0.5f, kSmooth) == doctest::Approx(0.f));
    CHECK(bar_collapse(0.5f, 0.51f, kSmooth) > 0.f);
    CHECK(bar_collapse(0.5f, 0.75f, kSmooth) == doctest::Approx(0.5f));
}

TEST_CASE("taller bars are always further along") {
    // Mid-transition the cascade has to be ordered by height, with no ties
    // except where both bars have already finished or not yet started.
    const float fracs[] = {1.f, 0.9f, 0.7f, 0.5f, 0.3f};
    for (const float t : {0.3f, 0.5f, 0.7f, 0.9f}) {
        for (std::size_t i = 1; i < std::size(fracs); ++i) {
            CHECK(bar_collapse(fracs[i - 1], t, kSmooth) >=
                  bar_collapse(fracs[i], t, kSmooth));
        }
    }
}

TEST_CASE("kMinWindow keeps the shortest bars from popping") {
    // A near-zero bar would otherwise fall inside one frame. It gets the floor
    // window instead, which starts earlier but still ends at 1.
    const float tiny = 0.001f;
    CHECK(bar_collapse(tiny, 1.f - kMinWindow - 0.01f, kSmooth) ==
          doctest::Approx(0.f));
    CHECK(bar_collapse(tiny, 1.f - kMinWindow + 0.01f, kSmooth) > 0.f);
    CHECK(bar_collapse(tiny, 1.f, kSmooth) == doctest::Approx(1.f));

    SUBCASE("bars at or above the floor are unaffected by it") {
        CHECK(bar_collapse(kMinWindow, 1.f - kMinWindow, kSmooth) ==
              doctest::Approx(0.f));
        CHECK(bar_collapse(0.4f, 0.6f, kSmooth) == doctest::Approx(0.f));
        CHECK(bar_collapse(0.4f, 0.61f, kSmooth) > 0.f);
    }
}

TEST_CASE("played backwards the shortest bar finishes rising first") {
    // Reveal drives t from 1 down to 0. Partway back the short bar is already
    // home (collapse 0) while the tallest is still on its way up.
    CHECK(bar_collapse(0.25f, 0.7f, kSmooth) == doctest::Approx(0.f));
    CHECK(bar_collapse(1.f, 0.7f, kSmooth) > 0.f);
}
