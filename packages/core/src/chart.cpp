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
#include "crosshair.h"
#include "drawings.h"
#include "labels.h"
#include "liquidity.h"
#include "ma.h"
#include "ma_overlay.h"
#include "macd.h"
#include "macd_pane.h"
#include "price_indicator.h"
#include "rsi.h"
#include "rsi_pane.h"
#include "volume.h"
#include "vwap.h"
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
    const int pane_count = (rsi_enabled ? 1 : 0) + (macd_enabled ? 1 : 0);
    const float indicator_h = static_cast<float>(pane_count) * height_px *
                              theme.floats[VROOM_FLOAT_INDICATOR_HEIGHT_FRAC];
    return vroom::Layout{
        width_px,
        height_px,
        axis_w,
        theme.floats[VROOM_FLOAT_X_AXIS_HEIGHT_PX],
        theme.floats[VROOM_FLOAT_RIGHT_PADDING_PX],
        theme.floats[VROOM_FLOAT_CANDLE_WIDTH_RATIO],
        0.05f,
        0.05f,
        indicator_h,
    };
}

void VroomChart::ensure_rsi() {
    if (!rsi_enabled || !rsi_dirty) return;
    vroom::rsi::compute(candles.data(), candles.size(), rsi_period, rsi_cache);
    if (rsi_ma_enabled) {
        vroom::rsi::compute_ma(rsi_cache, rsi_ma_period, rsi_ma_cache);
    } else {
        rsi_ma_cache.clear();
    }
    rsi_dirty = false;
}

void VroomChart::ensure_macd() {
    if (!macd_enabled || !macd_dirty) return;
    vroom::macd::compute(candles.data(), candles.size(), macd_fast, macd_slow,
                         macd_signal, macd_cache, macd_signal_cache,
                         macd_hist_cache);
    macd_dirty = false;
}

void VroomChart::ensure_overlays() {
    if (!overlays_dirty) return;
    overlay_caches.resize(overlays.size());
    for (std::size_t i = 0; i < overlays.size(); ++i) {
        const auto& ov = overlays[i];
        vroom::ma::compute(candles.data(), candles.size(), ov.kind, ov.period,
                           ov.source, overlay_caches[i]);
    }
    overlays_dirty = false;
}

