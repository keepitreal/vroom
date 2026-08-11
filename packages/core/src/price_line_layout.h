// Price-status-line geometry — pure functions, no Skia, no state.
//
// Split out from price_lines.cpp so the rendering pass and the hit-test pass
// derive their rectangles from the same arithmetic: a label the user can see but
// not click (or vice versa) is the classic failure mode for this widget. Being
// Skia-free it also unit-tests without a Skia checkout.

#pragma once

#include "vroom/vroom_chart.h"
namespace vroom::price_lines {

// Pill padding / corner radius, matching the current-price indicator so a price
// line and the price badge read as the same family of chrome.
constexpr float kPadH = 8.f;    // left/right of the text
constexpr float kPadV = 4.f;    // above/below the text
constexpr float kCorner = 6.f;  // rounded-pill corner radius

// Extra grab band above and below the stroke, so a 1px line is still an easy
// target. Matches the tolerance mainstream chart libraries settled on.
constexpr float kLineHitTolerance = 7.f;

// Gap between the label group and the price axis when right-aligned. Wide enough
// that the line still shows through it — a couple of dashes reading as "this
// continues to the badge" rather than one orphaned dot.
constexpr float kAxisGutter = 12.f;

struct Rect {
    float left = 0.f;
    float top = 0.f;
    float right = 0.f;
    float bottom = 0.f;

    // Empty rects stand in for absent segments (no quantity, no close button).
    bool empty() const { return right <= left || bottom <= top; }
    float width() const { return right - left; }
};

// True when (x, y) falls inside `r`. Always false for an empty rect.
bool contains(const Rect& r, float x, float y);

// The text extents a caller measured with the real font, plus which optional
// segments to reserve room for. `label_h` is the pill height (already including
// kPadV on both sides).
struct LabelMetrics {
    float text_w = 0.f;      // body text width; 0 hides the body pill
    float quantity_w = 0.f;  // quantity text width; 0 hides the quantity pill
    float label_h = 0.f;
    bool  closable = false;  // reserve the trailing square close-button cell
};

// The laid-out label group, vertically centered on the line. Any of the three
// segment rects may be empty. `left`/`right` bound whichever segments exist and
// are equal when the group is empty (a bare line).
struct GroupLayout {
    Rect  body;
    Rect  quantity;
    Rect  close;
    float left = 0.f;
    float right = 0.f;

    bool empty() const { return right <= left; }
};

// Lays the group out on the line at `y`. `pane_right` is the inner edge of the
// price axis (the line's right end).
//
// `style.align` picks the anchor: 2 (right, the default) parks the group's right
// edge `style.line_length_frac` of the pane width in from the axis; 1 centers
// it; 0 pins it to the left edge. The group is clamped into [0, pane_right], so
// a label wider than the pane starts flush left rather than overflowing.
GroupLayout layout_group(const LabelMetrics& metrics,
                         float y,
                         float pane_right,
                         const ::VroomPriceLineStyle& style);

// True when `y` is within the grab band of a line drawn at `line_y`.
bool hits_line(float line_y, float stroke_width, float y);

}  // namespace vroom::price_lines
