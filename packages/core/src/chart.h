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

    // 0/0 = uninitialized; set_candles defaults to last ~60 candles.
    int64_t visible_start_ms = 0;
    int64_t visible_end_ms = 0;

    // Cached y-axis width in pixels, sized to fit the widest formatted price
    // label. 0 = uncomputed; layout() falls back to a width ratio.
    float axis_width_px = 0.f;

    // --- price bounds (y-axis state) ---------------------------------------
    // Persistent across pans (panning preserves the price scale). Mutated
    // only by pinch zoom, axis drags, vertical drag-translate.
    vroom::PriceBounds price_bounds{0.0, 1.0};
    bool price_bounds_initialized = false;

    // --- crosshair (not yet drawn) -----------------------------------------
    bool   crosshair_active = false;
    float  crosshair_x_px = 0.f;
    float  crosshair_y_px = 0.f;

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

    // The main drawing pass. Calls into the labels and candles modules.
    void draw_chart(SkCanvas* canvas);

    // Re-records `chart_picture` by invoking draw_chart on a fresh
    // SkPictureRecorder. Also computes `last_dt_seconds` for fade animation.
    void rebuild_chart_picture();

    // True if any axis label is mid-fade. Used by the JS-side animation loop
    // to know when to keep ticking.
    bool is_animating_now() const;
};
