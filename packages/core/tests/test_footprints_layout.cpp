#include "doctest.h"

#include <vector>

#include "footprints_layout.h"

using vroom::footprints::Bucket;
using vroom::footprints::Stack;
using vroom::footprints::bucket_index;
using vroom::footprints::build_buckets;
using vroom::footprints::hits_badge;
using vroom::footprints::layout_stack;
using vroom::footprints::metrics_from;

namespace {

constexpr int64_t kMinute = 60'000;
constexpr int64_t kHour = 60 * kMinute;

// Five one-minute candles opening at t=0. Only time_ms and high matter here.
std::vector<VroomCandle> minute_candles(size_t n = 5, int64_t start = 0) {
    std::vector<VroomCandle> out(n);
    for (size_t i = 0; i < n; ++i) {
        out[i].time_ms = start + static_cast<int64_t>(i) * kMinute;
        out[i].open = out[i].close = 100.0;
        out[i].high = 101.0;
        out[i].low = 99.0;
    }
    return out;
}

VroomFootprint fp(int64_t t, int32_t side) {
    VroomFootprint f{};
    f.time_ms = t;
    f.side = side;
    return f;
}

Bucket bucket_with(bool buys, bool sells, int64_t buy_latest, int64_t sell_latest) {
    Bucket b;
    if (buys) {
        b.buys.push_back(0);
        b.buy_latest_ms = buy_latest;
    }
    if (sells) {
        b.sells.push_back(1);
        b.sell_latest_ms = sell_latest;
    }
    return b;
}

}  // namespace

