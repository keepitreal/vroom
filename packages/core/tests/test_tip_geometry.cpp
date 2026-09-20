#include "doctest.h"

#include <algorithm>

#include "tip_geometry.h"
#include "viewport.h"

using vroom::tip_geometry::gutter_px;
using vroom::tip_geometry::kClearPx;
using vroom::tip_geometry::of;

TEST_CASE("the dot scales with the stroke and floors at a visible size") {
    CHECK(of(3.f).dot_r == doctest::Approx(4.5f));
    CHECK(of(3.f).border_r == doctest::Approx(6.5f));

    SUBCASE("hairline widths keep a 2px dot rather than vanishing") {
        CHECK(of(0.5f).dot_r == doctest::Approx(2.f));
        CHECK(of(1.f).dot_r == doctest::Approx(2.f));
    }

    SUBCASE("a missing width falls back to the theme default") {
        CHECK(of(0.f).dot_r == of(1.5f).dot_r);
    }
}

TEST_CASE("the gutter clears the dot at every line width") {
    for (const float w : {0.5f, 1.f, 1.5f, 3.f, 8.f}) {
        CHECK(gutter_px(w) >= of(w).border_r + kClearPx);
    }
}

// The regression this file exists for: with the view pinned to the latest bar,
// the tip dot was clipped in half by the y-axis. Model the worst case the
// renderer allows — draw_close_tip drops the marker once its center passes
// candle_right, so the center can sit exactly on that edge — and check the dot
// still lands clear of the axis strip.
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
    const float tip_x = candle_right;  // worst case the on-pane guard permits
    const float dot_right = tip_x + of(kLineWidth).border_r;

    CHECK(dot_right < axis_left);
    CHECK(axis_left - dot_right >= doctest::Approx(kClearPx));

    SUBCASE("the pulse ring is not covered by the guarantee and may clip") {
        // 3.5x the border radius at its widest (tip_pulse.h). Reserving that
        // much gutter would cost the plot ~17px, so the ring is allowed to run
        // into the axis; only the dot is promised.
        CHECK(tip_x + of(kLineWidth).border_r * 3.5f > axis_left);
    }
}
