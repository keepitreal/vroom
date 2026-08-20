#include "doctest.h"

#include "curve.h"

#include <algorithm>
#include <cmath>
#include <vector>

using vroom::curve::Controls;
using vroom::curve::monotone_tangent;
using vroom::curve::secant;
using vroom::curve::segment_controls;

namespace {

struct Pt {
    float x, y;
};

// Mirrors the walk in ma_overlay.cpp: interior tangents are monotone-limited,
// endpoints fall back to the one secant they have.
std::vector<float> tangents(const std::vector<Pt>& p) {
    const std::size_t n = p.size();
    std::vector<float> m(n, 0.f);
    if (n < 2) return m;
    std::vector<float> d(n - 1);
    for (std::size_t i = 0; i + 1 < n; ++i) {
        d[i] = secant(p[i].x, p[i].y, p[i + 1].x, p[i + 1].y);
    }
    m[0] = d[0];
    m[n - 1] = d[n - 2];
    for (std::size_t i = 1; i + 1 < n; ++i) {
        m[i] = monotone_tangent(d[i - 1], d[i]);
    }
    return m;
}

// Evenly spaced points from a list of closes, the shape the line chart feeds in.
std::vector<Pt> series(std::initializer_list<float> ys, float step = 10.f) {
    std::vector<Pt> p;
    float x = 0.f;
    for (float y : ys) {
        p.push_back(Pt{x, y});
        x += step;
    }
    return p;
}

}  // namespace

TEST_CASE("secant") {
    CHECK(secant(0.f, 0.f, 10.f, 20.f) == doctest::Approx(2.f));
    CHECK(secant(0.f, 10.f, 10.f, 0.f) == doctest::Approx(-1.f));
    CHECK(secant(5.f, 3.f, 15.f, 3.f) == doctest::Approx(0.f));

    SUBCASE("non-increasing x yields 0 instead of an infinity") {
        CHECK(secant(5.f, 0.f, 5.f, 10.f) == doctest::Approx(0.f));
        CHECK(secant(10.f, 0.f, 0.f, 10.f) == doctest::Approx(0.f));
    }
}

TEST_CASE("monotone_tangent") {
    SUBCASE("a local extreme gets a flat tangent") {
        CHECK(monotone_tangent(2.f, -2.f) == doctest::Approx(0.f));
        CHECK(monotone_tangent(-3.f, 5.f) == doctest::Approx(0.f));
    }

    SUBCASE("a flat neighbor also pins the point") {
        CHECK(monotone_tangent(0.f, 3.f) == doctest::Approx(0.f));
        CHECK(monotone_tangent(3.f, 0.f) == doctest::Approx(0.f));
    }

    SUBCASE("same-sign neighbors average when the average is safe") {
        CHECK(monotone_tangent(2.f, 4.f) == doctest::Approx(3.f));
        CHECK(monotone_tangent(-2.f, -4.f) == doctest::Approx(-3.f));
    }

    SUBCASE("the limiter caps the tangent at 3x the smaller secant") {
        // Average would be 50.5; three times the smaller neighbor is 3.
        CHECK(monotone_tangent(1.f, 100.f) == doctest::Approx(3.f));
        CHECK(monotone_tangent(-1.f, -100.f) == doctest::Approx(-3.f));
    }
}

