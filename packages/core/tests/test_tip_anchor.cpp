#include "doctest.h"

#include "tip_anchor.h"

using vroom::tip_anchor::Anchor;
using vroom::tip_anchor::at;

TEST_CASE("at the right edge the tip slot is the visible slice's own last slot") {
    // The common case, and the one that has to stay byte-identical to drawing
    // the visible slice: slot_count matches range_end - range_start.
    const Anchor a = at(40, 100, 100, false);
    CHECK(a.slot_count == 60);
    CHECK(a.use_morph == false);
}

TEST_CASE("scrolled back, the slot count reaches past the visible slice") {
    // 20 candles on screen but 100 in the series: slot 0 has to land on index
    // 99, which is 60 slots past the right edge of the view.
    const Anchor a = at(40, 80, 100, false);
    CHECK(a.slot_count == 60);
    // visible[slot_count - 1] is candles[range_start + slot_count - 1].
    CHECK(40 + a.slot_count - 1 == 99);
}

TEST_CASE("the whole series visible anchors on the last candle") {
    const Anchor a = at(0, 100, 100, false);
    CHECK(a.slot_count == 100);
}

TEST_CASE("a single visible candle still yields a usable slot") {
    CHECK(at(99, 100, 100, false).slot_count == 1);
    CHECK(at(0, 1, 1, false).slot_count == 1);
}

TEST_CASE("the morph capture only pairs while the newest candle is on screen") {
    SUBCASE("at the right edge it pairs, so a timeframe switch animates the tip") {
        CHECK(at(40, 100, 100, true).use_morph == true);
    }

    SUBCASE("scrolled back it does not, so the dot can't strand mid-pane") {
        // The capture's slot 0 is the newest *visible* candle from before the
        // switch; pairing it with the series' newest would interpolate between
        // two unrelated candles.
        CHECK(at(40, 80, 100, true).use_morph == false);
    }

    SUBCASE("no morph running means nothing to pair either way") {
        CHECK(at(40, 100, 100, false).use_morph == false);
        CHECK(at(40, 80, 100, false).use_morph == false);
    }
}

TEST_CASE("degenerate slices report no slots rather than reaching out of bounds") {
    SUBCASE("empty series") {
        const Anchor a = at(0, 0, 0, false);
        CHECK(a.slot_count == 0);
        CHECK(a.use_morph == false);
    }

    SUBCASE("slice starting at or past the end") {
        CHECK(at(100, 100, 100, true).slot_count == 0);
        CHECK(at(100, 100, 100, true).use_morph == false);
        CHECK(at(120, 120, 100, false).slot_count == 0);
    }
}

TEST_CASE("a range_end past the series still counts as the right edge") {
    // visible_indices clamps to the buffer, so this shouldn't arise; treating it
    // as the right edge keeps the morph behaving rather than silently dropping.
    CHECK(at(40, 120, 100, true).use_morph == true);
    CHECK(at(40, 120, 100, true).slot_count == 60);
}
