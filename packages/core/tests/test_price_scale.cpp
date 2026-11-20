#include "doctest.h"

#include "price_scale.h"

using vroom::PriceBounds;
using vroom::price_scale::Flags;
using vroom::price_scale::Handback;
using vroom::price_scale::autoscale_engaged;
using vroom::price_scale::handback;
using vroom::price_scale::records_user_adjustment;
using vroom::price_scale::translate_moves_price;
using vroom::price_scale::zoom_scales_price;

namespace {
// What a price-axis drag does to the flags: it always takes the bounds manual,
// and records the adjustment only where that is tracked.
Flags after_axis_drag(Flags f) {
    if (records_user_adjustment(f)) f.user_adjusted = true;
    f.manual = true;
    return f;
}

Flags after_reset_price_scale(Flags f) {
    f.manual = false;
    return f;
}
}  // namespace

TEST_CASE("price_scale: autoscale engaged keeps plot gestures off the price range") {
    const Flags engaged{true, false, false};
    REQUIRE(autoscale_engaged(engaged));
    CHECK_FALSE(translate_moves_price(engaged));
    CHECK_FALSE(zoom_scales_price(engaged));
    // A time zoom or time-axis drag there doesn't disarm autoscale either.
    CHECK_FALSE(records_user_adjustment(engaged));
}

TEST_CASE("price_scale: a price-axis drag takes over under autoscale") {
    const Flags taken = after_axis_drag({true, false, false});
    CHECK(taken.manual);
    CHECK_FALSE(taken.user_adjusted);
    CHECK_FALSE(autoscale_engaged(taken));

    SUBCASE("after which the plot gestures move price as in manual mode") {
        CHECK(translate_moves_price(taken));
        CHECK(zoom_scales_price(taken));
    }

    SUBCASE("and a zoom there still isn't recorded, so the reset can re-engage") {
        CHECK_FALSE(records_user_adjustment(taken));
    }

    SUBCASE("reset_price_scale hands it back, easing from the bounds on screen") {
        CHECK(handback(taken, true) == Handback::kEase);
        CHECK(autoscale_engaged(after_reset_price_scale(taken)));
    }

    SUBCASE("reduced motion lands on the fit instead") {
        CHECK(handback(taken, false) == Handback::kSnap);
    }
}

TEST_CASE("price_scale: autoscale off is plain manual mode") {
    const Flags fresh{false, false, false};
    CHECK_FALSE(autoscale_engaged(fresh));
    CHECK(translate_moves_price(fresh));
    CHECK(zoom_scales_price(fresh));
    CHECK(records_user_adjustment(fresh));

    const Flags dragged = after_axis_drag(fresh);
    CHECK(dragged.manual);
    CHECK(dragged.user_adjusted);

    SUBCASE("reset_price_scale returns to the instant fit, animated or not") {
        CHECK(handback(dragged, true) == Handback::kFit);
        CHECK(handback(dragged, false) == Handback::kFit);
        // Only reset_view clears the record.
        CHECK(after_reset_price_scale(dragged).user_adjusted);
    }

    SUBCASE("with nothing manual there is nothing to hand back") {
        CHECK(handback(fresh, true) == Handback::kNone);
        CHECK(handback({true, false, false}, true) == Handback::kNone);
    }
}

TEST_CASE("price_scale: a scale the user set before autoscale came on stays theirs") {
    // Adjusted while autoscale was off, then autoscale turned on: it stays out
    // of the way until reset_view.
    const Flags kept{true, true, true};
    CHECK_FALSE(autoscale_engaged(kept));
    CHECK(translate_moves_price(kept));
    CHECK(zoom_scales_price(kept));
    CHECK(handback(kept, true) == Handback::kFit);
    CHECK_FALSE(autoscale_engaged(after_reset_price_scale(kept)));
}

TEST_CASE("price_scale::ease after a hand-back") {
    // The user zoomed in: the range handed back sits inside the candles.
    const PriceBounds manual{48.0, 52.0};
    const PriceBounds envelope{40.0, 60.0};
    const PriceBounds target{35.0, 65.0};
    const float dt = 1.f / 120.f;

    SUBCASE("eases out from the bounds on screen rather than jumping to the candles") {
        bool handing_back = true;
        const PriceBounds b =
            vroom::price_scale::ease(manual, target, envelope, dt, true, &handing_back);
        CHECK(handing_back);
        CHECK(b.min < manual.min);
        CHECK(b.min > envelope.min);
        CHECK(b.max > manual.max);
        CHECK(b.max < envelope.max);
    }

    SUBCASE("without the hand-back the same frame is held to the candles") {
        bool handing_back = false;
        const PriceBounds b =
            vroom::price_scale::ease(manual, target, envelope, dt, true, &handing_back);
        CHECK(b.min <= envelope.min);
        CHECK(b.max >= envelope.max);
    }

    SUBCASE("lands on the fit, and holds the candles again once it contains them") {
        bool handing_back = true;
        PriceBounds d = manual;
        bool contained_while_handing_back = false;
        for (int i = 0; i < 600 && !(d.min == target.min && d.max == target.max); ++i) {
            d = vroom::price_scale::ease(d, target, envelope, dt, true, &handing_back);
            const bool contains = d.min <= envelope.min && d.max >= envelope.max;
            if (contains && handing_back) contained_while_handing_back = true;
        }
        CHECK(d.min == target.min);
        CHECK(d.max == target.max);
        CHECK_FALSE(handing_back);
        CHECK_FALSE(contained_while_handing_back);
    }

    SUBCASE("reduced motion lands immediately") {
        bool handing_back = true;
        const PriceBounds b =
            vroom::price_scale::ease(manual, target, envelope, dt, false, &handing_back);
        CHECK(b.min == target.min);
        CHECK(b.max == target.max);
        CHECK_FALSE(handing_back);
    }

    SUBCASE("a fit that can't contain the candles still ends the hand-back") {
        // A log scale floors its low above an envelope that reaches zero.
        bool handing_back = true;
        const PriceBounds floored{1.0, 65.0};
        PriceBounds d = manual;
        for (int i = 0; i < 600 && handing_back; ++i) {
            d = vroom::price_scale::ease(d, floored, {0.0, 60.0}, dt, true, &handing_back);
        }
        CHECK_FALSE(handing_back);
    }
}
