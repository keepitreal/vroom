// VroomChart engine — orchestration only.
//
// The struct definition lives in chart.h; per-subsystem rendering and the
// public C facade live in candles.cpp, labels.cpp, and chart_facade.cpp.
// This file keeps just the constructor, the small helpers used everywhere
// (mark_dirty, layout, is_animating_now), the picture cache lifecycle
// (rebuild_chart_picture), and the top-level draw_chart that composes all
// the layers in z-order.

#include "chart.h"

#include <cmath>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRect.h"
#pragma clang diagnostic pop

#include "bollinger.h"
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
#include "price_lines.h"
#include "rsi.h"
#include "rsi_pane.h"
#include "style_inherit.h"
#include "tip_anchor.h"
#include "tip_pulse.h"
#include "volume.h"
#include "vwap.h"
#include "theme.h"
#include "viewport.h"

namespace {
constexpr SkColor kVwapLine = 0xff00bcd4;  // cyan
}  // namespace

VroomChart::VroomChart() : theme(vroom::default_theme()) {}

void VroomChart::mark_dirty() {
    chart_dirty = true;
    if (cb.on_redraw_requested) cb.on_redraw_requested(user_ctx);
}

vroom::Layout VroomChart::layout() const {
    const float axis_w = axis_width_px > 0.f
        ? axis_width_px
        : width_px * theme.floats[VROOM_FLOAT_Y_AXIS_WIDTH_RATIO];
    const int pane_count = (rsi.enabled ? 1 : 0) + (macd.enabled ? 1 : 0);
    const float indicator_h = static_cast<float>(pane_count) * height_px *
                              theme.floats[VROOM_FLOAT_INDICATOR_HEIGHT_FRAC];
    return vroom::Layout{
        width_px,
        height_px,
        vroom::axis_extent(axis_w, y_axis_collapse_t),
        vroom::axis_extent(theme.floats[VROOM_FLOAT_X_AXIS_HEIGHT_PX],
                           x_axis_collapse_t),
        theme.floats[VROOM_FLOAT_RIGHT_PADDING_PX],
        theme.floats[VROOM_FLOAT_CANDLE_WIDTH_RATIO],
        0.05f,
        0.05f,
        indicator_h,
        vroom::axis_opacity(y_axis_collapse_t),
        vroom::axis_opacity(x_axis_collapse_t),
    };
}

void VroomChart::ensure_rsi() {
    if (!rsi.enabled || !rsi_dirty) return;
    vroom::rsi::compute(candles.data(), candles.size(), rsi.period, rsi_cache);
    if (rsi.ma_visible) {
        vroom::rsi::compute_ma(rsi_cache, rsi.ma_period, rsi.ma_kind,
                               rsi_ma_cache);
    } else {
        rsi_ma_cache.clear();
    }
    rsi_dirty = false;
}

