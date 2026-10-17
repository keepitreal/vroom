#include "doctest.h"

#include <algorithm>

#include "tip_geometry.h"
#include "viewport.h"

using vroom::tip_geometry::gutter_px;
using vroom::tip_geometry::kClearPx;
using vroom::tip_geometry::of;
using vroom::tip_geometry::reaches_clip;

TEST_CASE("the dot scales with the stroke and floors at a visible size") {
    CHECK(of(3.f).dot_r == doctest::Approx(4.5f));

    SUBCASE("hairline widths keep a 2px dot rather than vanishing") {
        CHECK(of(0.5f).dot_r == doctest::Approx(2.f));
        CHECK(of(1.f).dot_r == doctest::Approx(2.f));
    }

    SUBCASE("a missing width falls back to the theme default") {
        CHECK(of(0.f).dot_r == of(1.5f).dot_r);
    }
}

TEST_CASE("an explicit radius overrides the stroke") {
    CHECK(of(3.f, 6.f).dot_r == doctest::Approx(6.f));
    CHECK(of(0.5f, 1.f).dot_r == doctest::Approx(1.f));

    SUBCASE("zero or negative follows the stroke, as when unset") {
        CHECK(of(3.f, 0.f).dot_r == of(3.f).dot_r);
        CHECK(of(3.f, -1.f).dot_r == of(3.f).dot_r);
    }
}

TEST_CASE("the gutter clears the dot at every line width") {
    for (const float w : {0.5f, 1.f, 1.5f, 3.f, 8.f}) {
        CHECK(gutter_px(w) >= of(w).dot_r + kClearPx);
    }
    CHECK(gutter_px(3.f, 6.f) == doctest::Approx(6.f + kClearPx));
}

// The regression this file exists for: with the view pinned to the latest bar,
// the tip dot was clipped in half by the y-axis. Model the worst case for a view
// following the latest bar — the newest candle's center sits exactly on
// candle_right — and check the dot still lands clear of the axis strip.
TEST_CASE("the tip dot clears the y-axis when the view sits on the newest bar") {
    constexpr float kWidth = 390.f;   // phone-width plot
    constexpr float kAxisW = 56.f;    // price labels
    constexpr float kLineWidth = 3.f; // the width the bug was reported at

    vroom::Layout lay{};
    lay.width_px = kWidth;
    lay.height_px = 600.f;
    lay.y_axis_width_px = kAxisW;
    lay.right_padding_px = std::max(6.f, gutter_px(kLineWidth));

    const float candle_right = vroom::candle_area_width(lay);
    const float axis_left = kWidth - kAxisW;
    const float tip_x = candle_right;  // worst case while following the latest bar
    const float dot_right = tip_x + of(kLineWidth).dot_r;

    CHECK(dot_right < axis_left);
    CHECK(axis_left - dot_right >= doctest::Approx(kClearPx));

    SUBCASE("the pulse ring is not covered by the guarantee and may clip") {
        // 3.5x the dot radius at its widest (tip_pulse.h). Reserving that
        // much gutter would cost the plot ~11px, so the ring is allowed to run
        // into the axis; only the dot is promised.
        CHECK(tip_x + of(kLineWidth).dot_r * 3.5f > axis_left);
    }
}

TEST_CASE("an explicit dot radius widens the gutter to keep the same clearance") {
    constexpr float kWidth = 390.f;
    constexpr float kAxisW = 56.f;
    constexpr float kDotR = 6.f;

    vroom::Layout lay{};
    lay.width_px = kWidth;
    lay.height_px = 600.f;
    lay.y_axis_width_px = kAxisW;
    lay.right_padding_px = std::max(6.f, gutter_px(3.f, kDotR));

    const float axis_left = kWidth - kAxisW;
    const float dot_right = vroom::candle_area_width(lay) + of(3.f, kDotR).dot_r;
    CHECK(axis_left - dot_right >= doctest::Approx(kClearPx));
}

// Panning into history carries the tip past the plot's right edge. It used to
// vanish the moment its center crossed candle_right, with half the dot and most
// of the ring still on screen.
TEST_CASE("the marker keeps drawing until none of it is left in the clip") {
    constexpr float kRight = 334.f;  // the y-axis strip's left edge
    constexpr float kBottom = 500.f;
    constexpr float kDotR = 6.f;
    constexpr float kRingR = kDotR * 3.5f;  // the pulse at its widest

    SUBCASE("past the right edge, until its full radius has crossed it") {
        CHECK(reaches_clip(kRight + 1.f, 250.f, kDotR, kRight, kBottom));
        CHECK(reaches_clip(kRight + kDotR - 0.1f, 250.f, kDotR, kRight, kBottom));
        CHECK_FALSE(reaches_clip(kRight + kDotR, 250.f, kDotR, kRight, kBottom));
    }

    SUBCASE("the ring holds it on screen after the dot has gone") {
        CHECK(reaches_clip(kRight + 15.f, 250.f, kRingR, kRight, kBottom));
        CHECK_FALSE(reaches_clip(kRight + 15.f, 250.f, kDotR, kRight, kBottom));
        CHECK_FALSE(reaches_clip(kRight + kRingR, 250.f, kRingR, kRight, kBottom));
    }

    SUBCASE("every edge, the same way") {
        CHECK(reaches_clip(-kDotR + 0.1f, 250.f, kDotR, kRight, kBottom));
        CHECK_FALSE(reaches_clip(-kDotR, 250.f, kDotR, kRight, kBottom));
        CHECK(reaches_clip(100.f, -kDotR + 0.1f, kDotR, kRight, kBottom));
        CHECK_FALSE(reaches_clip(100.f, kBottom + kDotR, kDotR, kRight, kBottom));
    }

    SUBCASE("off a corner, by distance rather than per axis") {
        // 5px out on both axes is within 6px per axis but ~7.07px away.
        CHECK_FALSE(reaches_clip(kRight + 5.f, -5.f, kDotR, kRight, kBottom));
        CHECK(reaches_clip(kRight + 4.f, -4.f, kDotR, kRight, kBottom));
    }

    SUBCASE("anywhere inside counts") {
        CHECK(reaches_clip(100.f, 250.f, kDotR, kRight, kBottom));
        CHECK(reaches_clip(kRight, 0.f, kDotR, kRight, kBottom));
    }
}