void VroomChart::ensure_vwap() {
    if (!vwap_enabled || !vwap_dirty) return;
    vroom::vwap::compute(candles.data(), candles.size(), vwap_reset_offset_min,
                         vwap_cache, vwap_breaks);
    vwap_dirty = false;
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

    const auto bounds = price_bounds_manual
        ? price_bounds
        : vroom::auto_price_bounds(visible, n);
    const int64_t window_ms = visible_end_ms - visible_start_ms;

    const float candle_area_h = vroom::price_pane_bottom(lay);
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

    // 4.5. Volume bars — drawn under the candles so candles z-index above.
    vroom::volume::draw(canvas, visible, n, lay, theme,
                        window_ms, visible_start_ms, candle_duration_ms);

    // 4.6. Liquidity bands (resting-order depth) — behind the candles so the
    //      candle bodies paint over the volume-driven tint. Anchored in price
    //      space, so they scale with the y-axis.
    vroom::liquidity::draw(canvas, *this, lay, bounds, candle_right,
                           candle_area_h);

    // 5. Candles (wicks + bodies)
    vroom::candles::draw(canvas, visible, n, lay, theme, bounds,
                         window_ms, visible_start_ms, candle_duration_ms);

    // 5.5. Moving-average overlay lines (SMA/EMA) on the price pane, over the
    //      candles. They share the candle price scale and don't reserve a pane.
    if (!overlays.empty()) {
        ensure_overlays();
        for (std::size_t k = 0; k < overlays.size(); ++k) {
            if (overlay_caches[k].size() != candles.size()) continue;
            const double* vis = overlay_caches[k].data() + range.start;
            vroom::ma_overlay::draw(canvas, lay, bounds, visible, n, vis,
                                    window_ms, visible_start_ms,
                                    candle_duration_ms, candle_right,
                                    candle_area_h, overlays[k].color,
                                    overlays[k].width);
        }
    }

    // 5.6. VWAP overlay (session, configurable reset) — a single price-pane line
    //      that breaks at each session reset (vwap_breaks).
    if (vwap_enabled) {
        ensure_vwap();
        if (vwap_cache.size() == candles.size()) {
            const double* vis = vwap_cache.data() + range.start;
            const unsigned char* brk = vwap_breaks.size() == candles.size()
                ? vwap_breaks.data() + range.start
                : nullptr;
            vroom::ma_overlay::draw(canvas, lay, bounds, visible, n, vis,
                                    window_ms, visible_start_ms,
                                    candle_duration_ms, candle_right,
                                    candle_area_h, vwap_color, vwap_width, brk);
        }
    }

    // 5.7. Drawing annotations (committed line tools + the in-progress draft).
    //      On the price pane above the candles/overlays, below the axis labels.
    vroom::drawings::draw(canvas, *this, lay, bounds, candle_right,
                          candle_area_h);

    // 6. Axis backgrounds (mask any candle overflow). The x-axis separator
    //    line is intentionally omitted for now. The bottom strip anchors at
    //    x_axis_top (below any indicator pane) so it never paints over it.
    SkPaint axis_bg;
    axis_bg.setColor(theme.colors[VROOM_COLOR_BACKGROUND]);
    canvas->drawRect(
        SkRect::MakeXYWH(0, vroom::x_axis_top(lay), candle_right,
                         lay.x_axis_height_px),
        axis_bg);
    const float axis_block_w = width_px - candle_right;
    canvas->drawRect(
        SkRect::MakeXYWH(candle_right, 0, axis_block_w, height_px),
        axis_bg);

    // 7. Labels (read from y_fades / x_fades, no state mutation here)
    vroom::labels::draw_y_labels(canvas, *this, lay, bounds);
    vroom::labels::draw_x_labels(canvas, *this, lay);

    // 7.5. Current-price line + box — above labels so the box covers any label
    //      it overlaps; tracks the latest close as the price scale moves.
    vroom::price_indicator::draw(canvas, *this, lay, bounds,
                                 candle_right, candle_area_h);

    // 7.6. Indicator panes stacked below the candles, ordered by enable
    //      sequence (most recently enabled at the bottom). Each pane is
    //      INDICATOR_HEIGHT_FRAC of the height; the candle pane already shrank
    //      to fit them (see layout()).
    if (lay.indicator_area_h > 0.f) {
        struct ActivePane { int order; int type; };  // type: 0 = RSI, 1 = MACD
        ActivePane panes[2];
        int count = 0;
        if (rsi_enabled) panes[count++] = {rsi_order, 0};
        if (macd_enabled) panes[count++] = {macd_order, 1};
        if (count == 2 && panes[0].order > panes[1].order) {
            const ActivePane tmp = panes[0];
            panes[0] = panes[1];
            panes[1] = tmp;
        }

        const float pane_h =
            height_px * theme.floats[VROOM_FLOAT_INDICATOR_HEIGHT_FRAC];
        float pane_top = candle_area_h;  // == price_pane_bottom(lay)
        for (int i = 0; i < count; ++i) {
            const float pane_bottom = pane_top + pane_h;
            if (panes[i].type == 0) {
                ensure_rsi();
                const double* rsi_vis = rsi_cache.size() == candles.size()
                    ? rsi_cache.data() + range.start : nullptr;
                const double* rsi_ma_vis =
                    (rsi_ma_enabled && rsi_ma_cache.size() == candles.size())
                        ? rsi_ma_cache.data() + range.start : nullptr;
                vroom::rsi_pane::draw(canvas, *this, lay, visible, n, rsi_vis,
                                      rsi_ma_vis, window_ms, visible_start_ms,
                                      candle_duration_ms, candle_right, pane_top,
                                      pane_bottom);
            } else {
                ensure_macd();
                const double* macd_vis = macd_cache.size() == candles.size()
                    ? macd_cache.data() + range.start : nullptr;
                const double* sig_vis =
                    macd_signal_cache.size() == candles.size()
                        ? macd_signal_cache.data() + range.start : nullptr;
                const double* hist_vis =
                    macd_hist_cache.size() == candles.size()
                        ? macd_hist_cache.data() + range.start : nullptr;
                vroom::macd_pane::draw(canvas, *this, lay, visible, n, macd_vis,
                                       sig_vis, hist_vis, window_ms,
                                       visible_start_ms, candle_duration_ms,
                                       candle_right, pane_top, pane_bottom);
            }
            pane_top = pane_bottom;
        }
    }

    // 7.7. Crosshair — drawn last so it sits on top of everything, including the
    //      indicator panes. The vertical line runs down to x_axis_top so it
    //      stays visible across the candle area and all below-chart panes; the
    //      horizontal line + ring stay in the price pane (clamped to
    //      candle_area_h).
    if (crosshair_active) {
        // Snap once: the slot gives both the vertical line's x (its center) and
        // the time shown in the date badge. (snap_x_to_candle does exactly this
        // internally, so the line position is unchanged.)
        const vroom::SnapResult snap = vroom::snap_to_slot(
            lay, visible, n, candle_duration_ms, visible_start_ms, window_ms,
            crosshair_x_px);
        const float snap_x = vroom::candle_center_x(
            lay, snap.time_ms, candle_duration_ms, visible_start_ms, window_ms);
        vroom::crosshair::draw(canvas, *this, lay, bounds, candle_right,
                               candle_area_h, vroom::x_axis_top(lay), snap_x,
                               snap.time_ms);
    }

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
