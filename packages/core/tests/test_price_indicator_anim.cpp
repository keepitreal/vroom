#include "doctest.h"

#include "price_indicator.h"
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

TEST_CASE("flat dash caps use the dash and gap as is") {
    const auto iv = vroom::price_indicator_dash::intervals(2.f, 2.f, 1.f, false);
    CHECK(iv.on == doctest::Approx(2.f));
    CHECK(iv.off == doctest::Approx(2.f));
}

TEST_CASE("round dash caps move the cap width from the dash into the gap") {
    using vroom::price_indicator_dash::intervals;

    const auto thin = intervals(4.f, 8.f, 1.f, true);
    CHECK(thin.on == doctest::Approx(3.f));
    CHECK(thin.off == doctest::Approx(9.f));

    const auto thick = intervals(4.f, 8.f, 2.f, true);
    CHECK(thick.on == doctest::Approx(2.f));
    CHECK(thick.off == doctest::Approx(10.f));
}

TEST_CASE("a round-capped dash shorter than the width is a dot on the same period") {
    const auto iv = vroom::price_indicator_dash::intervals(1.f, 4.f, 3.f, true);
    CHECK(iv.on == doctest::Approx(0.f));
    CHECK(iv.off == doctest::Approx(5.f));
}

TEST_CASE("a zero dash gap draws solid") {
    using vroom::price_indicator_dash::intervals;
    CHECK(intervals(4.f, 0.f, 1.f, false).off == doctest::Approx(0.f));
    CHECK(intervals(4.f, 0.f, 1.f, true).off == doctest::Approx(0.f));
}

TEST_CASE("the dash pattern hangs off the line's right end") {
    using vroom::price_indicator_dash::intervals;
    using vroom::price_indicator_dash::stroke_start;
    constexpr float kRight = 300.f;

    SUBCASE("flat caps start the first dash at the edge") {
        CHECK(stroke_start(kRight, 1.f, false) == doctest::Approx(kRight));
    }

    SUBCASE("a round cap ends flush with the edge, a whole dash long") {
        const float width = 2.f;
        const float start = stroke_start(kRight, width, true);
        const auto iv = intervals(4.f, 8.f, width, true);
        // Drawn leftward from `start`, the first dash covers [start - on, start]
        // and its caps reach half the width past either end.
        CHECK(start + width * 0.5f == doctest::Approx(kRight));
        CHECK(start - iv.on - width * 0.5f == doctest::Approx(kRight - 4.f));
    }
}

namespace {

using vroom::price_indicator_style::Morph;
using vroom::price_indicator_style::Style;

vroom::Theme price_line_theme(float dash, float gap, float width,
                              float opacity, bool round_cap) {
    vroom::Theme t = vroom::default_theme();
    t.floats[VROOM_FLOAT_PRICE_INDICATOR_DASH_PX] = dash;
    t.floats[VROOM_FLOAT_PRICE_INDICATOR_GAP_PX] = gap;
    t.floats[VROOM_FLOAT_PRICE_INDICATOR_WIDTH_PX] = width;
    t.floats[VROOM_FLOAT_PRICE_INDICATOR_OPACITY] = opacity;
    t.floats[VROOM_FLOAT_PRICE_INDICATOR_ROUND_CAP] = round_cap ? 1.f : 0.f;
    return t;
}

}  // namespace

TEST_CASE("a settled price-line blend draws the theme as is") {
    const vroom::Theme theme = price_line_theme(4.f, 6.f, 1.f, 0.3f, false);
    const Style s = vroom::price_indicator_style::current(Morph{}, theme, 0.7f);
    CHECK(s.gap == doctest::Approx(6.f));
    CHECK_FALSE(s.round_cap);
}

TEST_CASE("every number in the price-line style blends") {
    using vroom::price_indicator_style::begin;
    using vroom::price_indicator_style::current;

    const vroom::Theme candles = vroom::default_theme();  // 2 / 2, 1px, opaque
    const vroom::Theme line = price_line_theme(4.f, 8.f, 3.f, 0.3f, true);
    const Morph m = begin(Morph{}, candles, 0, 0.f, 1.f);
    const Style s = current(m, line, 0.25f);
    CHECK(s.dash == doctest::Approx(2.5f));
    CHECK(s.gap == doctest::Approx(3.5f));
    CHECK(s.width == doctest::Approx(1.5f));
    CHECK(s.opacity == doctest::Approx(0.825f));
    CHECK_FALSE(s.round_cap);
    CHECK(current(m, line, 0.5f).round_cap);
}

