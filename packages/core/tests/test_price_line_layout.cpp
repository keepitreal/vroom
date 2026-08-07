#include "doctest.h"

#include "price_line_layout.h"

using vroom::price_lines::GroupLayout;
using vroom::price_lines::LabelMetrics;
using vroom::price_lines::Rect;
using vroom::price_lines::kAxisGutter;
using vroom::price_lines::kPadH;

namespace {

// Right-aligned flush against the axis — the default the wrappers pass.
VroomPriceLineStyle make_style() {
    VroomPriceLineStyle s{};
    s.body_bg = 0x40ffffff;
    s.font_size_px = 0.f;
    s.line_length_frac = 0.f;
    s.align = 2;
    s.hover_boost = 1.2f;
    return s;
}

// 100px of body text, 40px of quantity text, 20px tall pills.
LabelMetrics make_metrics() {
    LabelMetrics m{};
    m.text_w = 100.f;
    m.quantity_w = 40.f;
    m.label_h = 20.f;
    m.closable = false;
    return m;
}

}  // namespace

TEST_CASE("contains") {
    const Rect r{10.f, 20.f, 50.f, 40.f};
    CHECK(vroom::price_lines::contains(r, 30.f, 30.f));
    CHECK(vroom::price_lines::contains(r, 10.f, 20.f));  // inclusive edges
    CHECK(vroom::price_lines::contains(r, 50.f, 40.f));
    CHECK_FALSE(vroom::price_lines::contains(r, 9.f, 30.f));
    CHECK_FALSE(vroom::price_lines::contains(r, 30.f, 41.f));

    SUBCASE("empty rects never contain anything") {
        CHECK(Rect{}.empty());
        CHECK_FALSE(vroom::price_lines::contains(Rect{}, 0.f, 0.f));
        // Zero-width but non-zero-height still counts as empty.
        CHECK_FALSE(vroom::price_lines::contains(Rect{10.f, 20.f, 10.f, 40.f},
                                                 10.f, 30.f));
    }
}

TEST_CASE("hits_line") {
    // Band = max(stroke, 1) + 7.
    CHECK(vroom::price_lines::hits_line(100.f, 1.f, 100.f));
    CHECK(vroom::price_lines::hits_line(100.f, 1.f, 108.f));
    CHECK(vroom::price_lines::hits_line(100.f, 1.f, 92.f));
    CHECK_FALSE(vroom::price_lines::hits_line(100.f, 1.f, 108.5f));
    CHECK_FALSE(vroom::price_lines::hits_line(100.f, 1.f, 91.5f));

    SUBCASE("a thicker stroke widens the band") {
        CHECK(vroom::price_lines::hits_line(100.f, 4.f, 111.f));
        CHECK_FALSE(vroom::price_lines::hits_line(100.f, 4.f, 111.5f));
    }

    SUBCASE("sub-pixel strokes still get the full tolerance") {
        CHECK(vroom::price_lines::hits_line(100.f, 0.f, 108.f));
    }
}

TEST_CASE("layout_group segment order and widths") {
    LabelMetrics m = make_metrics();
    m.closable = true;
    const GroupLayout g =
        vroom::price_lines::layout_group(m, 200.f, 1000.f, make_style());

    // body = 100 + 2*8 = 116, quantity = 40 + 2*8 = 56, close = label_h = 20.
    CHECK(g.body.width() == doctest::Approx(116.f));
    CHECK(g.quantity.width() == doctest::Approx(56.f));
    CHECK(g.close.width() == doctest::Approx(20.f));

    SUBCASE("segments butt together left to right") {
        CHECK(g.body.right == doctest::Approx(g.quantity.left));
        CHECK(g.quantity.right == doctest::Approx(g.close.left));
        CHECK(g.left == doctest::Approx(g.body.left));
        CHECK(g.right == doctest::Approx(g.close.right));
    }

    SUBCASE("every segment is centered on the line's y") {
        CHECK(g.body.top == doctest::Approx(190.f));
        CHECK(g.body.bottom == doctest::Approx(210.f));
        CHECK(g.quantity.top == doctest::Approx(g.body.top));
        CHECK(g.close.bottom == doctest::Approx(g.body.bottom));
    }
}

