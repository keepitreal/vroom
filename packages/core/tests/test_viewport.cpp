#include "doctest.h"

#include "viewport.h"
#include <initializer_list>

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

    SUBCASE("just past the last candle still snaps to it") {
        // x=965 -> target ~915, within half a period of the last candle
        // (t=900, center 950).
        CHECK(vroom::snap_x_to_candle(l, candles, 10, 100, 0, 1000, 965.f) ==
              doctest::Approx(950.f));
    }

    SUBCASE("past the last candle snaps onto the future grid") {
        // window 2000 leaves empty room past the last candle (t=900). x=550
        // maps to ~t=1100, snapping to the empty future slot at t=1100
        // (center 575) instead of clamping to the last candle.
        CHECK(vroom::snap_x_to_candle(l, candles, 10, 100, 0, 2000, 550.f) ==
              doctest::Approx(575.f));
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

TEST_CASE("time_at_x / x_at_time (free drawing-endpoint mapping)") {
    Layout l = make_layout();  // usable = 1000px

    SUBCASE("x_at_time maps linearly across the window") {
        // window 1000 starting at 0: t=0 -> 0px, t=500 -> 500px, t=1000 -> 1000px.
        CHECK(vroom::x_at_time(l, 0, 1000, 0) == doctest::Approx(0.f));
        CHECK(vroom::x_at_time(l, 0, 1000, 500) == doctest::Approx(500.f));
        CHECK(vroom::x_at_time(l, 0, 1000, 1000) == doctest::Approx(1000.f));
    }

    SUBCASE("time_at_x is the inverse of x_at_time") {
        CHECK(vroom::time_at_x(l, 0, 1000, 0.f) == 0);
        CHECK(vroom::time_at_x(l, 0, 1000, 500.f) == 500);
        CHECK(vroom::time_at_x(l, 0, 1000, 1000.f) == 1000);
    }

    SUBCASE("honors a non-zero visible start") {
        CHECK(vroom::x_at_time(l, 1000, 1000, 1500) == doctest::Approx(500.f));
        CHECK(vroom::time_at_x(l, 1000, 1000, 500.f) == 1500);
    }

    SUBCASE("subtracts y-axis and right padding from usable width") {
        l.y_axis_width_px = 80.f;
        l.right_padding_px = 20.f;  // usable = 900
        CHECK(vroom::x_at_time(l, 0, 900, 900) == doctest::Approx(900.f));
        CHECK(vroom::time_at_x(l, 0, 900, 900.f) == 900);
    }

    SUBCASE("round-trips for assorted times") {
        for (int64_t t : {0, 137, 500, 813, 1000}) {
            const float x = vroom::x_at_time(l, 0, 1000, t);
            CHECK(vroom::time_at_x(l, 0, 1000, x) == t);
        }
    }

    SUBCASE("degenerate window/usable falls back to the visible start") {
        CHECK(vroom::time_at_x(l, 42, 0, 500.f) == 42);
        Layout z = make_layout();
        z.width_px = 0.f;  // usable <= 0
        CHECK(vroom::time_at_x(z, 42, 1000, 500.f) == 42);
        CHECK(vroom::x_at_time(z, 0, 0, 100) == 0.f);
    }
}

TEST_CASE("snap_index_to_candle") {
    Layout l = make_layout();
    // 10 candles at t = 0,100,...,900; duration 100; window 1000; usable 1000.
    VroomCandle candles[10];
    for (int i = 0; i < 10; ++i) candles[i] = ohlc(i * 100, 1.0, 2.0);

    SUBCASE("left of first clamps to index 0") {
        CHECK(vroom::snap_index_to_candle(l, candles, 10, 100, 0, 1000, -50.f) ==
              0u);
    }

    SUBCASE("right of last clamps to the final index") {
        CHECK(vroom::snap_index_to_candle(l, candles, 10, 100, 0, 1000,
                                          2000.f) == 9u);
    }

    SUBCASE("returns the nearer of two candle indices") {
        // x=140 -> key 90, nearer to candle t=100 (index 1).
        CHECK(vroom::snap_index_to_candle(l, candles, 10, 100, 0, 1000, 140.f) ==
              1u);
        // x=90 -> key 40, nearer to candle t=0 (index 0).
        CHECK(vroom::snap_index_to_candle(l, candles, 10, 100, 0, 1000, 90.f) ==
              0u);
    }
}

TEST_CASE("snap_to_slot") {
    Layout l = make_layout();
    // 10 candles at t = 0,100,...,900; duration 100; usable 1000.
    VroomCandle candles[10];
    for (int i = 0; i < 10; ++i) candles[i] = ohlc(i * 100, 1.0, 2.0);

    SUBCASE("over a real candle reports the candle and its index") {
        // x=140 -> key 90, nearest real candle is t=100 (index 1).
        auto r = vroom::snap_to_slot(l, candles, 10, 100, 0, 1000, 140.f);
        CHECK(r.has_candle == true);
        CHECK(r.index == 1u);
        CHECK(r.time_ms == 100);
    }

    SUBCASE("left of first clamps to the first candle") {
        auto r = vroom::snap_to_slot(l, candles, 10, 100, 0, 1000, -50.f);
        CHECK(r.has_candle == true);
        CHECK(r.index == 0u);
        CHECK(r.time_ms == 0);
    }

    SUBCASE("just past the last candle still resolves to it") {
        // x=965 -> key ~915, within half a period of the last candle (t=900).
        auto r = vroom::snap_to_slot(l, candles, 10, 100, 0, 1000, 965.f);
        CHECK(r.has_candle == true);
        CHECK(r.index == 9u);
        CHECK(r.time_ms == 900);
    }

    SUBCASE("beyond the last candle snaps to an empty future slot") {
        // window 2000 leaves room past the last candle. x=550 -> key ~1050,
        // snapping to the empty grid slot at t=1100 (no candle there).
        auto r = vroom::snap_to_slot(l, candles, 10, 100, 0, 2000, 550.f);
        CHECK(r.has_candle == false);
        CHECK(r.time_ms == 1100);
    }

    SUBCASE("x clamps to the usable width (no off-screen future slots)") {
        // window 2000, x far past the right edge clamps to x=usable (1000),
        // i.e. target time = visible_end (2000) -> key ~1950 -> slot t=1950.
        auto r = vroom::snap_to_slot(l, candles, 10, 100, 0, 2000, 9000.f);
        auto edge = vroom::snap_to_slot(l, candles, 10, 100, 0, 2000, 1000.f);
        CHECK(r.has_candle == false);
        CHECK(r.time_ms == edge.time_ms);
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

TEST_CASE("auto_price_bounds") {
    SUBCASE("empty keeps the {0,1} sentinel unwidened") {
        PriceBounds b = vroom::auto_price_bounds(nullptr, 0);
        CHECK(b.min == 0.0);
        CHECK(b.max == 1.0);
    }

    SUBCASE("widens price_bounds about the midpoint by kAutoYZoom") {
        VroomCandle candles[2] = {
            ohlc(0, 10.0, 20.0),
            ohlc(1, 12.0, 30.0),
        };
        // raw bounds {10,30}: mid 20, half 10 -> widened half 15 -> {5,35}
        PriceBounds b = vroom::auto_price_bounds(candles, 2);
        CHECK(b.min == doctest::Approx(5.0));
        CHECK(b.max == doctest::Approx(35.0));
    }

    SUBCASE("flat candle collapses to a zero-height range at its price") {
        VroomCandle candles[1] = {ohlc(0, 50.0, 50.0)};
        PriceBounds b = vroom::auto_price_bounds(candles, 1);
        CHECK(b.min == doctest::Approx(50.0));
        CHECK(b.max == doctest::Approx(50.0));
    }
}

TEST_CASE("preserve_envelope_bounds") {
    SUBCASE("a halved envelope halves the axis range") {
        PriceBounds axis{0.0, 100.0};
        PriceBounds old_env{40.0, 60.0};  // span 20, mid 50 -> t = 0.5
        PriceBounds new_env{45.0, 55.0};  // span 10, mid 50
        PriceBounds b = vroom::preserve_envelope_bounds(axis, old_env, new_env);
        CHECK(b.min == doctest::Approx(25.0));
        CHECK(b.max == doctest::Approx(75.0));
    }

    SUBCASE("a doubled envelope doubles the axis range") {
        PriceBounds axis{0.0, 100.0};
        PriceBounds old_env{45.0, 55.0};  // span 10, mid 50 -> t = 0.5
        PriceBounds new_env{40.0, 60.0};  // span 20
        PriceBounds b = vroom::preserve_envelope_bounds(axis, old_env, new_env);
        CHECK(b.min == doctest::Approx(-50.0));
        CHECK(b.max == doctest::Approx(150.0));
    }

    SUBCASE("keeps the envelope's pixel height and position") {
        Layout l = make_layout();  // 1000px draw band, no padding
        PriceBounds axis{0.0, 100.0};
        PriceBounds old_env{70.0, 90.0};  // off-center, near the top
        PriceBounds new_env{20.0, 25.0};  // tighter and much lower
        const float old_top = vroom::price_to_y(l, axis, old_env.max);
        const float old_bot = vroom::price_to_y(l, axis, old_env.min);
        PriceBounds b = vroom::preserve_envelope_bounds(axis, old_env, new_env);
        CHECK(vroom::price_to_y(l, b, new_env.max) == doctest::Approx(old_top));
        CHECK(vroom::price_to_y(l, b, new_env.min) == doctest::Approx(old_bot));
    }

    SUBCASE("an unchanged envelope is the identity") {
        PriceBounds axis{10.0, 20.0};
        PriceBounds env{12.0, 18.0};
        PriceBounds b = vroom::preserve_envelope_bounds(axis, env, env);
        CHECK(b.min == doctest::Approx(10.0));
        CHECK(b.max == doctest::Approx(20.0));
    }

    SUBCASE("degenerate spans return the axis unchanged") {
        PriceBounds axis{0.0, 100.0};
        PriceBounds env{40.0, 60.0};
        PriceBounds flat{50.0, 50.0};

        PriceBounds no_old = vroom::preserve_envelope_bounds(axis, flat, env);
        CHECK(no_old.min == doctest::Approx(0.0));
        CHECK(no_old.max == doctest::Approx(100.0));

        PriceBounds no_new = vroom::preserve_envelope_bounds(axis, env, flat);
        CHECK(no_new.min == doctest::Approx(0.0));
        CHECK(no_new.max == doctest::Approx(100.0));

        PriceBounds flat_axis = vroom::preserve_envelope_bounds(flat, env, env);
        CHECK(flat_axis.min == doctest::Approx(50.0));
        CHECK(flat_axis.max == doctest::Approx(50.0));
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

TEST_CASE("y_to_price") {
    Layout l = make_layout();  // candle area = full 1000px height, no padding
    PriceBounds b{0.0, 100.0};

    SUBCASE("inverts price_to_y: small y -> high price, large y -> low price") {
        CHECK(vroom::y_to_price(l, b, 0.f) == doctest::Approx(100.0));
        CHECK(vroom::y_to_price(l, b, 1000.f) == doctest::Approx(0.0));
        CHECK(vroom::y_to_price(l, b, 500.f) == doctest::Approx(50.0));
    }

    SUBCASE("round-trips with price_to_y") {
        for (double price : {0.0, 12.5, 50.0, 87.3, 100.0}) {
            const float y = vroom::price_to_y(l, b, price);
            CHECK(vroom::y_to_price(l, b, y) == doctest::Approx(price));
        }
    }

    SUBCASE("zero range returns bounds.min") {
        PriceBounds flat{50.0, 50.0};
        CHECK(vroom::y_to_price(l, flat, 500.f) == doctest::Approx(50.0));
    }

    SUBCASE("honors top/bottom padding fractions") {
        l.top_padding_frac = 0.1f;
        l.bottom_padding_frac = 0.1f;  // draw band 100..900
        CHECK(vroom::y_to_price(l, b, 100.f) == doctest::Approx(100.0));
        CHECK(vroom::y_to_price(l, b, 900.f) == doctest::Approx(0.0));
    }
}
