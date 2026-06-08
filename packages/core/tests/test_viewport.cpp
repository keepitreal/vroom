#include "doctest.h"

#include "viewport.h"

using vroom::Layout;
using vroom::PriceBounds;
using vroom::IndexRange;

namespace {

// A simple layout with no axis/padding reservations: usable width == width,
// candle area height == height. Individual tests override fields as needed.
Layout make_layout() {
    Layout l{};
    l.width_px = 1000.f;
    l.height_px = 1000.f;
    l.y_axis_width_px = 0.f;
    l.x_axis_height_px = 0.f;
    l.right_padding_px = 0.f;
    l.candle_width_ratio = 1.f;
    l.top_padding_frac = 0.f;
    l.bottom_padding_frac = 0.f;
    return l;
}

// Candle with explicit time + low/high; OHLC/volume default to something sane.
VroomCandle ohlc(int64_t t, double low, double high) {
    return VroomCandle{t, low, high, low, high, 0.0};
}

}  // namespace

TEST_CASE("candle_body_width") {
    Layout l = make_layout();

    SUBCASE("degenerate args return 0") {
        CHECK(vroom::candle_body_width(l, 0, 100) == 0.f);
        CHECK(vroom::candle_body_width(l, -1, 100) == 0.f);
        CHECK(vroom::candle_body_width(l, 1000, 0) == 0.f);
        CHECK(vroom::candle_body_width(l, 1000, -5) == 0.f);
    }

    SUBCASE("known geometry") {
        // usable = 1000, slot = 1000 * 100/1000 = 100, body = 100 * ratio(1)
        CHECK(vroom::candle_body_width(l, 1000, 100) == doctest::Approx(100.f));
    }

    SUBCASE("subtracts y-axis and right padding from usable width") {
        l.y_axis_width_px = 80.f;
        l.right_padding_px = 20.f;  // usable = 900
        // slot = 900 * 100/1000 = 90
        CHECK(vroom::candle_body_width(l, 1000, 100) == doctest::Approx(90.f));
    }

    SUBCASE("respects candle_width_ratio") {
        l.candle_width_ratio = 0.5f;
        CHECK(vroom::candle_body_width(l, 1000, 100) == doctest::Approx(50.f));
    }
}

TEST_CASE("candle_center_x") {
    Layout l = make_layout();

    SUBCASE("degenerate window returns 0") {
        CHECK(vroom::candle_center_x(l, 0, 100, 0, 0) == 0.f);
    }

    SUBCASE("uses the candle period midpoint") {
        // window 1000, duration 100, start 0: candle at t=0 has center 50px.
        CHECK(vroom::candle_center_x(l, 0, 100, 0, 1000) ==
              doctest::Approx(50.f));
        // candle at t=900 -> center_time 950 -> 950px.
        CHECK(vroom::candle_center_x(l, 900, 100, 0, 1000) ==
              doctest::Approx(950.f));
    }

    SUBCASE("a candle centered in the window lands mid-usable") {
        // window 200, duration 100, candle t=50 -> center_time 100 -> frac 0.5.
        CHECK(vroom::candle_center_x(l, 50, 100, 0, 200) ==
              doctest::Approx(500.f));
    }
}

TEST_CASE("snap_x_to_candle") {
    Layout l = make_layout();
    // 10 candles at t = 0,100,...,900; duration 100; window 1000; usable 1000.
    VroomCandle candles[10];
    for (int i = 0; i < 10; ++i) candles[i] = ohlc(i * 100, 1.0, 2.0);

    SUBCASE("no candles returns x unchanged") {
        CHECK(vroom::snap_x_to_candle(l, candles, 0, 100, 0, 1000, 42.f) ==
              42.f);
    }

    SUBCASE("degenerate window returns x unchanged") {
        CHECK(vroom::snap_x_to_candle(l, candles, 10, 100, 0, 0, 42.f) == 42.f);
    }

    SUBCASE("left of first clamps to first candle center") {
        CHECK(vroom::snap_x_to_candle(l, candles, 10, 100, 0, 1000, -50.f) ==
              doctest::Approx(50.f));
    }

    SUBCASE("right of last clamps to last candle center") {
        CHECK(vroom::snap_x_to_candle(l, candles, 10, 100, 0, 1000, 2000.f) ==
              doctest::Approx(950.f));
    }

    SUBCASE("snaps to the nearer of two candles") {
        // x=140 -> target 140 -> key 90, closer to candle t=100 (center 150).
        CHECK(vroom::snap_x_to_candle(l, candles, 10, 100, 0, 1000, 140.f) ==
              doctest::Approx(150.f));
        // x=90 -> target 90 -> key 40, closer to candle t=0 (center 50).
        CHECK(vroom::snap_x_to_candle(l, candles, 10, 100, 0, 1000, 90.f) ==
              doctest::Approx(50.f));
    }
}