TEST_CASE("segment_controls") {
    SUBCASE("tension 0 lands the controls on the straight segment's thirds") {
        // Wild tangents, so this only passes if tension 0 truly ignores them —
        // that continuity is what lets callers animate the knob.
        const Controls c = segment_controls(0.f, 0.f, 30.f, 30.f,
                                            100.f, -100.f, 0.f);
        CHECK(c.c1x == doctest::Approx(10.f));
        CHECK(c.c1y == doctest::Approx(10.f));
        CHECK(c.c2x == doctest::Approx(20.f));
        CHECK(c.c2y == doctest::Approx(20.f));
    }

    SUBCASE("tension 1 leans fully into the supplied tangents") {
        const Controls c = segment_controls(0.f, 0.f, 30.f, 30.f, 0.f, 0.f, 1.f);
        CHECK(c.c1y == doctest::Approx(0.f));
        CHECK(c.c2y == doctest::Approx(30.f));
    }

    SUBCASE("control x sits at the thirds whatever the tension") {
        for (float t : {0.f, 0.35f, 1.f}) {
            const Controls c =
                segment_controls(0.f, 0.f, 30.f, 5.f, 1.f, -1.f, t);
            CHECK(c.c1x == doctest::Approx(10.f));
            CHECK(c.c2x == doctest::Approx(20.f));
        }
    }

    SUBCASE("half tension sits halfway between straight and smooth") {
        const Controls straight =
            segment_controls(0.f, 0.f, 30.f, 30.f, 0.f, 0.f, 0.f);
        const Controls smooth =
            segment_controls(0.f, 0.f, 30.f, 30.f, 0.f, 0.f, 1.f);
        const Controls half =
            segment_controls(0.f, 0.f, 30.f, 30.f, 0.f, 0.f, 0.5f);
        CHECK(half.c1y == doctest::Approx((straight.c1y + smooth.c1y) * 0.5f));
        CHECK(half.c2y == doctest::Approx((straight.c2y + smooth.c2y) * 0.5f));
    }
}

TEST_CASE("monotone tangents keep the curve from overshooting") {
    // The guarantee that matters on a price chart: a control point outside the
    // segment's own y range means the rendered curve leaves it too, drawing a
    // price that never traded.
    const std::vector<std::vector<Pt>> shapes = {
        series({0.f, 1.f, 20.f}),               // gentle then steep
        series({0.f, 10.f, 0.f, 10.f, 0.f}),    // alternating spikes
        series({5.f, 5.f, 5.f, 100.f}),         // flat run into a jump
        series({100.f, 4.f, 3.9f, 3.8f, 0.f}),  // crash then a slow bleed
        series({0.f, 50.f, 49.f, 51.f, 2.f}),   // noisy top
    };

    for (const std::vector<Pt>& p : shapes) {
        const std::vector<float> m = tangents(p);
        for (std::size_t i = 0; i + 1 < p.size(); ++i) {
            const Controls c = segment_controls(p[i].x, p[i].y,
                                                p[i + 1].x, p[i + 1].y,
                                                m[i], m[i + 1], 1.f);
            const float lo = std::min(p[i].y, p[i + 1].y);
            const float hi = std::max(p[i].y, p[i + 1].y);
            // A hair of tolerance: the limiter's bound is exact, so controls are
            // allowed to touch the endpoints.
            CHECK(c.c1y >= lo - 0.001f);
            CHECK(c.c1y <= hi + 0.001f);
            CHECK(c.c2y >= lo - 0.001f);
            CHECK(c.c2y <= hi + 0.001f);
        }
    }
}

TEST_CASE("an unlimited tangent would overshoot") {
    // Guards the limiter itself: on this shape the plain average sends a control
    // point below the segment, which is exactly what monotone_tangent prevents.
    const std::vector<Pt> p = series({0.f, 1.f, 20.f});
    const float d0 = secant(p[0].x, p[0].y, p[1].x, p[1].y);
    const float d1 = secant(p[1].x, p[1].y, p[2].x, p[2].y);

    const float naive = (d0 + d1) * 0.5f;
    const Controls bad =
        segment_controls(p[0].x, p[0].y, p[1].x, p[1].y, d0, naive, 1.f);
    CHECK(bad.c2y < 0.f);

    const Controls good = segment_controls(p[0].x, p[0].y, p[1].x, p[1].y, d0,
                                           monotone_tangent(d0, d1), 1.f);
    CHECK(good.c2y >= -0.001f);
}
