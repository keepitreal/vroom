#include "doctest.h"

#include "crosshair_button.h"

using vroom::crosshair_button::Rect;

namespace {
VroomCrosshairButtonStyle style_with(float radius) {
    VroomCrosshairButtonStyle s{};
    s.enabled = 1;
    s.corner_radius_px = radius;
    return vroom::crosshair_button::resolve(s);
}
}  // namespace

TEST_CASE("crosshair_button::resolve") {
    SUBCASE("sentinels fall back to defaults") {
        const auto s = style_with(-1.f);
        CHECK(s.corner_radius_px == doctest::Approx(6.f));
        CHECK(s.hover_boost == doctest::Approx(1.25f));
    }

    SUBCASE("explicit values are kept") {
        VroomCrosshairButtonStyle in{};
        in.corner_radius_px = 0.f;
        in.hover_boost = 1.f;
        const auto s = vroom::crosshair_button::resolve(in);
        CHECK(s.corner_radius_px == doctest::Approx(0.f));
        CHECK(s.hover_boost == doctest::Approx(1.f));
    }
}

TEST_CASE("crosshair_button::corner_radius") {
    CHECK(vroom::crosshair_button::corner_radius(style_with(-1.f), 20.f) ==
          doctest::Approx(6.f));
    CHECK(vroom::crosshair_button::corner_radius(style_with(999.f), 20.f) ==
          doctest::Approx(10.f));
    CHECK(vroom::crosshair_button::corner_radius(style_with(0.f), 20.f) ==
          doctest::Approx(0.f));
}

TEST_CASE("crosshair_button::pill_rect") {
    // Text spans 500..560; 10px plus, 8px pad, 6px gap, 20px tall.
    const Rect r =
        vroom::crosshair_button::pill_rect(500.f, 560.f, 200.f, 20.f, 10.f, 8.f, 6.f);

    SUBCASE("keeps the text in place and grows leftward for the plus") {
        CHECK(r.right == doctest::Approx(568.f));
        CHECK(r.left == doctest::Approx(476.f));
        CHECK(r.top == doctest::Approx(190.f));
        CHECK(r.bottom == doctest::Approx(210.f));
        CHECK(vroom::crosshair_button::plus_center_x(r, 10.f, 8.f) ==
              doctest::Approx(489.f));
    }

    SUBCASE("no glyph and no gap is the plain badge") {
        const Rect plain = vroom::crosshair_button::pill_rect(500.f, 560.f, 200.f,
                                                              20.f, 0.f, 8.f, 0.f);
        CHECK(plain.left == doctest::Approx(492.f));
        CHECK(plain.right == doctest::Approx(568.f));
    }

    SUBCASE("plus-only pill ends at the anchor") {
        const Rect only = vroom::crosshair_button::pill_rect(592.f, 592.f, 200.f,
                                                             20.f, 10.f, 8.f, 0.f);
        CHECK(only.right == doctest::Approx(600.f));
        CHECK(only.left == doctest::Approx(574.f));
    }

    SUBCASE("contains covers the plus and the price") {
        CHECK(vroom::crosshair_button::contains(r, 489.f, 200.f));
        CHECK(vroom::crosshair_button::contains(r, 540.f, 200.f));
        CHECK_FALSE(vroom::crosshair_button::contains(r, 470.f, 200.f));
        CHECK_FALSE(vroom::crosshair_button::contains(r, 540.f, 215.f));
    }
}
