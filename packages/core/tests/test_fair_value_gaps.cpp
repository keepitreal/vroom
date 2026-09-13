#include "doctest.h"

#include <vector>

#include "fair_value_gaps.h"

namespace {
constexpr int64_t kStep = 60'000;

// Bars given as high/low/close, one per row, spaced kStep apart. Detection reads
// highs and lows; close matters only to the kFillClose scan.
std::vector<VroomCandle> from_hlc(
    std::initializer_list<std::initializer_list<double>> rows) {
    std::vector<VroomCandle> out;
    int64_t t = 0;
    for (const auto& r : rows) {
        const double hi = *r.begin();
        const double lo = *(r.begin() + 1);
        const double cl = *(r.begin() + 2);
        out.push_back(VroomCandle{t, cl, hi, lo, cl, 0.0});
        t += kStep;
    }
    return out;
}

std::vector<vroom::fvg::Gap> run(const std::vector<VroomCandle>& c,
                                 int max_bars_back = 1000,
                                 bool wait_for_close = false,
                                 int fill_type = vroom::fvg::kFillClose) {
    std::vector<vroom::fvg::Gap> out;
    vroom::fvg::compute(c.data(), c.size(), max_bars_back, wait_for_close,
                        fill_type, out);
    return out;
}
}  // namespace

TEST_CASE("fvg::compute finds a bullish gap between the outer wicks") {
    // Bar 0 tops out at 10, bar 2 bottoms at 14 — 10..14 was skipped.
    auto c = from_hlc({{10, 6, 9}, {15, 9, 14}, {18, 14, 17}});
    auto g = run(c);

    REQUIRE(g.size() == 1);
    CHECK(g[0].bullish);
    CHECK(g[0].time_ms == kStep);  // anchored to the middle bar
    CHECK(g[0].bottom == doctest::Approx(10.0));
    CHECK(g[0].top == doctest::Approx(14.0));
    CHECK(g[0].filled_ms == 0);
}

TEST_CASE("fvg::compute finds a bearish gap between the outer wicks") {
    // Bar 0 bottoms at 20, bar 2 tops at 16 — 16..20 was skipped.
    auto c = from_hlc({{24, 20, 21}, {21, 15, 16}, {16, 12, 13}});
    auto g = run(c);

    REQUIRE(g.size() == 1);
    CHECK_FALSE(g[0].bullish);
    CHECK(g[0].time_ms == kStep);
    CHECK(g[0].bottom == doctest::Approx(16.0));
    CHECK(g[0].top == doctest::Approx(20.0));
}

TEST_CASE("fvg::compute ignores overlapping and exactly touching wicks") {
    SUBCASE("overlap") {
        auto c = from_hlc({{12, 6, 11}, {15, 9, 14}, {18, 10, 17}});
        CHECK(run(c).empty());
    }
    SUBCASE("touching — high == low leaves no band behind") {
        auto c = from_hlc({{10, 6, 9}, {15, 9, 14}, {18, 10, 17}});
        CHECK(run(c).empty());
    }
}

TEST_CASE("fvg::compute windows the scan to the last max_bars_back bars") {
    // A climb with a gap around bar 1 (10..14) and another around bar 5
    // (20..25). Bars 3 and 4 overlap their neighbours, so nothing forms there.
    auto c = from_hlc({{10, 6, 9},
                       {15, 9, 14},
                       {18, 14, 17},
                       {19, 15, 18},
                       {20, 16, 19},
                       {26, 19, 25},
                       {30, 25, 29}});

    CHECK(run(c).size() == 2);

    SUBCASE("a short window keeps only the recent gap") {
        auto g = run(c, 3);
        REQUIRE(g.size() == 1);
        CHECK(g[0].time_ms == 5 * kStep);
    }
    SUBCASE("zero scans nothing") { CHECK(run(c, 0).empty()); }
}

TEST_CASE("fvg::compute wait_for_close withholds a gap on the live bar") {
    auto c = from_hlc({{10, 6, 9}, {15, 9, 14}, {18, 14, 17}});

    CHECK(run(c, 1000, /*wait_for_close=*/false).size() == 1);
    CHECK(run(c, 1000, /*wait_for_close=*/true).empty());

    // One more closed bar and the same gap is admitted.
    c.push_back(VroomCandle{3 * kStep, 17, 19, 16, 18, 0.0});
    CHECK(run(c, 1000, /*wait_for_close=*/true).size() == 1);
}

