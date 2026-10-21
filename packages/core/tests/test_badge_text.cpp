#include "doctest.h"

#include "theme.h"

TEST_CASE("badge_text_color falls back to the theme's BADGE_TEXT") {
    vroom::Theme t;
    t.colors[VROOM_COLOR_BADGE_TEXT] = 0xffffffff;

    SUBCASE("0 inherits") {
        CHECK(vroom::badge_text_color(t, 0u) == 0xffffffffu);
    }
    SUBCASE("any fully transparent value inherits") {
        CHECK(vroom::badge_text_color(t, 0x00123456u) == 0xffffffffu);
    }
    SUBCASE("an opaque override wins") {
        CHECK(vroom::badge_text_color(t, 0xff0b0f14u) == 0xff0b0f14u);
    }
    SUBCASE("a translucent override keeps its alpha") {
        CHECK(vroom::badge_text_color(t, 0x800b0f14u) == 0x800b0f14u);
    }
}
