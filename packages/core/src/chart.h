// Internal definition of `struct VroomChart` — the central state bag for a
// single chart instance. Lives in this internal header so the various
// rendering / facade modules (chart.cpp, chart_facade.cpp, labels.cpp,
// candles.cpp, …) can all see the same layout.
//
// The public C API in `vroom/vroom_chart.h` keeps `VroomChart` opaque, so
// consumers never include this header.

#pragma once

#include <chrono>
#include <vector>

#include "vroom/vroom_chart.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkPicture.h"  // SkPicture must be complete so the
                                     // sk_sp<SkPicture> field's destructor
                                     // is callable wherever VroomChart is.
#include "include/core/SkRefCnt.h"
#pragma clang diagnostic pop

#include "labels.h"
#include "theme.h"
#include "viewport.h"

class SkCanvas;

struct VroomChart {
    // --- bridge / lifecycle -------------------------------------------------
    VroomCallbacks cb{};
    void* user_ctx = nullptr;

    // --- data ---------------------------------------------------------------
    std::vector<VroomCandle> candles;

    // Inferred from the gap between consecutive candles when data is loaded.
    // Each candle occupies a fixed pixel slot of (duration / window) × width
    // so the visual size is stable as the user pans / scrolls into "future"
    // empty space past the latest candle.
    int64_t candle_duration_ms = 60'000;

    // --- layout -------------------------------------------------------------
    float width_px = 0.f;
    float height_px = 0.f;
    float px_ratio = 1.f;

    // 0/0 = uninitialized; set_candles defaults to last ~80 candles.
    int64_t visible_start_ms = 0;
    int64_t visible_end_ms = 0;

    // Cached y-axis width in pixels, sized to fit the widest formatted price
    // label. 0 = uncomputed; layout() falls back to a width ratio.
    float axis_width_px = 0.f;

    // --- price bounds (y-axis state) ---------------------------------------
    // Two modes. Auto (price_bounds_manual == false, the default): the y-range
    // continuously follows the visible candles each frame and `price_bounds`
    // is ignored. Manual: the user has touched the y-axis (pinch zoom, axis
    // drag, vertical drag-translate) and `price_bounds` is frozen until
    // vroom_chart_reset_view / reset_price_scale re-enables auto mode.
    vroom::PriceBounds price_bounds{0.0, 1.0};
    bool price_bounds_manual = false;

    // --- crosshair (not yet drawn) -----------------------------------------
    bool   crosshair_active = false;
    float  crosshair_x_px = 0.f;
    float  crosshair_y_px = 0.f;

    // --- indicators ---------------------------------------------------------
    // RSI, drawn in a pane below the candles. rsi_cache is aligned to `candles`
    // (one value per candle, NaN where undefined) and recomputed lazily by
    // ensure_rsi() when rsi_dirty (set on any data or config change).
    bool rsi_enabled = false;
    int  rsi_period = 14;
    double rsi_upper = 70.0;   // overbought band
    double rsi_lower = 30.0;   // oversold band
    bool rsi_ma_enabled = true;  // RSI-based moving average (trendline)
    int  rsi_ma_period = 14;
    std::vector<double> rsi_cache;     // RSI per candle (NaN where undefined)
    std::vector<double> rsi_ma_cache;  // SMA of rsi_cache (empty if MA off)
    bool rsi_dirty = true;
    // User y-axis zoom for the RSI pane (drag on its y-axis strip). 1.0 = the
    // default 0..100 fit; >1 zooms in (taller), <1 zooms out. Anchored at 50.
    double rsi_y_scale = 1.0;

    // MACD, drawn in its own pane. Caches aligned to `candles` (NaN warmup).
    bool macd_enabled = false;
    int  macd_fast = 12;
    int  macd_slow = 26;
    int  macd_signal = 9;
    std::vector<double> macd_cache;
    std::vector<double> macd_signal_cache;
    std::vector<double> macd_hist_cache;
    bool macd_dirty = true;
    // User y-axis zoom for the MACD pane (drag on its y-axis strip). 1.0 = the
    // default auto-fit amplitude; >1 zooms in, <1 zooms out. Anchored at zero.
    double macd_y_scale = 1.0;