TEST_CASE("the price-line blend rides the morph fade either way") {
    using vroom::price_indicator_style::begin;
    using vroom::price_indicator_style::current;
    using vroom::price_indicator_style::progress;

    const vroom::Theme candles = vroom::default_theme();
    const vroom::Theme line = price_line_theme(4.f, 8.f, 1.f, 0.3f, true);

    // Candles → line: captured at fade 0, then the line theme lands.
    const Morph to_line = begin(Morph{}, candles, 0, 0.f, 1.f);
    CHECK(current(to_line, line, 0.f).gap == doctest::Approx(2.f));
    CHECK(current(to_line, line, 1.f).gap == doctest::Approx(8.f));

    // Reversed at 0.6: the way back starts where the line stood.
    const Style mid = current(to_line, line, 0.6f);
    const Morph back = begin(to_line, line, 0, 0.6f, 0.f);
    CHECK(progress(back, 0.6f) == doctest::Approx(0.f));
    CHECK(current(back, candles, 0.6f).gap == doctest::Approx(mid.gap));
    CHECK(current(back, candles, 0.3f).gap ==
          doctest::Approx(mid.gap + (2.f - mid.gap) * 0.5f));
    CHECK(current(back, candles, 0.f).gap == doctest::Approx(2.f));

    // Ending it (set_chart_type) draws the theme whatever the fade says.
    Morph settled = back;
    settled.from_fade = settled.to_fade;
    CHECK(current(settled, candles, 0.6f).gap == doctest::Approx(2.f));
}

TEST_CASE("the price line and badge color blends on the same fade") {
    using vroom::lerp_argb;
    using vroom::price_indicator_style::begin;
    using vroom::price_indicator_style::color;
    constexpr uint32_t kRed = 0xFFFF0000u;    // the line theme's period color
    constexpr uint32_t kGreen = 0xFF00FF00u;  // the candle theme's bull accent

    const vroom::Theme line = vroom::default_theme();
    CHECK(color(Morph{}, kGreen, 0.3f) == kGreen);

    // Line → candles, captured red at fade 1; the candles resolve green.
    const Morph to_candles = begin(Morph{}, line, kRed, 1.f, 0.f);
    CHECK(color(to_candles, kGreen, 1.f) == kRed);
    CHECK(color(to_candles, kGreen, 0.5f) == 0xFF808000u);
    CHECK(color(to_candles, kGreen, 0.f) == kGreen);

    // Reversed at 0.75: back to red from the blend on screen, not from green.
    const uint32_t mid = color(to_candles, kGreen, 0.75f);
    CHECK(mid == lerp_argb(kRed, kGreen, 0.25f));
    const Morph back = begin(to_candles, line, kGreen, 0.75f, 1.f);
    CHECK(color(back, kRed, 0.75f) == mid);
    CHECK(color(back, kRed, 0.875f) == lerp_argb(mid, kRed, 0.5f));
    CHECK(color(back, kRed, 1.f) == kRed);

    // Ending it (set_chart_type) draws the accent whatever the fade says.
    Morph settled = back;
    settled.from_fade = settled.to_fade;
    CHECK(color(settled, kRed, 0.8f) == kRed);
}

TEST_CASE("line to candles only closes the price line's gaps") {
    using vroom::price_indicator_dash::intervals;
    using vroom::price_indicator_dash::stroke_start;
    using vroom::price_indicator_style::begin;
    using vroom::price_indicator_style::current;
    constexpr float kRight = 300.f;

    const vroom::Theme line = price_line_theme(4.f, 8.f, 1.f, 0.3f, true);
    const vroom::Theme candles = price_line_theme(4.f, 6.f, 1.f, 0.3f, false);
    const Morph m = begin(Morph{}, line, 0, 1.f, 0.f);
    for (int i = 0; i <= 8; ++i) {
        const float t = static_cast<float>(i) / 8.f;
        const Style s = current(m, candles, 1.f - t);
        const auto iv = intervals(s.dash, s.gap, s.width, s.round_cap);
        const float cap = s.round_cap ? s.width * 0.5f : 0.f;
        const float start = stroke_start(kRight, s.width, s.round_cap);
        CAPTURE(t);
        CHECK(s.round_cap == (t < 0.5f));
        CHECK(s.opacity == doctest::Approx(0.3f));
        // The first dash keeps its right end and its visible length through
        // the cap switch, and the gap closes evenly from 8 to 6.
        CHECK(start + cap == doctest::Approx(kRight));
        CHECK(iv.on + 2.f * cap == doctest::Approx(4.f));
        CHECK(iv.off - 2.f * cap == doctest::Approx(8.f - 2.f * t));
    }
}
