#include "doctest.h"

#include <string>

#include "crosshair_style.h"

namespace cs = vroom::crosshair_style;

namespace {
VroomCrosshairStyle unset(int32_t mode) {
    VroomCrosshairStyle s{};
    s.mode = mode;
    s.line_style = -1;
    s.time_format = -1;
    s.simple_height = -1.f;
    s.dim_opacity = -1.f;
    return s;
}

std::tm at(int mon, int mday, int hour, int min) {
    std::tm t{};
    t.tm_mon = mon;
    t.tm_mday = mday;
    t.tm_hour = hour;
    t.tm_min = min;
    return t;
}

std::string fmt(const std::tm& t, int32_t format, bool hour12) {
    char buf[32];
    cs::format_tm(t, format, hour12, buf, sizeof(buf));
    return buf;
}
}  // namespace

TEST_CASE("crosshair_style::resolve") {
    SUBCASE("zero-initialized is the original advanced crosshair") {
        const auto s = cs::resolve(VroomCrosshairStyle{});
        CHECK(s.mode == VROOM_CROSSHAIR_ADVANCED);
        CHECK(s.line_style == VROOM_CROSSHAIR_LINE_DASHED);
        CHECK(s.line_width_px == doctest::Approx(1.f));
        CHECK(s.time_format == VROOM_CROSSHAIR_TIME_DATETIME);
        CHECK(s.hour12 == 0);
    }

    SUBCASE("advanced sentinels") {
        const auto s = cs::resolve(unset(VROOM_CROSSHAIR_ADVANCED));
        CHECK(s.line_style == VROOM_CROSSHAIR_LINE_DASHED);
        CHECK(s.time_format == VROOM_CROSSHAIR_TIME_DATETIME);
    }

    SUBCASE("simple sentinels") {
        const auto s = cs::resolve(unset(VROOM_CROSSHAIR_SIMPLE));
        CHECK(s.line_style == VROOM_CROSSHAIR_LINE_SOLID);
        CHECK(s.time_format == VROOM_CROSSHAIR_TIME_TIME);
        CHECK(s.simple_height == doctest::Approx(1.f));
        CHECK(s.dim_opacity == doctest::Approx(0.35f));
    }

    SUBCASE("explicit values are kept, out-of-range ones clamped") {
        VroomCrosshairStyle in = unset(VROOM_CROSSHAIR_SIMPLE);
        in.line_style = VROOM_CROSSHAIR_LINE_DOTTED;
        in.line_width_px = 2.f;
        in.time_format = VROOM_CROSSHAIR_TIME_DATE;
        in.simple_height = 1.5f;
        in.dim_opacity = 0.f;
        const auto s = cs::resolve(in);
        CHECK(s.line_style == VROOM_CROSSHAIR_LINE_DOTTED);
        CHECK(s.line_width_px == doctest::Approx(2.f));
        CHECK(s.time_format == VROOM_CROSSHAIR_TIME_DATE);
        CHECK(s.simple_height == doctest::Approx(1.f));
        CHECK(s.dim_opacity == doctest::Approx(0.f));
    }

    SUBCASE("an unknown mode falls back to advanced") {
        VroomCrosshairStyle in{};
        in.mode = 9;
        CHECK(cs::resolve(in).mode == VROOM_CROSSHAIR_ADVANCED);
    }
}

TEST_CASE("crosshair_style::format_tm") {
    const std::tm morning = at(9, 14, 9, 5);
    const std::tm evening = at(9, 14, 23, 30);
    const std::tm midnight = at(0, 2, 0, 7);
    const std::tm noon = at(11, 31, 12, 0);

    CHECK(fmt(morning, VROOM_CROSSHAIR_TIME_DATETIME, false) == "10/14 09:05");
    CHECK(fmt(evening, VROOM_CROSSHAIR_TIME_DATETIME, true) == "10/14 11:30 PM");
    CHECK(fmt(morning, VROOM_CROSSHAIR_TIME_TIME, false) == "09:05");
    CHECK(fmt(morning, VROOM_CROSSHAIR_TIME_TIME, true) == "9:05 AM");
    CHECK(fmt(midnight, VROOM_CROSSHAIR_TIME_TIME, true) == "12:07 AM");
    CHECK(fmt(noon, VROOM_CROSSHAIR_TIME_TIME, true) == "12:00 PM");
    CHECK(fmt(morning, VROOM_CROSSHAIR_TIME_DATE, false) == "Oct 14");
    CHECK(fmt(noon, VROOM_CROSSHAIR_TIME_DATE, true) == "Dec 31");
}

TEST_CASE("crosshair_style::simple_geometry") {
    SUBCASE("height 1 puts the badge at the top") {
        const auto g = cs::simple_geometry(400.f, 360.f, 20.f, 1.f);
        CHECK(g.badge_top == doctest::Approx(0.f));
        CHECK(g.line_top == doctest::Approx(20.f));
    }

    SUBCASE("height 0.5 puts the badge's top at the middle") {
        const auto g = cs::simple_geometry(400.f, 360.f, 20.f, 0.5f);
        CHECK(g.badge_top == doctest::Approx(200.f));
        CHECK(g.line_top == doctest::Approx(220.f));
    }

    SUBCASE("height 0 keeps the badge above the line's bottom") {
        const auto g = cs::simple_geometry(400.f, 360.f, 20.f, 0.f);
        CHECK(g.badge_top == doctest::Approx(340.f));
        CHECK(g.line_top == doctest::Approx(360.f));
    }

    SUBCASE("a chart shorter than the badge pins it to the top") {
        const auto g = cs::simple_geometry(10.f, 10.f, 20.f, 0.5f);
        CHECK(g.badge_top == doctest::Approx(0.f));
        CHECK(g.line_top == doctest::Approx(10.f));
    }
}