TEST_CASE("visible_indices") {
    VroomCandle candles[5];
    for (int i = 0; i < 5; ++i) candles[i] = ohlc(i * 100, 1.0, 2.0);

    SUBCASE("empty returns {0,0}") {
        IndexRange r = vroom::visible_indices(candles, 0, 100, 300);
        CHECK(r.start == 0);
        CHECK(r.end == 0);
    }

    SUBCASE("(0,0) returns the full range") {
        IndexRange r = vroom::visible_indices(candles, 5, 0, 0);
        CHECK(r.start == 0);
        CHECK(r.end == 5);
    }

    SUBCASE("sub-slice is half-open [first,last)") {
        // times 0,100,200,300,400; [100,300] -> indices 1,2,3 -> {1,4}.
        IndexRange r = vroom::visible_indices(candles, 5, 100, 300);
        CHECK(r.start == 1);
        CHECK(r.end == 4);
    }

    SUBCASE("bounds outside the data clamp to the ends") {
        IndexRange r = vroom::visible_indices(candles, 5, -100, 1000);
        CHECK(r.start == 0);
        CHECK(r.end == 5);
    }
}

TEST_CASE("price_bounds") {
    SUBCASE("empty defaults to {0,1}") {
        PriceBounds b = vroom::price_bounds(nullptr, 0);
        CHECK(b.min == 0.0);
        CHECK(b.max == 1.0);
    }

    SUBCASE("min over lows, max over highs") {
        VroomCandle candles[3] = {
            ohlc(0, 5.0, 15.0),
            ohlc(1, 3.0, 20.0),
            ohlc(2, 8.0, 12.0),
        };
        PriceBounds b = vroom::price_bounds(candles, 3);
        CHECK(b.min == doctest::Approx(3.0));
        CHECK(b.max == doctest::Approx(20.0));
    }
}

TEST_CASE("price_to_y") {
    Layout l = make_layout();  // candle area = full 1000px height, no padding
    PriceBounds b{0.0, 100.0};

    SUBCASE("inverted: high price -> small y, low price -> large y") {
        CHECK(vroom::price_to_y(l, b, 100.0) == doctest::Approx(0.f));
        CHECK(vroom::price_to_y(l, b, 0.0) == doctest::Approx(1000.f));
        CHECK(vroom::price_to_y(l, b, 50.0) == doctest::Approx(500.f));
    }

    SUBCASE("zero range returns the vertical midpoint") {
        PriceBounds flat{50.0, 50.0};
        CHECK(vroom::price_to_y(l, flat, 50.0) == doctest::Approx(500.f));
    }

    SUBCASE("honors top/bottom padding fractions") {
        l.top_padding_frac = 0.1f;
        l.bottom_padding_frac = 0.1f;  // draw band 100..900
        CHECK(vroom::price_to_y(l, b, 100.0) == doctest::Approx(100.f));
        CHECK(vroom::price_to_y(l, b, 0.0) == doctest::Approx(900.f));
    }

    SUBCASE("reserves the x-axis strip from the candle area") {
        l.x_axis_height_px = 200.f;  // candle area = 800px
        CHECK(vroom::price_to_y(l, b, 0.0) == doctest::Approx(800.f));
        CHECK(vroom::price_to_y(l, b, 100.0) == doctest::Approx(0.f));
    }
}
