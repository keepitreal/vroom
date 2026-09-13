#include "doctest.h"

#include <cmath>
#include <vector>

#include "ichimoku.h"

namespace {
constexpr int64_t kStep = 60'000;

// Bars with independent highs and lows, so the midpoint lines are distinct from
// the closes and a wrong source shows up immediately.
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

struct Series {
    std::vector<double> tenkan, kijun, senkou_a, senkou_b, chikou;
};

Series run(const std::vector<VroomCandle>& c, int tp, int kp, int sp) {
    Series s;
    vroom::ichimoku::compute(c.data(), c.size(), tp, kp, sp, s.tenkan, s.kijun,
                             s.senkou_a, s.senkou_b, s.chikou);
    return s;
}
}  // namespace

TEST_CASE("ichimoku::compute midpoints, hand-computed") {
    // high/low/close per bar.
    auto c = from_hlc({{10, 6, 8}, {14, 8, 12}, {12, 4, 5}, {20, 10, 18}});
    auto s = run(c, 2, 3, 4);

    REQUIRE(s.tenkan.size() == 4);
    REQUIRE(s.kijun.size() == 4);
    REQUIRE(s.senkou_a.size() == 4);
    REQUIRE(s.senkou_b.size() == 4);
    REQUIRE(s.chikou.size() == 4);

    // Tenkan, period 2: midpoint of the trailing two-bar high/low range.
    CHECK(std::isnan(s.tenkan[0]));
    CHECK(s.tenkan[1] == doctest::Approx((14.0 + 6.0) / 2));   // hi 14, lo 6
    CHECK(s.tenkan[2] == doctest::Approx((14.0 + 4.0) / 2));   // hi 14, lo 4
    CHECK(s.tenkan[3] == doctest::Approx((20.0 + 4.0) / 2));   // hi 20, lo 4

    // Kijun, period 3.
    CHECK(std::isnan(s.kijun[0]));
    CHECK(std::isnan(s.kijun[1]));
    CHECK(s.kijun[2] == doctest::Approx((14.0 + 4.0) / 2));
    CHECK(s.kijun[3] == doctest::Approx((20.0 + 4.0) / 2));

    // Senkou B, period 4 — only the last bar has a full window.
    for (std::size_t i = 0; i < 3; ++i) CHECK(std::isnan(s.senkou_b[i]));
    CHECK(s.senkou_b[3] == doctest::Approx((20.0 + 4.0) / 2));

    // Senkou A averages the two, so it inherits the longer warmup.
    CHECK(std::isnan(s.senkou_a[0]));
    CHECK(std::isnan(s.senkou_a[1]));
    CHECK(s.senkou_a[2] == doctest::Approx((s.tenkan[2] + s.kijun[2]) / 2));
    CHECK(s.senkou_a[3] == doctest::Approx((s.tenkan[3] + s.kijun[3]) / 2));

    // Chikou is the close, undisplaced — the shift happens at draw time.
    CHECK(s.chikou[0] == doctest::Approx(8.0));
    CHECK(s.chikou[3] == doctest::Approx(18.0));
}

TEST_CASE("ichimoku::compute series shorter than a window is all NaN") {
    auto c = from_hlc({{10, 6, 8}, {14, 8, 12}});
    auto s = run(c, 9, 26, 52);
    REQUIRE(s.tenkan.size() == 2);
    for (std::size_t i = 0; i < 2; ++i) {
        CHECK(std::isnan(s.tenkan[i]));
        CHECK(std::isnan(s.kijun[i]));
        CHECK(std::isnan(s.senkou_a[i]));
        CHECK(std::isnan(s.senkou_b[i]));
        // Chikou needs no lookback, so it is defined from the first bar.
        CHECK(!std::isnan(s.chikou[i]));
    }
}

TEST_CASE("ichimoku::compute empty input and clamped periods") {
    std::vector<double> a, b, cc, d, e;
    vroom::ichimoku::compute(nullptr, 0, 9, 26, 52, a, b, cc, d, e);
    CHECK(a.empty());
    CHECK(e.empty());

    // A period below 1 leaves the series undefined rather than reading out of
    // bounds; the facade clamps before it gets here.
    auto c = from_hlc({{10, 6, 8}, {14, 8, 12}, {12, 4, 5}});
    auto s = run(c, 0, 2, 2);
    for (double v : s.tenkan) CHECK(std::isnan(v));
    CHECK(s.kijun[2] == doctest::Approx((14.0 + 4.0) / 2));
}

TEST_CASE("ichimoku::shifted_source_range forward, backward and zero shift") {
    // Five bars at t = 0, 60k, 120k, 180k, 240k.
    auto c = from_hlc({{1, 1, 1}, {1, 1, 1}, {1, 1, 1}, {1, 1, 1}, {1, 1, 1}});
    const auto* p = c.data();
    const std::size_t n = c.size();

    // Zero shift matches visible_indices exactly.
    auto r = vroom::ichimoku::shifted_source_range(p, n, kStep, 3 * kStep, 0);
    CHECK(r.start == 1);
    CHECK(r.end == 4);

    // Leading spans: a window covering the two slots past the last bar picks up
    // the two source bars whose shifted times land there.
    const int64_t lead = 2 * kStep;
    r = vroom::ichimoku::shifted_source_range(p, n, 5 * kStep, 6 * kStep, lead);
    CHECK(r.start == 3);
    CHECK(r.end == 5);

    // Everything shifted forward: the whole series is reachable from a window
    // that starts after the last real bar.
    r = vroom::ichimoku::shifted_source_range(p, n, lead, 4 * kStep + lead, lead);
    CHECK(r.start == 0);
    CHECK(r.end == n);

    // Chikou lags, so the bars it draws in a given window come from later.
    r = vroom::ichimoku::shifted_source_range(p, n, 0, kStep, -lead);
    CHECK(r.start == 2);
    CHECK(r.end == 4);

    // Past the end in either direction is empty, not clamped.
    r = vroom::ichimoku::shifted_source_range(p, n, 20 * kStep, 21 * kStep, lead);
    CHECK(r.start == r.end);
}

TEST_CASE("ichimoku::shifted_source_range passes the unframed sentinel through") {
    auto c = from_hlc({{1, 1, 1}, {1, 1, 1}, {1, 1, 1}});
    // 0/0 means "no viewport yet, draw everything" — a shift must not turn that
    // into an empty range.
    auto r = vroom::ichimoku::shifted_source_range(c.data(), c.size(), 0, 0,
                                                   26 * kStep);
    CHECK(r.start == 0);
    CHECK(r.end == c.size());
}