    // Stacking order for the indicator panes: each indicator gets the next
    // sequence number on its off->on transition, so the most recently enabled
    // pane sorts last (bottom). -1 = not currently enabled.
    int rsi_order = -1;
    int macd_order = -1;
    int pane_seq = 0;

    // Moving-average overlay lines (SMA/EMA) drawn on the price pane. Not panes
    // — they don't reserve indicator_area_h. overlay_caches[i] is the computed
    // series for overlays[i], aligned to `candles` (NaN warmup).
    std::vector<VroomOverlay> overlays;
    std::vector<std::vector<double>> overlay_caches;
    bool overlays_dirty = true;

    // VWAP overlay (session anchor, configurable reset time). Single line on the
    // price pane; vwap_breaks marks session resets so the line lifts the pen.
    bool vwap_enabled = false;
    int  vwap_reset_offset_min = 0;       // session boundary offset from UTC midnight
    uint32_t vwap_color = 0xff00bcd4;     // cyan
    float vwap_width = 1.5f;
    std::vector<double> vwap_cache;
    std::vector<unsigned char> vwap_breaks;
    bool vwap_dirty = true;

    // --- drawings (line annotations) ---------------------------------------
    // Committed two-point lines, anchored in data space so they track the
    // candles on pan/zoom. Drawn on the price pane above candles/overlays.
    std::vector<VroomDrawing> drawings;

    // Transient in-progress "draft" the drawing tool shows while placing points.
    // draft_a is always drawn (node dot); draft_b is drawn when draft_has_b.
    // draft_guide draws the live guideline A->B; when false only node dots show
    // (the committed segment renders via `drawings`). draft_color/draft_width
    // style the guideline to match the eventual line.
    bool           draft_active = false;
    VroomDrawPoint draft_a{};
    bool           draft_has_b = false;
    VroomDrawPoint draft_b{};
    bool           draft_guide = false;
    uint32_t       draft_color = 0xff2962ff;
    float          draft_width = 2.f;

    // --- theme --------------------------------------------------------------
    vroom::Theme theme;

    // --- picture cache ------------------------------------------------------
    sk_sp<SkPicture> chart_picture;
    bool chart_dirty = true;

    // --- label fade animation ----------------------------------------------
    std::vector<vroom::labels::YLabelFade> y_fades;
    std::vector<vroom::labels::XLabelFade> x_fades;
    std::chrono::steady_clock::time_point last_anim_tick{};
    bool anim_started = false;

    // dt of the current rebuild — populated by rebuild_chart_picture and
    // consumed by the label-fade updaters. Public so the labels namespace
    // can read it without a getter.
    float last_dt_seconds = 0.f;

    // --- methods ------------------------------------------------------------
    VroomChart();

    // Marks the chart_picture dirty and fires the on_redraw_requested
    // callback (if any).
    void mark_dirty();

    // Builds a Layout snapshot for the current geometry / theme / axis sizing.
    vroom::Layout layout() const;

    // Recomputes rsi_cache (over the full candle series) when rsi_dirty and
    // RSI is enabled. No-op otherwise.
    void ensure_rsi();

    // Recomputes the MACD caches when macd_dirty and MACD is enabled.
    void ensure_macd();

    // Recomputes overlay_caches (one per overlay line) when overlays_dirty.
    void ensure_overlays();

    // Recomputes the VWAP cache when vwap_dirty and VWAP is enabled.
    void ensure_vwap();

    // The main drawing pass. Calls into the labels and candles modules.
    void draw_chart(SkCanvas* canvas);

    // Re-records `chart_picture` by invoking draw_chart on a fresh
    // SkPictureRecorder. Also computes `last_dt_seconds` for fade animation.
    void rebuild_chart_picture();

    // True if any axis label is mid-fade. Used by the JS-side animation loop
    // to know when to keep ticking.
    bool is_animating_now() const;
};
