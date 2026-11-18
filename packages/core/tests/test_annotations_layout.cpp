#include "doctest.h"

#include "annotations_layout.h"
#include "vroom/vroom_chart.h"

using namespace vroom::annotations;

TEST_CASE("marker_size falls back to the default") {
    CHECK(marker_size(0.f) == kDefaultMarkerSize);
    CHECK(marker_size(-3.f) == kDefaultMarkerSize);
    CHECK(marker_size(14.f) == 14.f);
}

TEST_CASE("triangle points the requested way around its center") {
    const auto up = triangle(VROOM_ANNOTATION_TRIANGLE_UP, 50.f, 100.f, 10.f);
    CHECK(up[0].x == 50.f);
    CHECK(up[0].y == 95.f);  // apex above center
    CHECK(up[1].y == 105.f);
    CHECK(up[2].y == 105.f);
    CHECK(up[1].x - up[2].x == 10.f);

    const auto down = triangle(VROOM_ANNOTATION_TRIANGLE_DOWN, 50.f, 100.f, 10.f);
    CHECK(down[0].y == 105.f);  // apex below center
    CHECK(down[1].y == 95.f);
}

TEST_CASE("label_left honors alignment") {
    CHECK(label_left(0, 100.f, 40.f) == 100.f);
    CHECK(label_left(1, 100.f, 40.f) == 80.f);
    CHECK(label_left(2, 100.f, 40.f) == 60.f);
}

TEST_CASE("overlaps_plot rejects spans entirely off one side") {
    CHECK(overlaps_plot(-20.f, -1.f, 300.f) == false);
    CHECK(overlaps_plot(301.f, 400.f, 300.f) == false);
    CHECK(overlaps_plot(-20.f, 10.f, 300.f));
    CHECK(overlaps_plot(290.f, 400.f, 300.f));
    CHECK(overlaps_plot(40.f, -40.f, 300.f));  // reversed endpoints
}