TEST_CASE("fvg::compute resolves the fill at the first bar to reach the edge") {
    // Bullish gap 10..14 around bar 1. Bar 3 wicks into it without closing
    // through; bar 4 closes below its bottom.
    auto c = from_hlc({{10, 6, 9},
                       {15, 9, 14},
                       {18, 14, 17},
                       {17, 9, 13},
                       {14, 8, 9}});

    SUBCASE("close mode waits for a close past the far edge") {
        auto g = run(c, 1000, false, vroom::fvg::kFillClose);
        REQUIRE(g.size() == 1);
        CHECK(g[0].filled_ms == 4 * kStep);
    }
    SUBCASE("wick mode settles as soon as a low reaches through") {
        auto g = run(c, 1000, false, vroom::fvg::kFillWick);
        REQUIRE(g.size() == 1);
        CHECK(g[0].filled_ms == 3 * kStep);
    }
}

TEST_CASE("fvg::compute fills a bearish gap from above") {
    // Bearish gap 16..20 around bar 1, then a push back up through the top.
    auto c = from_hlc({{24, 20, 21}, {21, 15, 16}, {16, 12, 13}, {22, 14, 21}});

    auto g = run(c);
    REQUIRE(g.size() == 1);
    CHECK_FALSE(g[0].bullish);
    CHECK(g[0].filled_ms == 3 * kStep);
}

TEST_CASE("fvg::compute inverts a filled bullish gap and leaves it live") {
    // Bullish gap 10..14 around bar 1, closed through at bar 4. Price then
    // drifts lower, so the bearish zone it inverted into still stands.
    auto c = from_hlc({{10, 6, 9},
                       {15, 9, 14},
                       {18, 14, 17},
                       {17, 9, 13},
                       {14, 8, 9},
                       {11, 7, 8},
                       {10, 6, 7}});

    auto g = run(c);
    REQUIRE(g.size() == 1);
    CHECK(g[0].filled_ms == 4 * kStep);
    CHECK(g[0].invalidated_ms == 0);
}

TEST_CASE("fvg::compute ends the inversion on a close back past the far edge") {
    // Same gap and fill, but bar 5 reclaims the whole band from below.
    auto c = from_hlc({{10, 6, 9},
                       {15, 9, 14},
                       {18, 14, 17},
                       {17, 9, 13},
                       {14, 8, 9},
                       {16, 9, 15}});

    auto g = run(c);
    REQUIRE(g.size() == 1);
    CHECK(g[0].filled_ms == 4 * kStep);
    CHECK(g[0].invalidated_ms == 5 * kStep);
}

TEST_CASE("fvg::compute inverts a bearish gap into support and back out") {
    // Bearish gap 16..20 around bar 1. Bar 3 closes above its top, turning it
    // into support; bar 5 closes below its bottom and the zone is done.
    auto c = from_hlc({{24, 20, 21},
                       {21, 15, 16},
                       {16, 12, 13},
                       {22, 14, 21},
                       {22, 15, 20},
                       {21, 14, 15}});

    auto g = run(c);
    REQUIRE(g.size() == 1);
    CHECK_FALSE(g[0].bullish);
    CHECK(g[0].filled_ms == 3 * kStep);
    CHECK(g[0].invalidated_ms == 5 * kStep);
}

TEST_CASE("fvg::compute invalidates on a wick when fill_type is wick") {
    // Bullish gap 10..14. Bar 3 wicks below the bottom, bar 4 wicks back to
    // the top — neither bar closes past either edge.
    auto c = from_hlc({{10, 6, 9},
                       {15, 9, 14},
                       {18, 14, 17},
                       {17, 9, 13},
                       {14, 8, 9}});

    auto g = run(c, 1000, false, vroom::fvg::kFillWick);
    REQUIRE(g.size() == 1);
    CHECK(g[0].filled_ms == 3 * kStep);
    CHECK(g[0].invalidated_ms == 4 * kStep);
}

TEST_CASE("fvg::compute ignores bars past the far edge before the fill") {
    // Bar 3 closes at 20, well above the gap's top, but the gap has not been
    // filled yet — there is no inversion for it to invalidate.
    auto c = from_hlc({{10, 6, 9},
                       {15, 9, 14},
                       {18, 14, 17},
                       {21, 14, 20},
                       {19, 8, 9}});

    auto g = run(c);
    REQUIRE(g.size() == 1);
    CHECK(g[0].filled_ms == 4 * kStep);
    CHECK(g[0].invalidated_ms == 0);
}

TEST_CASE("fvg::compute handles degenerate input") {
    std::vector<vroom::fvg::Gap> out{vroom::fvg::Gap{}};

    SUBCASE("null series clears the output") {
        vroom::fvg::compute(nullptr, 0, 300, false, vroom::fvg::kFillClose, out);
        CHECK(out.empty());
    }
    SUBCASE("fewer than three bars can't form a gap") {
        auto c = from_hlc({{10, 6, 9}, {18, 14, 17}});
        vroom::fvg::compute(c.data(), c.size(), 300, false,
                            vroom::fvg::kFillClose, out);
        CHECK(out.empty());
    }
}