TEST_CASE("bucket_index") {
    const auto c = minute_candles();

    SUBCASE("a trade lands in the bar whose window contains it") {
        CHECK(bucket_index(c.data(), c.size(), kMinute, 0) == 0);
        CHECK(bucket_index(c.data(), c.size(), kMinute, 30'000) == 0);
        CHECK(bucket_index(c.data(), c.size(), kMinute, 2 * kMinute + 1) == 2);
    }

    SUBCASE("window edges are half-open") {
        // The bar's own open belongs to it; the next open belongs to the next bar.
        CHECK(bucket_index(c.data(), c.size(), kMinute, kMinute - 1) == 0);
        CHECK(bucket_index(c.data(), c.size(), kMinute, kMinute) == 1);
    }

    SUBCASE("outside every window") {
        CHECK(bucket_index(c.data(), c.size(), kMinute, -1) == -1);
        // One ms past the last bar's close.
        CHECK(bucket_index(c.data(), c.size(), kMinute, 5 * kMinute) == -1);
        CHECK(bucket_index(c.data(), c.size(), kMinute, 99 * kMinute) == -1);
    }

    SUBCASE("a gap between bars swallows nothing") {
        // Bars at 0 and at 10m, still 1m wide: a trade at 5m has no bar.
        std::vector<VroomCandle> gapped = minute_candles(1);
        VroomCandle late{};
        late.time_ms = 10 * kMinute;
        gapped.push_back(late);
        CHECK(bucket_index(gapped.data(), gapped.size(), kMinute, 5 * kMinute) == -1);
        CHECK(bucket_index(gapped.data(), gapped.size(), kMinute, 10 * kMinute) == 1);
    }

    SUBCASE("degenerate args") {
        CHECK(bucket_index(nullptr, 0, kMinute, 0) == -1);
        CHECK(bucket_index(c.data(), 0, kMinute, 0) == -1);
        CHECK(bucket_index(c.data(), c.size(), 0, 0) == -1);
        CHECK(bucket_index(c.data(), c.size(), -kMinute, 0) == -1);
    }
}

TEST_CASE("build_buckets groups by candle and side") {
    const auto c = minute_candles();

    SUBCASE("many trades on one bar collapse to one bucket with two sides") {
        const std::vector<VroomFootprint> prints{
            fp(1'000, VROOM_FOOTPRINT_BUY),
            fp(2'000, VROOM_FOOTPRINT_BUY),
            fp(3'000, VROOM_FOOTPRINT_SELL),
            fp(4'000, VROOM_FOOTPRINT_BUY),
        };
        const auto b = build_buckets(c.data(), c.size(), kMinute, prints.data(),
                                     prints.size());
        REQUIRE(b.size() == 1);
        CHECK(b[0].candle_time_ms == 0);
        CHECK(b[0].badge_count() == 2);
        CHECK(b[0].buys.size() == 3);
        CHECK(b[0].sells.size() == 1);
        // Latest per side drives the stack order.
        CHECK(b[0].buy_latest_ms == 4'000);
        CHECK(b[0].sell_latest_ms == 3'000);
    }

    SUBCASE("one side only yields one badge") {
        const std::vector<VroomFootprint> prints{fp(1'000, VROOM_FOOTPRINT_SELL)};
        const auto b = build_buckets(c.data(), c.size(), kMinute, prints.data(),
                                     prints.size());
        REQUIRE(b.size() == 1);
        CHECK(b[0].badge_count() == 1);
        CHECK_FALSE(b[0].has_buys());
        CHECK(b[0].has_sells());
    }

    SUBCASE("indices are into the host's array and sorted by time") {
        // Host order is deliberately shuffled.
        const std::vector<VroomFootprint> prints{
            fp(50'000, VROOM_FOOTPRINT_BUY),  // index 0, latest
            fp(10'000, VROOM_FOOTPRINT_BUY),  // index 1, earliest
            fp(30'000, VROOM_FOOTPRINT_BUY),  // index 2
        };
        const auto b = build_buckets(c.data(), c.size(), kMinute, prints.data(),
                                     prints.size());
        REQUIRE(b.size() == 1);
        REQUIRE(b[0].buys.size() == 3);
        CHECK(b[0].buys[0] == 1);
        CHECK(b[0].buys[1] == 2);
        CHECK(b[0].buys[2] == 0);
        CHECK(b[0].buy_latest_ms == 50'000);
    }

    SUBCASE("buckets come back ascending by candle even from shuffled input") {
        const std::vector<VroomFootprint> prints{
            fp(3 * kMinute, VROOM_FOOTPRINT_BUY),
            fp(1 * kMinute, VROOM_FOOTPRINT_BUY),
            fp(2 * kMinute, VROOM_FOOTPRINT_SELL),
        };
        const auto b = build_buckets(c.data(), c.size(), kMinute, prints.data(),
                                     prints.size());
        REQUIRE(b.size() == 3);
        CHECK(b[0].candle_time_ms == 1 * kMinute);
        CHECK(b[1].candle_time_ms == 2 * kMinute);
        CHECK(b[2].candle_time_ms == 3 * kMinute);
    }

    SUBCASE("out-of-range trades and unknown sides are dropped") {
        const std::vector<VroomFootprint> prints{
            fp(-1, VROOM_FOOTPRINT_BUY),          // before the first bar
            fp(99 * kMinute, VROOM_FOOTPRINT_BUY),  // past the last
            fp(1'000, 7),                          // not a side we know
        };
        const auto b = build_buckets(c.data(), c.size(), kMinute, prints.data(),
                                     prints.size());
        CHECK(b.empty());
    }

    SUBCASE("degenerate args") {
        const std::vector<VroomFootprint> prints{fp(0, VROOM_FOOTPRINT_BUY)};
        CHECK(build_buckets(nullptr, 0, kMinute, prints.data(), 1).empty());
        CHECK(build_buckets(c.data(), c.size(), kMinute, nullptr, 0).empty());
        CHECK(build_buckets(c.data(), c.size(), kMinute, prints.data(), 0).empty());
    }
}

TEST_CASE("build_buckets re-groups when the interval widens") {
    // The same four fills, spread across the first hour.
    const std::vector<VroomFootprint> prints{
        fp(0 * kMinute + 100, VROOM_FOOTPRINT_BUY),
        fp(17 * kMinute, VROOM_FOOTPRINT_BUY),
        fp(41 * kMinute, VROOM_FOOTPRINT_SELL),
        fp(59 * kMinute, VROOM_FOOTPRINT_SELL),
    };

    SUBCASE("at 1m they land on four separate bars") {
        const auto c = minute_candles(60);
        const auto b = build_buckets(c.data(), c.size(), kMinute, prints.data(),
                                     prints.size());
        REQUIRE(b.size() == 4);
        for (const auto& one : b) CHECK(one.badge_count() == 1);
    }

    SUBCASE("at 1h they collapse onto one bar as two badges") {
        std::vector<VroomCandle> c(2);
        c[0].time_ms = 0;
        c[1].time_ms = kHour;
        const auto b = build_buckets(c.data(), c.size(), kHour, prints.data(),
                                     prints.size());
        REQUIRE(b.size() == 1);
        CHECK(b[0].candle_time_ms == 0);
        CHECK(b[0].badge_count() == 2);
        CHECK(b[0].buys.size() == 2);
        CHECK(b[0].sells.size() == 2);
        CHECK(b[0].buy_latest_ms == 17 * kMinute);
        CHECK(b[0].sell_latest_ms == 59 * kMinute);
    }
}

TEST_CASE("layout_stack") {
    // A bar whose high sits at y=200, in a pane starting at y=0.
    constexpr float kHighY = 200.f;
    constexpr float kPaneTop = 0.f;
    constexpr float kR = 9.f;
    constexpr float kG = 4.f;
    constexpr float kM = 8.f;

    SUBCASE("a single badge clears the high by margin + radius") {
        const Stack s = layout_stack(bucket_with(true, false, 100, 0), kHighY,
                                     kPaneTop, kR, kG, kM);
        REQUIRE(s.count == 1);
        CHECK(s.sides[0] == VROOM_FOOTPRINT_BUY);
        CHECK(s.y[0] == doctest::Approx(200.f - 8.f - 9.f));
    }

    SUBCASE("an empty bucket lays out nothing") {
        const Stack s = layout_stack(Bucket{}, kHighY, kPaneTop, kR, kG, kM);
        CHECK(s.count == 0);
    }

    SUBCASE("both sides stack upward, earlier latest-trade nearest the bar") {
        // The buy's last fill precedes the sell's, so the buy sits lower.
        const Stack s = layout_stack(bucket_with(true, true, 100, 900), kHighY,
                                     kPaneTop, kR, kG, kM);
        REQUIRE(s.count == 2);
        CHECK(s.sides[0] == VROOM_FOOTPRINT_BUY);
        CHECK(s.sides[1] == VROOM_FOOTPRINT_SELL);
        CHECK(s.y[0] == doctest::Approx(183.f));
        // One pitch (2r + gap = 22) higher, i.e. a smaller y.
        CHECK(s.y[1] == doctest::Approx(161.f));
        // The gap is what keeps them separately hoverable.
        CHECK(s.y[0] - s.y[1] - 2.f * kR == doctest::Approx(kG));
    }

    SUBCASE("order flips when the sell's latest fill came first") {
        const Stack s = layout_stack(bucket_with(true, true, 900, 100), kHighY,
                                     kPaneTop, kR, kG, kM);
        REQUIRE(s.count == 2);
        CHECK(s.sides[0] == VROOM_FOOTPRINT_SELL);
        CHECK(s.sides[1] == VROOM_FOOTPRINT_BUY);
    }

    SUBCASE("equal latest fills put the buy nearest the bar") {
        const Stack s = layout_stack(bucket_with(true, true, 500, 500), kHighY,
                                     kPaneTop, kR, kG, kM);
        REQUIRE(s.count == 2);
        CHECK(s.sides[0] == VROOM_FOOTPRINT_BUY);
    }

    SUBCASE("a stack that would overflow the pane top slides down") {
        // High near the top of the pane: the untouched stack would sit above it.
        const Stack s = layout_stack(bucket_with(true, true, 100, 900), 20.f,
                                     kPaneTop, kR, kG, kM);
        REQUIRE(s.count == 2);
        // Topmost badge is flush inside the pane...
        CHECK(s.y[1] - kR == doctest::Approx(kPaneTop));
        // ...and the pair keeps its spacing and its order.
        CHECK(s.y[0] - s.y[1] == doctest::Approx(2.f * kR + kG));
        CHECK(s.sides[0] == VROOM_FOOTPRINT_BUY);
    }

    SUBCASE("a stack with room to spare is left alone") {
        const Stack s = layout_stack(bucket_with(true, true, 100, 900), kHighY,
                                     kPaneTop, kR, kG, kM);
        CHECK(s.y[1] - kR > kPaneTop);
    }

    SUBCASE("non-positive style values fall back to the defaults") {
        const Stack s = layout_stack(bucket_with(true, false, 100, 0), kHighY,
                                     kPaneTop, 0.f, -1.f, 0.f);
        REQUIRE(s.count == 1);
        CHECK(s.y[0] == doctest::Approx(kHighY - vroom::footprints::kMargin -
                                        vroom::footprints::kRadius));
    }
}

TEST_CASE("hits_badge") {
    const float tol = vroom::footprints::kHitTolerance;

    CHECK(hits_badge(100.f, 50.f, 9.f, 100.f, 50.f));       // dead center
    CHECK(hits_badge(100.f, 50.f, 9.f, 108.f, 50.f));       // inside the circle
    CHECK(hits_badge(100.f, 50.f, 9.f, 100.f + 9.f + tol, 50.f));  // on the slop edge
    CHECK_FALSE(hits_badge(100.f, 50.f, 9.f, 100.f + 9.f + tol + 1.f, 50.f));
    CHECK_FALSE(hits_badge(100.f, 50.f, 9.f, 100.f, 90.f));

    SUBCASE("the target is round, not square") {
        // A corner of the bounding box is outside the circle.
        const float d = 9.f + tol;
        CHECK_FALSE(hits_badge(0.f, 0.f, 9.f, d, d));
    }

    SUBCASE("a degenerate radius hits nothing") {
        CHECK_FALSE(hits_badge(100.f, 50.f, 0.f, 100.f, 50.f));
        CHECK_FALSE(hits_badge(100.f, 50.f, -9.f, 100.f, 50.f));
    }
}

TEST_CASE("metrics_from") {
    SUBCASE("a null style is all defaults") {
        const auto m = metrics_from(nullptr);
        CHECK(m.radius == doctest::Approx(vroom::footprints::kRadius));
        CHECK(m.gap == doctest::Approx(vroom::footprints::kGap));
        CHECK(m.margin == doctest::Approx(vroom::footprints::kMargin));
        CHECK(m.hover_boost == doctest::Approx(vroom::footprints::kHoverBoost));
    }

    SUBCASE("set fields win, unset fields default") {
        VroomFootprintStyle s{};
        s.radius_px = 12.f;
        s.gap_px = 0.f;  // unset
        s.margin_px = 3.f;
        s.hover_boost = 1.f;
        const auto m = metrics_from(&s);
        CHECK(m.radius == doctest::Approx(12.f));
        CHECK(m.gap == doctest::Approx(vroom::footprints::kGap));
        CHECK(m.margin == doctest::Approx(3.f));
        CHECK(m.hover_boost == doctest::Approx(1.f));
    }
}
