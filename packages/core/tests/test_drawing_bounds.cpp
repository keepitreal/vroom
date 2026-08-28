#include "doctest.h"

#include "drawing_bounds.h"

using vroom::drawing_bounds::Accumulator;
using vroom::drawing_bounds::RectPx;

TEST_CASE("two points span their extent") {
    Accumulator acc;
    acc.add(10.f, 20.f);
    acc.add(40.f, 60.f);
    const RectPx r = acc.to_rect(0.f);
    CHECK(r.x == doctest::Approx(10.f));
    CHECK(r.y == doctest::Approx(20.f));
    CHECK(r.width == doctest::Approx(30.f));
    CHECK(r.height == doctest::Approx(40.f));
}

TEST_CASE("order doesn't matter — a box drawn up-left spans the same rect") {
    // A box's stored corners are whichever two the user dragged between, so the
    // accumulator has to normalize rather than assume a is the top-left.
    Accumulator forward;
    forward.add(10.f, 20.f);
    forward.add(40.f, 60.f);
    Accumulator backward;
    backward.add(40.f, 60.f);
    backward.add(10.f, 20.f);
    const RectPx a = forward.to_rect(0.f);
    const RectPx b = backward.to_rect(0.f);
    CHECK(a.x == doctest::Approx(b.x));
    CHECK(a.y == doctest::Approx(b.y));
    CHECK(a.width == doctest::Approx(b.width));
    CHECK(a.height == doctest::Approx(b.height));
}

TEST_CASE("a multi-point stroke covers where it wandered, not just its ends") {
    // The regression this guards: a pencil stroke's a/b mirror its first and
    // last sample, so bounding only those would clip the arc between them.
    Accumulator acc;
    acc.add(0.f, 50.f);    // first
    acc.add(25.f, 5.f);    // the peak, well above both ends
    acc.add(50.f, 50.f);   // last
    const RectPx r = acc.to_rect(0.f);
    CHECK(r.y == doctest::Approx(5.f));
    CHECK(r.height == doctest::Approx(45.f));
}

TEST_CASE("stroke width grows the rect by half on every side") {
    Accumulator acc;
    acc.add(10.f, 10.f);
    acc.add(20.f, 30.f);
    const RectPx r = acc.to_rect(4.f);
    CHECK(r.x == doctest::Approx(8.f));
    CHECK(r.y == doctest::Approx(8.f));
    CHECK(r.width == doctest::Approx(14.f));   // 10 + 2 + 2
    CHECK(r.height == doctest::Approx(24.f));  // 20 + 2 + 2
}

TEST_CASE("a single point yields the stroke's own footprint") {
    // What a one-sample pencil press paints: a dot of the stroke's diameter.
    Accumulator acc;
    acc.add(100.f, 200.f);
    const RectPx r = acc.to_rect(6.f);
    CHECK(r.x == doctest::Approx(97.f));
    CHECK(r.y == doctest::Approx(197.f));
    CHECK(r.width == doctest::Approx(6.f));
    CHECK(r.height == doctest::Approx(6.f));
}

TEST_CASE("nothing added reports empty and a zero rect") {
    Accumulator acc;
    CHECK(acc.has == false);
    const RectPx r = acc.to_rect(2.f);
    CHECK(r.width == doctest::Approx(0.f));
    CHECK(r.height == doctest::Approx(0.f));
}

TEST_CASE("a non-finite coordinate is dropped rather than propagated") {
    // One NaN would otherwise win every min/max comparison and hand the host a
    // rectangle it can't position anything against.
    Accumulator acc;
    acc.add(10.f, 10.f);
    acc.add(std::nan(""), 50.f);
    acc.add(30.f, 20.f);
    const RectPx r = acc.to_rect(0.f);
    CHECK(acc.has == true);
    CHECK(r.x == doctest::Approx(10.f));
    CHECK(r.width == doctest::Approx(20.f));
    CHECK(r.height == doctest::Approx(10.f));
}

TEST_CASE("a negative stroke width doesn't shrink the rect") {
    Accumulator acc;
    acc.add(0.f, 0.f);
    acc.add(10.f, 10.f);
    const RectPx r = acc.to_rect(-4.f);
    CHECK(r.x == doctest::Approx(0.f));
    CHECK(r.width == doctest::Approx(10.f));
}
