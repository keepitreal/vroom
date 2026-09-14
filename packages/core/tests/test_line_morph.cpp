#include "doctest.h"

#include "line_morph.h"

using vroom::find_line_morph;
using vroom::LineKey;
using vroom::LineKind;
using vroom::LineMorph;
using vroom::LineSnapshot;
using vroom::morph_line_count;
using vroom::morph_vertex;
using vroom::MorphVertex;

namespace {
MorphVertex pt(float x, float y) { return MorphVertex{x, y, true}; }
MorphVertex gap() { return MorphVertex{}; }

LineMorph capture(LineKey key, std::size_t n) {
    LineMorph m;
    m.key = key;
    m.pts.assign(n, LineSnapshot{0.f, 0.f, true});
    return m;
}
}  // namespace

TEST_CASE("a slot both sides define interpolates between them") {
    const MorphVertex to = pt(100.f, 20.f);
    const MorphVertex from = pt(0.f, 60.f);

    SUBCASE("t = 0 is the capture, pixel for pixel") {
        const MorphVertex v = morph_vertex(to, from, 0.f);
        CHECK(v.valid);
        CHECK(v.x == doctest::Approx(0.f));
        CHECK(v.y == doctest::Approx(60.f));
    }

    SUBCASE("t = 1 is the new series, so a landed morph draws it untouched") {
        const MorphVertex v = morph_vertex(to, from, 1.f);
        CHECK(v.x == doctest::Approx(100.f));
        CHECK(v.y == doctest::Approx(20.f));
    }

    SUBCASE("midway is the midpoint on both axes") {
        const MorphVertex v = morph_vertex(to, from, 0.5f);
        CHECK(v.x == doctest::Approx(50.f));
        CHECK(v.y == doctest::Approx(40.f));
    }
}

TEST_CASE("a slot only one side defines holds that side rather than lifting the pen") {
    // The warmup gap at a line's left end covers a different span at each
    // resolution, so one side goes valid before the other. Sliding that end
    // reads better than blinking it.
    SUBCASE("only the new series reaches this slot") {
        const MorphVertex v = morph_vertex(pt(10.f, 30.f), gap(), 0.25f);
        CHECK(v.valid);
        CHECK(v.x == doctest::Approx(10.f));
        CHECK(v.y == doctest::Approx(30.f));
    }

    SUBCASE("only the capture reached it") {
        const MorphVertex v = morph_vertex(gap(), pt(10.f, 30.f), 0.25f);
        CHECK(v.valid);
        CHECK(v.x == doctest::Approx(10.f));
        CHECK(v.y == doctest::Approx(30.f));
    }
}

TEST_CASE("a slot neither side defines stays invalid so the polyline breaks") {
    CHECK(morph_vertex(gap(), gap(), 0.5f).valid == false);
}

TEST_CASE("the capture stops contributing once the morph lands") {
    const LineMorph m = capture(LineKey{LineKind::Atr}, 40);

    CHECK(morph_line_count(&m, 0.f) == 40);
    CHECK(morph_line_count(&m, 0.999f) == 40);
    // Mirrors morph_from_count: at 1 the draw path collapses back to the new
    // series alone, which is what releases the capture.
    CHECK(morph_line_count(&m, 1.f) == 0);
    // An indicator enabled mid-morph has no capture at all.
    CHECK(morph_line_count(nullptr, 0.5f) == 0);
}

TEST_CASE("lookup pairs a series only with its own capture") {
    std::vector<LineMorph> lines;
    lines.push_back(capture(LineKey{LineKind::Overlay, 0, 7}, 3));
    lines.push_back(capture(LineKey{LineKind::Overlay, 1, 9}, 5));
    lines.push_back(capture(LineKey{LineKind::Rsi}, 8));

    SUBCASE("matching key") {
        const LineMorph* m = find_line_morph(lines, LineKey{LineKind::Overlay, 1, 9});
        REQUIRE(m != nullptr);
        CHECK(m->pts.size() == 5);
        CHECK(find_line_morph(lines, LineKey{LineKind::Rsi})->pts.size() == 8);
    }

    SUBCASE("an overlay that changed period between frames finds nothing") {
        // Same slot, different MA — pairing on index alone would reshape one
        // average's line out of another's geometry.
        CHECK(find_line_morph(lines, LineKey{LineKind::Overlay, 1, 4}) == nullptr);
    }

    SUBCASE("an overlay that shifted position finds nothing") {
        CHECK(find_line_morph(lines, LineKey{LineKind::Overlay, 2, 9}) == nullptr);
    }

    SUBCASE("a series that was not captured finds nothing") {
        CHECK(find_line_morph(lines, LineKey{LineKind::MacdSignal}) == nullptr);
    }
}
