#include "doctest.h"

#include "crosshair_button.h"

using vroom::crosshair_button::Rect;

namespace {
VroomCrosshairButtonStyle style_with(float size, float radius) {
    VroomCrosshairButtonStyle s{};
    s.enabled = 1;
    s.size_px = size;
    s.corner_radius_px = radius;
    s.gap_px = -1.f;
    return vroom::crosshair_button::resolve(s);
}
}  // namespace

TEST_CASE("crosshair_button::resolve") {
    SUBCASE("sentinels fall back to defaults") {
        VroomCrosshairButtonStyle in{};
        in.corner_radius_px = -1.f;
        in.gap_px = -1.f;
        const auto s = vroom::crosshair_button::resolve(in);
        CHECK(s.size_px == doctest::Approx(20.f));
        CHECK(s.corner_radius_px == doctest::Approx(4.f));
        CHECK(s.icon_stroke_px == doctest::Approx(1.5f));
        CHECK(s.gap_px == doctest::Approx(4.f));
        CHECK(s.hover_boost == doctest::Approx(1.25f));
    }

    SUBCASE("size is clamped and the radius never exceeds a circle") {
        CHECK(style_with(4.f, 0.f).size_px == doctest::Approx(12.f));
        CHECK(style_with(100.f, 0.f).size_px == doctest::Approx(48.f));
        CHECK(style_with(20.f, 50.f).corner_radius_px == doctest::Approx(10.f));
        CHECK(style_with(20.f, 0.f).corner_radius_px == doctest::Approx(0.f));
    }
}

TEST_CASE("crosshair_button::button_rect") {
    const auto s = style_with(20.f, 4.f);  // gap 4

    SUBCASE("sits left of the anchor, centered on the line") {
        const Rect r = vroom::crosshair_button::button_rect(500.f, 200.f, 0.f, 400.f, s);
        CHECK(r.right == doctest::Approx(496.f));
        CHECK(r.left == doctest::Approx(476.f));
        CHECK(r.top == doctest::Approx(190.f));
        CHECK(r.bottom == doctest::Approx(210.f));
    }

    SUBCASE("stays inside the pane near its edges") {
        const Rect top = vroom::crosshair_button::button_rect(500.f, 3.f, 0.f, 400.f, s);
        CHECK(top.top == doctest::Approx(0.f));
        const Rect bot = vroom::crosshair_button::button_rect(500.f, 399.f, 0.f, 400.f, s);
        CHECK(bot.bottom == doctest::Approx(400.f));
    }

    SUBCASE("contains") {
        const Rect r = vroom::crosshair_button::button_rect(500.f, 200.f, 0.f, 400.f, s);
        CHECK(vroom::crosshair_button::contains(r, 486.f, 200.f));
        CHECK_FALSE(vroom::crosshair_button::contains(r, 470.f, 200.f));
        CHECK_FALSE(vroom::crosshair_button::contains(r, 486.f, 215.f));
    }
}