void VroomChart::ensure_macd() {
    if (!macd.enabled || !macd_dirty) return;
    vroom::macd::compute(candles.data(), candles.size(), macd.fast, macd.slow,
                         macd.signal, macd.source, macd.ma_kind,
                         macd.signal_ma_kind, macd_cache, macd_signal_cache,
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
    if (!vwap.enabled || !vwap_dirty) return;
    vroom::vwap::compute(candles.data(), candles.size(), vwap.reset_offset_min,
                         vwap_cache, vwap_breaks);
    vwap_dirty = false;
}

void VroomChart::ensure_bollinger() {
    if (!bollinger.enabled || !bollinger_dirty) return;
    vroom::bollinger::compute(candles.data(), candles.size(), bollinger.period,
                              bollinger.mult, bollinger.source,
                              bollinger.basis_kind, bb_middle_cache,
                              bb_upper_cache, bb_lower_cache);
    bollinger_dirty = false;
}

void VroomChart::draw_chart(SkCanvas* canvas) {
    begin_frame();
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
    //    While an interval morph fades the old ticks out, the axes are still laid
    //    out against the pre-switch scale and window so nothing appears to move;
    //    they adopt the new ones at the midpoint, invisible. Every axis call in
    //    the frame has to agree on which of the two it's using.
    const auto axis_phase = vroom::labels::interval_phase(*this);
    const auto& axis_bounds = axis_phase.outgoing ? morph_from_bounds : bounds;
    const int64_t axis_start_ms =
        axis_phase.outgoing ? morph_from_start_ms : visible_start_ms;
    const int64_t axis_end_ms =
        axis_phase.outgoing ? morph_from_end_ms : visible_end_ms;
    vroom::labels::update_y_fades(*this, lay, axis_bounds);
    vroom::labels::update_x_fades(*this, lay, axis_start_ms, axis_end_ms);

    // 4. Gridlines — drawn before candles so candle bodies overlay them.
    //    Vertical (time) gridlines are intentionally disabled for now —
    //    re-enable by adding `vroom::labels::draw_x_gridlines(canvas, *this,
    //    axis_start_ms, axis_end_ms, candle_right, candle_area_h);` here.
    vroom::labels::draw_y_gridlines(canvas, *this, lay, axis_bounds,
                                    candle_right, candle_area_h);

    // 4.35. Price-series morph state, shared by the gradient fill below and the
    //       series itself (5). `morph_fade` crossfades candles→line and
    //       `morph_collapse` folds each candle toward its close (the line vertex);
    //       fade 0 = pure candles, fade 1 = pure line.
    //
    //       An interval *transform* reshapes each slot from the geometry it held
    //       before the timeframe switch (see `morph_from`). A host-chosen
    //       *fade* skips the pairing and uses the same envelope as the axes
    //       (`labels::interval_phase`): outgoing snapshot out, then new scene in.
    const float fade = morph_fade;
    const float collapse = morph_collapse;
    const bool morphing = interval_morph_t < 1.f && !morph_from.empty();
    const std::size_t morph_n = morphing ? morph_from.size() : 0;
    const bool fade_swap = morphing && interval_morph_fade;
    const bool fade_out = fade_swap && axis_phase.outgoing;
    const bool fade_in = fade_swap && !axis_phase.outgoing;
    const bool reshape = morphing && !fade_swap;
    const vroom::CandleSnapshot* morph_src = reshape ? morph_from.data() : nullptr;
    const std::size_t morph_n_draw = reshape ? morph_n : 0;
    const float morph_t = reshape ? interval_morph_t : 1.f;

    if (fade_out) {
        // First half: only the captured series, at the axis envelope opacity.
        // Layers with no snapshot (volume, overlays, indicators, drawings)
        // would already be the new data, so they stay hidden until the incoming
        // half. morph_t = 0 draws the capture pixel-identically.
        const float layer = axis_phase.opacity;
        if (layer > 0.f) {
            if (fade > 0.f) {
                vroom::ma_overlay::draw_close_gradient(
                    canvas, lay, bounds, visible, 0, window_ms,
                    visible_start_ms, candle_duration_ms, candle_right,
                    candle_area_h, theme.colors[VROOM_COLOR_LINE],
                    theme.floats[VROOM_FLOAT_LINE_GRADIENT_OPACITY],
                    fade * layer, morph_from.data(), morph_n, 0.f,
                    theme.floats[VROOM_FLOAT_LINE_TENSION]);
            }
            if (fade < 1.f) {
                vroom::candles::draw(canvas, visible, 0, lay, theme, bounds,
                                     window_ms, visible_start_ms,
                                     candle_duration_ms, collapse,
                                     (1.f - fade) * layer, morph_from.data(),
                                     morph_n, 0.f);
            }
            if (fade > 0.f) {
                vroom::ma_overlay::draw_close_line(
                    canvas, lay, bounds, visible, 0, window_ms,
                    visible_start_ms, candle_duration_ms, candle_right,
                    candle_area_h, theme.colors[VROOM_COLOR_LINE],
                    theme.floats[VROOM_FLOAT_LINE_WIDTH_PX], fade * layer,
                    morph_from.data(), morph_n, 0.f,
                    theme.floats[VROOM_FLOAT_LINE_TENSION]);
                if (theme.floats[VROOM_FLOAT_LINE_TIP_DOT] > 0.5f) {
                    vroom::ma_overlay::draw_close_tip(
                        canvas, lay, bounds, visible, 0, window_ms,
                        visible_start_ms, candle_duration_ms, candle_right,
                        candle_area_h, theme.colors[VROOM_COLOR_LINE],
                        theme.colors[VROOM_COLOR_BACKGROUND],
                        theme.floats[VROOM_FLOAT_LINE_WIDTH_PX], fade * layer,
                        theme.floats[VROOM_FLOAT_LINE_TIP_PULSE] > 0.5f,
                        tip_pulse_elapsed_s / vroom::tip_pulse::kPeriodSeconds,
                        morph_from.data(), morph_n, 0.f);
                }
            }
        }
    } else {
        // Incoming fade: the new scene at envelope opacity, no slot lerp
        // (`morph_t` is already 1). Transforms use the reshape path above.
        const bool wrap = fade_in && axis_phase.opacity > 0.f &&
                          axis_phase.opacity < 0.999f;
        if (wrap) {
            canvas->saveLayerAlpha(
                nullptr, static_cast<U8CPU>(axis_phase.opacity * 255.f + 0.5f));
        }
        const bool draw_new = !fade_in || axis_phase.opacity > 0.f;

        if (draw_new) {
            // 4.4. Line-mode gradient fill — the wash under the close polyline.
            if (fade > 0.f) {
                vroom::ma_overlay::draw_close_gradient(
                    canvas, lay, bounds, visible, n, window_ms,
                    visible_start_ms, candle_duration_ms, candle_right,
                    candle_area_h, theme.colors[VROOM_COLOR_LINE],
                    theme.floats[VROOM_FLOAT_LINE_GRADIENT_OPACITY], fade,
                    morph_src, morph_n_draw, morph_t,
                    theme.floats[VROOM_FLOAT_LINE_TENSION]);
            }

            // 4.5. Volume bars — drawn under the candles so candles z-index above.
            if (volume_collapse_t < 1.f) {
                vroom::volume::draw(canvas, visible, n, lay, theme, volume,
                                    volume_collapse_t, volume_collapse_easing,
                                    window_ms, visible_start_ms,
                                    candle_duration_ms);
            }

            // 4.6. Liquidity bands (resting-order depth).
            vroom::liquidity::draw(canvas, *this, lay, bounds, candle_right,
                                   candle_area_h);

            // 4.7. Bollinger Band fill.
            if (bollinger.enabled) {
                ensure_bollinger();
                if (bollinger.fill_enabled &&
                    bb_upper_cache.size() == candles.size() &&
                    bb_lower_cache.size() == candles.size()) {
                    vroom::ma_overlay::fill_between(
                        canvas, lay, bounds, visible, n,
                        bb_upper_cache.data() + range.start,
                        bb_lower_cache.data() + range.start, window_ms,
                        visible_start_ms, candle_duration_ms, candle_right,
                        candle_area_h, bollinger.upper_color,
                        bollinger.fill_opacity);
                }
            }

            // 5. Price series — candles, a close-price line, or a blend.
            if (fade < 1.f) {
                vroom::candles::draw(canvas, visible, n, lay, theme, bounds,
                                     window_ms, visible_start_ms,
                                     candle_duration_ms, collapse, 1.f - fade,
                                     morph_src, morph_n_draw, morph_t);
            }
            if (fade > 0.f) {
                vroom::ma_overlay::draw_close_line(
                    canvas, lay, bounds, visible, n, window_ms,
                    visible_start_ms, candle_duration_ms, candle_right,
                    candle_area_h, theme.colors[VROOM_COLOR_LINE],
                    theme.floats[VROOM_FLOAT_LINE_WIDTH_PX], fade, morph_src,
                    morph_n_draw, morph_t,
                    theme.floats[VROOM_FLOAT_LINE_TENSION]);
            }

            // 5.5. Moving-average overlay lines.
            if (!overlays.empty()) {
                ensure_overlays();
                for (std::size_t k = 0; k < overlays.size(); ++k) {
                    if (overlay_caches[k].size() != candles.size()) continue;
                    const double* vis = overlay_caches[k].data() + range.start;
                    vroom::ma_overlay::draw(
                        canvas, lay, bounds, visible, n, vis, window_ms,
                        visible_start_ms, candle_duration_ms, candle_right,
                        candle_area_h, overlays[k].color, overlays[k].width);
                }
            }

            // 5.6. VWAP overlay.
            if (vwap.enabled) {
                ensure_vwap();
                if (vwap_cache.size() == candles.size()) {
                    const double* vis = vwap_cache.data() + range.start;
                    const unsigned char* brk =
                        vwap_breaks.size() == candles.size()
                            ? vwap_breaks.data() + range.start
                            : nullptr;
                    vroom::ma_overlay::draw(
                        canvas, lay, bounds, visible, n, vis, window_ms,
                        visible_start_ms, candle_duration_ms, candle_right,
                        candle_area_h,
                        vroom::style::color_or(vwap.color, kVwapLine),
                        vroom::style::width_or(vwap.width, 1.5f), brk);
                }
            }

            // 5.65. Bollinger Band lines.
            if (bollinger.enabled) {
                ensure_bollinger();
                const std::size_t sz = candles.size();
                if (bb_upper_cache.size() == sz && bb_lower_cache.size() == sz &&
                    bb_middle_cache.size() == sz) {
                    const auto stroke = [&](const std::vector<double>& cache,
                                              uint32_t color, float width) {
                        vroom::ma_overlay::draw(
                            canvas, lay, bounds, visible, n,
                            cache.data() + range.start, window_ms,
                            visible_start_ms, candle_duration_ms, candle_right,
                            candle_area_h, color, width);
                    };
                    stroke(bb_upper_cache, bollinger.upper_color,
                           bollinger.upper_width);
                    stroke(bb_lower_cache, bollinger.lower_color,
                           bollinger.lower_width);
                    stroke(bb_middle_cache, bollinger.middle_color,
                           bollinger.middle_width);
                }
            }

            // 5.7. Drawing annotations.
            vroom::drawings::draw(canvas, *this, lay, bounds, candle_right,
                                  candle_area_h);

            // 5.8. Line-mode tip marker.
            if (fade > 0.f && theme.floats[VROOM_FLOAT_LINE_TIP_DOT] > 0.5f) {
                const auto anchor = vroom::tip_anchor::at(
                    range.start, range.end, candles.size(), reshape);
                vroom::ma_overlay::draw_close_tip(
                    canvas, lay, bounds, visible, anchor.slot_count, window_ms,
                    visible_start_ms, candle_duration_ms, candle_right,
                    candle_area_h, theme.colors[VROOM_COLOR_LINE],
                    theme.colors[VROOM_COLOR_BACKGROUND],
                    theme.floats[VROOM_FLOAT_LINE_WIDTH_PX], fade,
                    theme.floats[VROOM_FLOAT_LINE_TIP_PULSE] > 0.5f,
                    tip_pulse_elapsed_s / vroom::tip_pulse::kPeriodSeconds,
                    anchor.use_morph ? morph_src : nullptr,
                    anchor.use_morph ? morph_n_draw : 0, morph_t);
            }

            // Incoming fade: indicator panes share the scene opacity (and
            // paint before axis masks so overflow still clips). Transforms
            // keep the settled z-order after the price indicator.
            if (fade_in && lay.indicator_area_h > 0.f) {
                struct ActivePane {
                    int order;
                    int type;
                };
                ActivePane panes[2];
                int count = 0;
                if (rsi.enabled) panes[count++] = {rsi_order, 0};
                if (macd.enabled) panes[count++] = {macd_order, 1};
                if (count == 2 && panes[0].order > panes[1].order) {
                    const ActivePane tmp = panes[0];
                    panes[0] = panes[1];
                    panes[1] = tmp;
                }

                const float pane_h =
                    height_px * theme.floats[VROOM_FLOAT_INDICATOR_HEIGHT_FRAC];
                float pane_top = candle_area_h;
                for (int i = 0; i < count; ++i) {
                    const float pane_bottom = pane_top + pane_h;
                    if (panes[i].type == 0) {
                        ensure_rsi();
                        const double* rsi_vis =
                            rsi_cache.size() == candles.size()
                                ? rsi_cache.data() + range.start
                                : nullptr;
                        const double* rsi_ma_vis =
                            (rsi.ma_visible &&
                             rsi_ma_cache.size() == candles.size())
                                ? rsi_ma_cache.data() + range.start
                                : nullptr;
                        vroom::rsi_pane::draw(
                            canvas, *this, lay, visible, n, rsi_vis, rsi_ma_vis,
                            window_ms, visible_start_ms, candle_duration_ms,
                            candle_right, pane_top, pane_bottom);
                    } else {
                        ensure_macd();
                        const double* macd_vis =
                            macd_cache.size() == candles.size()
                                ? macd_cache.data() + range.start
                                : nullptr;
                        const double* sig_vis =
                            macd_signal_cache.size() == candles.size()
                                ? macd_signal_cache.data() + range.start
                                : nullptr;
                        const double* hist_vis =
                            macd_hist_cache.size() == candles.size()
                                ? macd_hist_cache.data() + range.start
                                : nullptr;
                        vroom::macd_pane::draw(
                            canvas, *this, lay, visible, n, macd_vis, sig_vis,
                            hist_vis, window_ms, visible_start_ms,
                            candle_duration_ms, candle_right, pane_top,
                            pane_bottom);
                    }
                    pane_top = pane_bottom;
                }
            }
        }

        if (wrap) canvas->restore();
    }

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
    vroom::labels::draw_y_labels(canvas, *this, lay, axis_bounds);
    vroom::labels::draw_x_labels(canvas, *this, lay, axis_start_ms, axis_end_ms);

    // 7.5. Current-price line + box — above labels so the box covers any label
    //      it overlaps; tracks the latest close as the price scale moves.
    //      Hidden during a fade's outgoing half: the close is already the new
    //      series and there is no snapshot to fade.
    if (!fade_out) {
        vroom::price_indicator::draw(canvas, *this, lay, bounds,
                                     candle_right, candle_area_h);

        // 7.55. Consumer-supplied price status lines — same tier as the
        //       current-price indicator (their badges must cover the labels
        //       underneath), but after it so a resting order at the last close
        //       stays readable.
        vroom::price_lines::draw(canvas, *this, lay, bounds, candle_right,
                                   candle_area_h);
    }

    // 7.6. Indicator panes stacked below the candles, ordered by enable
    //      sequence (most recently enabled at the bottom). Each pane is
    //      INDICATOR_HEIGHT_FRAC of the height; the candle pane already shrank
    //      to fit them (see layout()). A fade already drew them with the scene.
    if (!fade_swap && lay.indicator_area_h > 0.f) {
        struct ActivePane { int order; int type; };  // type: 0 = RSI, 1 = MACD
        ActivePane panes[2];
        int count = 0;
        if (rsi.enabled) panes[count++] = {rsi_order, 0};
        if (macd.enabled) panes[count++] = {macd_order, 1};
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
                    (rsi.ma_visible && rsi_ma_cache.size() == candles.size())
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

void VroomChart::begin_frame() {
    // dt for the fade animations. A large gap (resume from background, or an idle
    // chart that a click just woke up) is clamped to a nominal frame rather than
    // zeroed: dt == 0 means "snap" to the fade updaters, which would finish every
    // fade in the first frame after any idle period.
    constexpr float kNominalFrameSeconds = 1.f / 60.f;
    const auto now = std::chrono::steady_clock::now();
    float dt = 0.f;
    if (anim_started) {
        dt = std::chrono::duration<float>(now - last_anim_tick).count();
        if (dt > 0.1f) dt = kNominalFrameSeconds;
    }
    last_anim_tick = now;
    anim_started = true;
    last_dt_seconds = dt;

    // Wrapped rather than accumulated: the phase is the only thing anyone reads,
    // and a chart left open for hours would otherwise lose float precision on it.
    tip_pulse_elapsed_s =
        std::fmod(tip_pulse_elapsed_s + dt, vroom::tip_pulse::kPeriodSeconds);
}

bool VroomChart::tip_pulse_active() const {
    return morph_fade > 0.f && !candles.empty() &&
           theme.floats[VROOM_FLOAT_LINE_TIP_DOT] > 0.5f &&
           theme.floats[VROOM_FLOAT_LINE_TIP_PULSE] > 0.5f;
}

void VroomChart::rebuild_chart_picture() {
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(SkRect::MakeWH(width_px, height_px));
    draw_chart(canvas);
    chart_picture = recorder.finishRecordingAsPicture();
    chart_dirty = false;
}

bool VroomChart::is_animating_now() const {
    // The pulse never finishes on its own, so this is what keeps the host loops
    // requeueing frames for it.
    if (tip_pulse_active()) return true;
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
