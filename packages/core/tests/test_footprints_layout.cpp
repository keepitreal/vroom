#include "doctest.h"

#include <vector>

#include "footprints_layout.h"

using vroom::footprints::BadgeLayout;
using vroom::footprints::Bucket;
using vroom::footprints::bucket_index;
using vroom::footprints::build_buckets;
using vroom::footprints::hits_badge;
using vroom::footprints::layout_badge;
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
        CHECK(b[0].has_buys());
        CHECK(b[0].has_sells());
        CHECK(b[0].buys.size() == 3);
        CHECK(b[0].sells.size() == 1);
        // Latest per side drives the combined badge's dot order.
        CHECK(b[0].buy_latest_ms == 4'000);
        CHECK(b[0].sell_latest_ms == 3'000);
    }

    SUBCASE("one side only yields a one-sided bucket") {
        const std::vector<VroomFootprint> prints{fp(1'000, VROOM_FOOTPRINT_SELL)};
        const auto b = build_buckets(c.data(), c.size(), kMinute, prints.data(),
                                     prints.size());
        REQUIRE(b.size() == 1);
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
            fp(2'000, VROOM_FOOTPRINT_BOTH),       // output only, never an input
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
        for (const auto& one : b) CHECK(one.has_buys() != one.has_sells());
    }

    SUBCASE("at 1h they collapse onto one bar holding both sides") {
        std::vector<VroomCandle> c(2);
        c[0].time_ms = 0;
        c[1].time_ms = kHour;
        const auto b = build_buckets(c.data(), c.size(), kHour, prints.data(),
                                     prints.size());
        REQUIRE(b.size() == 1);
        CHECK(b[0].candle_time_ms == 0);
        CHECK(b[0].buys.size() == 2);
        CHECK(b[0].sells.size() == 2);
        CHECK(b[0].buy_latest_ms == 17 * kMinute);
        CHECK(b[0].sell_latest_ms == 59 * kMinute);
    }
}

TEST_CASE("layout_badge") {
    // A bar whose high sits at y=200 and close at y=260, in a pane starting at y=0.
    constexpr float kHighY = 200.f;
    constexpr float kCloseY = 260.f;
    constexpr float kPaneTop = 0.f;
    constexpr float kR = 9.f;
    constexpr float kM = 8.f;
    const Bucket buy_only = bucket_with(true, false, 100, 0);

    SUBCASE("in candle mode a badge clears the high by margin + radius") {
        const BadgeLayout b =
            layout_badge(buy_only, kHighY, kCloseY, 0.f, kPaneTop, kR, kM);
        CHECK(b.side == VROOM_FOOTPRINT_BUY);
        CHECK(b.y == doctest::Approx(200.f - 8.f - 9.f));
    }

    SUBCASE("in line mode a badge is centered on the close") {
        const BadgeLayout b =
            layout_badge(buy_only, kHighY, kCloseY, 1.f, kPaneTop, kR, kM);
        CHECK(b.y == doctest::Approx(kCloseY));
    }

    SUBCASE("mid-morph the badge sits proportionally between the two") {
        const BadgeLayout b =
            layout_badge(buy_only, kHighY, kCloseY, 0.5f, kPaneTop, kR, kM);
        CHECK(b.y == doctest::Approx((183.f + kCloseY) * 0.5f));
    }

    SUBCASE("an out-of-range morph clamps to the endpoints") {
        CHECK(layout_badge(buy_only, kHighY, kCloseY, -1.f, kPaneTop, kR, kM).y ==
              doctest::Approx(183.f));
        CHECK(layout_badge(buy_only, kHighY, kCloseY, 2.f, kPaneTop, kR, kM).y ==
              doctest::Approx(kCloseY));
    }

    SUBCASE("an empty bucket lays out nothing") {
        const BadgeLayout b =
            layout_badge(Bucket{}, kHighY, kCloseY, 0.f, kPaneTop, kR, kM);
        CHECK(b.side == -1);
    }

    SUBCASE("a sell-only bucket is a sell badge") {
        const BadgeLayout b = layout_badge(bucket_with(false, true, 0, 100), kHighY,
                                           kCloseY, 0.f, kPaneTop, kR, kM);
        CHECK(b.side == VROOM_FOOTPRINT_SELL);
        CHECK(b.first_side == VROOM_FOOTPRINT_SELL);
    }

    SUBCASE("both sides combine into one badge, earlier latest-trade first") {
        // The buy's last fill precedes the sell's.
        const BadgeLayout b = layout_badge(bucket_with(true, true, 100, 900), kHighY,
                                           kCloseY, 0.f, kPaneTop, kR, kM);
        CHECK(b.side == VROOM_FOOTPRINT_BOTH);
        CHECK(b.first_side == VROOM_FOOTPRINT_BUY);
        // Same place a single-side badge would sit.
        CHECK(b.y == doctest::Approx(183.f));
    }

    SUBCASE("the first side flips when the sell's latest fill came first") {
        const BadgeLayout b = layout_badge(bucket_with(true, true, 900, 100), kHighY,
                                           kCloseY, 0.f, kPaneTop, kR, kM);
        CHECK(b.side == VROOM_FOOTPRINT_BOTH);
        CHECK(b.first_side == VROOM_FOOTPRINT_SELL);
    }

    SUBCASE("equal latest fills put the buy first") {
        const BadgeLayout b = layout_badge(bucket_with(true, true, 500, 500), kHighY,
                                           kCloseY, 0.f, kPaneTop, kR, kM);
        CHECK(b.first_side == VROOM_FOOTPRINT_BUY);
    }

    SUBCASE("a badge that would overflow the pane top slides down") {
        // High near the top of the pane: the untouched badge would sit above it.
        const BadgeLayout b =
            layout_badge(buy_only, 20.f, 40.f, 0.f, kPaneTop, kR, kM);
        CHECK(b.y - kR == doctest::Approx(kPaneTop));
    }

    SUBCASE("a badge with room to spare is left alone") {
        const BadgeLayout b =
            layout_badge(buy_only, kHighY, kCloseY, 0.f, kPaneTop, kR, kM);
        CHECK(b.y - kR > kPaneTop);
    }

    SUBCASE("non-positive style values fall back to the defaults") {
        const BadgeLayout b =
            layout_badge(buy_only, kHighY, kCloseY, 0.f, kPaneTop, 0.f, -1.f);
        CHECK(b.y == doctest::Approx(kHighY - vroom::footprints::kMargin -
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
        CHECK(m.margin == doctest::Approx(vroom::footprints::kMargin));
        CHECK(m.hover_boost == doctest::Approx(vroom::footprints::kHoverBoost));
    }

    SUBCASE("set fields win, unset fields default") {
        VroomFootprintStyle s{};
        s.radius_px = 12.f;
        s.margin_px = 0.f;  // unset
        s.hover_boost = 1.f;
        const auto m = metrics_from(&s);
        CHECK(m.radius == doctest::Approx(12.f));
        CHECK(m.margin == doctest::Approx(vroom::footprints::kMargin));
        CHECK(m.hover_boost == doctest::Approx(1.f));
    }
}
