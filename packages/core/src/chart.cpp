// VroomChart engine — orchestration only.
//
// The struct definition lives in chart.h; per-subsystem rendering and the
// public C facade live in candles.cpp, labels.cpp, and chart_facade.cpp.
// This file keeps just the constructor, the small helpers used everywhere
// (mark_dirty, layout, is_animating_now), the picture cache lifecycle
// (rebuild_chart_picture), and the top-level draw_chart that composes all
// the layers in z-order.

#include "chart.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRect.h"
#pragma clang diagnostic pop

#include "candles.h"
#include "chart_internal.h"
#include "labels.h"
#include "theme.h"
#include "viewport.h"

VroomChart::VroomChart() : theme(vroom::default_theme()) {}

void VroomChart::mark_dirty() {
    chart_dirty = true;
    if (cb.on_redraw_requested) cb.on_redraw_requested(user_ctx);
}

vroom::Layout VroomChart::layout() const {
    const float axis_w = axis_width_px > 0.f
        ? axis_width_px
        : width_px * theme.floats[VROOM_FLOAT_Y_AXIS_WIDTH_RATIO];
    return vroom::Layout{
        width_px,
        height_px,
        axis_w,
        theme.floats[VROOM_FLOAT_X_AXIS_HEIGHT_PX],
        theme.floats[VROOM_FLOAT_RIGHT_PADDING_PX],
        theme.floats[VROOM_FLOAT_CANDLE_WIDTH_RATIO],
        0.05f,
        0.05f,
    };
}

void VroomChart::draw_chart(SkCanvas* canvas) {
    const auto lay = layout();

    // 1. Background
    SkPaint bg;
    bg.setColor(theme.colors[VROOM_COLOR_BACKGROUND]);
    canvas->drawRect(SkRect::MakeWH(width_px, height_px), bg);

    if (candles.empty()) return;

    // 2. Visible slice + bounds + window_ms + geometry
    const auto range = vroom::visible_indices(
        candles.data(), candles.size(),
        visible_start_ms, visible_end_ms);
    const size_t n = range.end - range.start;
    if (n == 0) return;
    const ::VroomCandle* visible = candles.data() + range.start;

    const auto bounds = price_bounds_initialized
        ? price_bounds
        : vroom::price_bounds(visible, n);
    const int64_t window_ms = visible_end_ms - visible_start_ms;

    const float candle_area_h = height_px - lay.x_axis_height_px;
    const float candle_right =
        width_px - lay.y_axis_width_px - lay.right_padding_px;

    // 3. Update label fade state ONCE per frame — both gridlines and labels
    //    share these opacities so their animations stay in lockstep.
    vroom::labels::update_y_fades(*this, lay, bounds);
    vroom::labels::update_x_fades(*this, lay);

    // 4. Gridlines — drawn before candles so candle bodies overlay them.
    //    Vertical (time) gridlines are intentionally disabled for now —
    //    re-enable by adding `vroom::labels::draw_x_gridlines(canvas, *this,
    //    candle_right, candle_area_h);` here.
    vroom::labels::draw_y_gridlines(canvas, *this, lay, bounds,
                                    candle_right, candle_area_h);

    // 5. Candles (wicks + bodies)
    vroom::candles::draw(canvas, visible, n, lay, theme, bounds,
                         window_ms, visible_start_ms, candle_duration_ms);

    // 6. Axis backgrounds (mask any candle overflow) + separator lines
    SkPaint axis_bg;
    axis_bg.setColor(theme.colors[VROOM_COLOR_BACKGROUND]);
    canvas->drawRect(
        SkRect::MakeXYWH(0, candle_area_h, candle_right,
                         lay.x_axis_height_px),
        axis_bg);
    const float axis_block_w = width_px - candle_right;
    canvas->drawRect(
        SkRect::MakeXYWH(candle_right, 0, axis_block_w, height_px),
        axis_bg);

    SkPaint border;
    border.setColor(theme.colors[VROOM_COLOR_GRID]);
    border.setStrokeWidth(1.f);
    border.setAntiAlias(true);
    canvas->drawLine(candle_right, 0, candle_right, candle_area_h, border);
    canvas->drawLine(0, candle_area_h, candle_right, candle_area_h, border);

    // 7. Labels (read from y_fades / x_fades, no state mutation here)
    vroom::labels::draw_y_labels(canvas, *this, lay, bounds);
    vroom::labels::draw_x_labels(canvas, *this, lay);

    // 8. GC fades that have fully faded out and aren't coming back.
    vroom::labels::gc_y_fades(*this);
    vroom::labels::gc_x_fades(*this);
}

void VroomChart::rebuild_chart_picture() {
    // Compute dt for fade animations. We cap large gaps (resume from
    // background, long idle) so we don't snap animations to completion.
    auto now = std::chrono::steady_clock::now();
    float dt = 0.f;
    if (anim_started) {
        dt = std::chrono::duration<float>(now - last_anim_tick).count();
        if (dt > 0.1f) dt = 0.f;
    }
    last_anim_tick = now;
    anim_started = true;
    last_dt_seconds = dt;

    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(SkRect::MakeWH(width_px, height_px));
    draw_chart(canvas);
    chart_picture = recorder.finishRecordingAsPicture();
    chart_dirty = false;
}

bool VroomChart::is_animating_now() const {
    for (const auto& f : y_fades) {
        if (f.opacity != f.target) return true;
    }
    for (const auto& f : x_fades) {
        if (f.opacity != f.target) return true;
    }
    return false;
}

// chart_internal.h exports — bridge layer (JSI HostObject) consumers.

namespace vroom {

sk_sp<SkPicture> render_chart_picture(VroomChart* chart) {
    if (!chart) return nullptr;
    if (chart->chart_dirty || !chart->chart_picture || chart->is_animating_now()) {
        chart->rebuild_chart_picture();
    }
    return chart->chart_picture;
}

bool is_animating(VroomChart* chart) {
    return chart ? chart->is_animating_now() : false;
}

}  // namespace vroom
