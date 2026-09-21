#include "doctest.h"

#include "price_indicator_anim.h"
#include "viewport.h"

using vroom::CandleSnapshot;
using vroom::PriceBounds;
using vroom::price_indicator_anim::Level;
using vroom::price_indicator_anim::level_at;

namespace {

vroom::Layout make_layout() {
    vroom::Layout lay{};
    lay.width_px = 390.f;
    lay.height_px = 600.f;
    lay.y_axis_width_px = 56.f;
    lay.x_axis_height_px = 24.f;
    lay.right_padding_px = 6.f;
    lay.top_padding_frac = 0.05f;
    lay.bottom_padding_frac = 0.05f;
    return lay;
}

// A capture of `close` measured against `band`, the way capture_morph builds it.
CandleSnapshot capture(const PriceBounds& band, double close, bool bull) {
    CandleSnapshot s{};
    s.x = 0.9f;
    s.close = static_cast<float>(vroom::price_fraction(band, close));
    s.bull = bull;
    return s;
}

}  // namespace

TEST_CASE("with no capture the indicator sits on the settled close") {
    const auto lay = make_layout();
    const PriceBounds band{100.0, 200.0};

    const Level l = level_at(lay, band, 150.0, true, nullptr, band, 0.4f);
    CHECK(l.price == doctest::Approx(150.0));
    CHECK(l.y == doctest::Approx(vroom::price_to_y(lay, band, 150.0)));
    // Nothing to blend from, so the new direction's color at full strength.
    CHECK(l.bull_t == doctest::Approx(1.f));
}

TEST_CASE("a morph starts on the outgoing close and lands on the new one") {
    const auto lay = make_layout();
    const PriceBounds band{100.0, 200.0};
    const CandleSnapshot from = capture(band, 140.0, true);

    SUBCASE("t = 0 is the frame the capture replaced") {
        const Level l = level_at(lay, band, 160.0, true, &from, band, 0.f);
        CHECK(l.price == doctest::Approx(140.0));
        CHECK(l.y == doctest::Approx(vroom::price_to_y(lay, band, 140.0)));
    }

    SUBCASE("t = 1 is the settled frame") {
        const Level l = level_at(lay, band, 160.0, true, &from, band, 1.f);
        CHECK(l.price == doctest::Approx(160.0));
        CHECK(l.y == doctest::Approx(vroom::price_to_y(lay, band, 160.0)));
    }

    SUBCASE("halfway is halfway, and the badge agrees with its own position") {
        const Level l = level_at(lay, band, 160.0, true, &from, band, 0.5f);
        CHECK(l.price == doctest::Approx(150.0));
        // A still band is the case the two interpolations collapse into one, so
        // the number the badge shows is exactly the price its pixel stands for.
        CHECK(vroom::y_to_price(lay, band, l.y) == doctest::Approx(l.price));
    }

    SUBCASE("t outside [0,1] is clamped rather than extrapolated") {
        CHECK(level_at(lay, band, 160.0, true, &from, band, -1.f).price ==
              doctest::Approx(140.0));
        CHECK(level_at(lay, band, 160.0, true, &from, band, 2.f).price ==
              doctest::Approx(160.0));
    }
}

// The case the two interpolation spaces disagree on: a tick that sets a new
// high makes auto_price_bounds rescale, so the capture's band and the frame's
// band differ. Both ends still have to be exact — a jump at t=0 would undo the
// point of animating, and a wrong price at t=1 would be a lie.
TEST_CASE("a rescaled band keeps both ends of the animation honest") {
    const auto lay = make_layout();
    const PriceBounds captured{100.0, 200.0};
    const PriceBounds widened{100.0, 260.0};  // the tick pushed the top out
    const CandleSnapshot from = capture(captured, 190.0, true);

    SUBCASE("the first frame lands on the pixel the close already occupied") {
        const Level l = level_at(lay, widened, 250.0, true, &from, captured, 0.f);
        CHECK(l.y == doctest::Approx(vroom::price_to_y(lay, captured, 190.0)));
        CHECK(l.price == doctest::Approx(190.0));
    }

    SUBCASE("the last frame is the new close in the new band") {
        const Level l = level_at(lay, widened, 250.0, true, &from, captured, 1.f);
        CHECK(l.y == doctest::Approx(vroom::price_to_y(lay, widened, 250.0)));
        CHECK(l.price == doctest::Approx(250.0));
    }
}

// The badge has to track the line chart's tip, which draws at close_vertex's
// slot 0. Pinning the expression here is what stops the two drifting apart.
TEST_CASE("the level's y matches the vertex the line chart's tip draws at") {
    const auto lay = make_layout();
    const PriceBounds captured{100.0, 200.0};
    const PriceBounds band{90.0, 210.0};
    const CandleSnapshot from = capture(captured, 140.0, false);
    constexpr double kClose = 175.0;

    for (const float t : {0.f, 0.25f, 0.5f, 0.75f, 1.f}) {
        // close_vertex, ma_overlay.cpp: lerp(y_at_fraction(frm->close),
        //                                    price_to_y(bounds, to->close), t)
        const float a = vroom::y_at_fraction(lay, from.close);
        const float b = vroom::price_to_y(lay, band, kClose);
        const float tip_y = a + (b - a) * t;

        CHECK(level_at(lay, band, kClose, true, &from, captured, t).y ==
              doctest::Approx(tip_y));
    }
}

TEST_CASE("the accent cross-fades only when the candle changes direction") {
    const auto lay = make_layout();
    const PriceBounds band{100.0, 200.0};

    SUBCASE("same direction holds the settled color throughout") {
        const CandleSnapshot from = capture(band, 140.0, true);
        CHECK(level_at(lay, band, 160.0, true, &from, band, 0.f).bull_t ==
              doctest::Approx(1.f));
        CHECK(level_at(lay, band, 160.0, true, &from, band, 0.5f).bull_t ==
              doctest::Approx(1.f));
    }

    SUBCASE("a flip walks from the old accent to the new one") {
        const CandleSnapshot from = capture(band, 140.0, false);
        CHECK(level_at(lay, band, 160.0, true, &from, band, 0.f).bull_t ==
              doctest::Approx(0.f));
        CHECK(level_at(lay, band, 160.0, true, &from, band, 0.5f).bull_t ==
              doctest::Approx(0.5f));
        CHECK(level_at(lay, band, 160.0, true, &from, band, 1.f).bull_t ==
              doctest::Approx(1.f));
    }
}