TEST_CASE("layout_group omits absent segments") {
    VroomPriceLineStyle s = make_style();

    SUBCASE("no quantity, no close button") {
        LabelMetrics m = make_metrics();
        m.quantity_w = 0.f;
        const GroupLayout g = vroom::price_lines::layout_group(m, 200.f, 1000.f, s);
        CHECK_FALSE(g.body.empty());
        CHECK(g.quantity.empty());
        CHECK(g.close.empty());
        // The group is exactly the body pill.
        CHECK(g.right - g.left == doctest::Approx(116.f));
    }

    SUBCASE("close button with no text at all") {
        LabelMetrics m{};
        m.label_h = 20.f;
        m.closable = true;
        const GroupLayout g = vroom::price_lines::layout_group(m, 200.f, 1000.f, s);
        CHECK(g.body.empty());
        CHECK(g.quantity.empty());
        CHECK(g.close.width() == doctest::Approx(20.f));
        CHECK(g.left == doctest::Approx(g.close.left));
    }

    SUBCASE("a bare line reports an empty group at the anchor") {
        LabelMetrics m{};
        m.label_h = 20.f;
        const GroupLayout g = vroom::price_lines::layout_group(m, 200.f, 1000.f, s);
        CHECK(g.empty());
        CHECK(g.left == doctest::Approx(1000.f));
        CHECK(g.right == doctest::Approx(1000.f));
    }

    SUBCASE("a zero-height label degenerates to a bare line") {
        LabelMetrics m = make_metrics();
        m.label_h = 0.f;
        CHECK(vroom::price_lines::layout_group(m, 200.f, 1000.f, s).empty());
    }
}

TEST_CASE("layout_group alignment") {
    LabelMetrics m = make_metrics();  // group = 116 + 56 = 172 wide
    VroomPriceLineStyle s = make_style();

    SUBCASE("right-aligned sits a gutter in from the axis") {
        s.align = 2;
        const GroupLayout g = vroom::price_lines::layout_group(m, 200.f, 1000.f, s);
        CHECK(g.right == doctest::Approx(1000.f - kAxisGutter));
        CHECK(g.left == doctest::Approx(1000.f - kAxisGutter - 172.f));
    }

    SUBCASE("line_length_frac pushes the group left by a fraction of the pane") {
        s.align = 2;
        s.line_length_frac = 0.5f;
        const GroupLayout g = vroom::price_lines::layout_group(m, 200.f, 1000.f, s);
        CHECK(g.right == doctest::Approx(500.f - kAxisGutter));
    }

    SUBCASE("centered") {
        s.align = 1;
        const GroupLayout g = vroom::price_lines::layout_group(m, 200.f, 1000.f, s);
        CHECK(g.left == doctest::Approx((1000.f - 172.f) * 0.5f));
    }

    SUBCASE("left-aligned pins to the pane's left edge") {
        s.align = 0;
        const GroupLayout g = vroom::price_lines::layout_group(m, 200.f, 1000.f, s);
        CHECK(g.left == doctest::Approx(0.f));
        CHECK(g.right == doctest::Approx(172.f));
    }

    SUBCASE("line_length_frac is clamped to 0..1") {
        s.align = 2;
        s.line_length_frac = 4.f;
        // Fully inset would put the group off the left edge; it clamps to 0.
        CHECK(vroom::price_lines::layout_group(m, 200.f, 1000.f, s).left ==
              doctest::Approx(0.f));

        s.line_length_frac = -1.f;
        CHECK(vroom::price_lines::layout_group(m, 200.f, 1000.f, s).right ==
              doctest::Approx(1000.f - kAxisGutter));
    }
}

TEST_CASE("layout_group clamps into the pane") {
    VroomPriceLineStyle s = make_style();

    SUBCASE("never overlaps the price axis") {
        LabelMetrics m = make_metrics();
        s.align = 0;  // left-aligned, but the pane is narrower than the group
        const GroupLayout g = vroom::price_lines::layout_group(m, 200.f, 100.f, s);
        CHECK(g.left == doctest::Approx(0.f));
    }

    SUBCASE("a label wider than the pane starts flush left") {
        LabelMetrics m = make_metrics();
        m.text_w = 2000.f;
        const GroupLayout g = vroom::price_lines::layout_group(m, 200.f, 300.f, s);
        CHECK(g.left == doctest::Approx(0.f));
    }

    SUBCASE("a degenerate pane yields no group") {
        LabelMetrics m = make_metrics();
        CHECK(vroom::price_lines::layout_group(m, 200.f, 0.f, s).empty());
        CHECK(vroom::price_lines::layout_group(m, 200.f, -10.f, s).empty());
    }
}

TEST_CASE("layout_group padding is symmetric around the text") {
    LabelMetrics m{};
    m.text_w = 50.f;
    m.label_h = 20.f;
    const GroupLayout g =
        vroom::price_lines::layout_group(m, 100.f, 1000.f, make_style());
    // Text is inset by kPadH on each side of the pill.
    CHECK(g.body.width() - m.text_w == doctest::Approx(2.f * kPadH));
}
