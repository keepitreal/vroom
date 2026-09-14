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

#include "atr.h"
#include "atr_pane.h"
#include "bollinger.h"
#include "candles.h"
#include "chart_internal.h"
#include "crosshair.h"
#include "drawings.h"
#include "footprints.h"
#include "fvg_overlay.h"
#include "ichimoku.h"
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
    const int pane_count =
        (rsi.enabled ? 1 : 0) + (macd.enabled ? 1 : 0) + (atr.enabled ? 1 : 0);
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

void VroomChart::ensure_atr() {
    if (!atr.enabled || !atr_dirty) return;
    vroom::atr::compute(candles.data(), candles.size(), atr.period,
                        atr.smoothing, atr_cache);
    atr_dirty = false;
}

int VroomChart::indicator_panes(vroom::IndicatorPane out[vroom::kMaxPanes]) const {
    int count = 0;
    if (rsi.enabled) out[count++] = {rsi_order, vroom::PaneKind::Rsi};
    if (macd.enabled) out[count++] = {macd_order, vroom::PaneKind::Macd};
    if (atr.enabled) out[count++] = {atr_order, vroom::PaneKind::Atr};
    // Insertion sort: at most three entries, already nearly ordered.
    for (int i = 1; i < count; ++i) {
        const vroom::IndicatorPane key = out[i];
        int j = i - 1;
        for (; j >= 0 && out[j].order > key.order; --j) out[j + 1] = out[j];
        out[j + 1] = key;
    }
    return count;
}

void VroomChart::draw_indicator_panes(SkCanvas* canvas,
                                      const vroom::Layout& lay,
                                      const VroomCandle* visible,
                                      std::size_t n, std::size_t first,
                                      int64_t window_ms, float candle_right,
                                      float pane_top, bool use_capture,
                                      float morph_t) {
    vroom::IndicatorPane panes[vroom::kMaxPanes];
    const int count = indicator_panes(panes);
    const float pane_h =
        height_px * theme.floats[VROOM_FLOAT_INDICATOR_HEIGHT_FRAC];

    // A cache only lines up with the candles once its ensure_* has run against
    // the current series; until then there is nothing to plot.
    const auto visible_slice = [&](const std::vector<double>& cache) -> const double* {
        return cache.size() == candles.size() ? cache.data() + first : nullptr;
    };
    const auto capture = [&](vroom::LineKey key) -> const vroom::LineMorph* {
        return use_capture ? vroom::find_line_morph(morph_lines, key) : nullptr;
    };

    for (int i = 0; i < count; ++i) {
        const float pane_bottom = pane_top + pane_h;
        switch (panes[i].kind) {
            case vroom::PaneKind::Rsi: {
                ensure_rsi();
                vroom::rsi_pane::draw(
                    canvas, *this, lay, visible, n, visible_slice(rsi_cache),
                    rsi.ma_visible ? visible_slice(rsi_ma_cache) : nullptr,
                    window_ms, visible_start_ms, candle_duration_ms,
                    candle_right, pane_top, pane_bottom,
                    capture({vroom::LineKind::Rsi}),
                    rsi.ma_visible ? capture({vroom::LineKind::RsiMa}) : nullptr,
                    morph_t);
                break;
            }
            case vroom::PaneKind::Macd: {
                ensure_macd();
                vroom::macd_pane::draw(
                    canvas, *this, lay, visible, n, visible_slice(macd_cache),
                    visible_slice(macd_signal_cache),
                    visible_slice(macd_hist_cache), window_ms, visible_start_ms,
                    candle_duration_ms, candle_right, pane_top, pane_bottom,
                    capture({vroom::LineKind::Macd}),
                    capture({vroom::LineKind::MacdSignal}),
                    capture({vroom::LineKind::MacdHistogram}), morph_t);
                break;
            }
            case vroom::PaneKind::Atr: {
                ensure_atr();
                vroom::atr_pane::draw(canvas, *this, lay, visible, n,
                                      visible_slice(atr_cache), window_ms,
                                      visible_start_ms, candle_duration_ms,
                                      candle_right, pane_top, pane_bottom,
                                      capture({vroom::LineKind::Atr}), morph_t);
                break;
            }
        }
        pane_top = pane_bottom;
    }
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

void VroomChart::ensure_ichimoku() {
    if (!ichimoku.enabled || !ichimoku_dirty) return;
    vroom::ichimoku::compute(candles.data(), candles.size(),
                             ichimoku.tenkan_period, ichimoku.kijun_period,
                             ichimoku.senkou_b_period, ich_tenkan_cache,
                             ich_kijun_cache, ich_senkou_a_cache,
                             ich_senkou_b_cache, ich_chikou_cache);
    ichimoku_dirty = false;
}

void VroomChart::ensure_fvg() {
    if (!fvg.enabled || !fvg_dirty) return;
    vroom::fvg::compute(candles.data(), candles.size(), fvg.max_bars_back,
                        fvg.wait_for_close != 0, fvg.fill_type, fvg_cache);
    fvg_dirty = false;
}

void VroomChart::ensure_footprint_buckets() {
    if (!footprint_buckets_dirty) return;
    footprint_buckets = vroom::footprints::build_buckets(
        candles.data(), candles.size(), candle_duration_ms, footprints.data(),
        footprints.size());
    footprint_buckets_dirty = false;
}

vroom::IndexRange VroomChart::ichimoku_source_range(int64_t shift_ms) const {
    return vroom::ichimoku::shifted_source_range(
        candles.data(), candles.size(), visible_start_ms - candle_duration_ms,
        visible_end_ms + candle_duration_ms, shift_ms);
}

void VroomChart::draw_overlay_fills(SkCanvas* canvas, const vroom::Layout& lay,
                                    const vroom::PriceBounds& bounds,
                                    const VroomCandle* visible, std::size_t n,
                                    std::size_t first, int64_t window_ms,
                                    float candle_right, float candle_area_h,
                                    bool use_capture, float morph_t) {
    const std::size_t sz = candles.size();
    const auto capture = [&](vroom::LineKey key) -> const vroom::LineMorph* {
        return use_capture ? vroom::find_line_morph(morph_lines, key) : nullptr;
    };

    if (bollinger.enabled) {
        ensure_bollinger();
        if (bollinger.fill_enabled && bb_upper_cache.size() == sz &&
            bb_lower_cache.size() == sz) {
            vroom::ma_overlay::fill_between(
                canvas, lay, bounds, visible, n, bb_upper_cache.data() + first,
                bb_lower_cache.data() + first, window_ms, visible_start_ms,
                candle_duration_ms, candle_right, candle_area_h,
                bollinger.upper_color, bollinger.fill_opacity,
                capture({vroom::LineKind::BollingerUpper}),
                capture({vroom::LineKind::BollingerLower}), morph_t);
        }
    }

    if (ichimoku.enabled && ichimoku.cloud_enabled) {
        ensure_ichimoku();
        if (ich_senkou_a_cache.size() == sz && ich_senkou_b_cache.size() == sz) {
            const int64_t shift_ms =
                static_cast<int64_t>(ichimoku.displacement) * candle_duration_ms;
            const auto r = ichimoku_source_range(shift_ms);
            // A fade's outgoing half has no new data to slice, but its capture
            // still has to paint.
            const bool has_new = n > 0 && r.end > r.start;
            vroom::ma_overlay::fill_cloud(
                canvas, lay, bounds,
                has_new ? candles.data() + r.start : candles.data(),
                has_new ? r.end - r.start : 0,
                has_new ? ich_senkou_a_cache.data() + r.start : nullptr,
                has_new ? ich_senkou_b_cache.data() + r.start : nullptr,
                window_ms, visible_start_ms, candle_duration_ms, candle_right,
                candle_area_h, ichimoku.bullish_cloud_color,
                ichimoku.bearish_cloud_color, ichimoku.cloud_opacity, shift_ms,
                capture({vroom::LineKind::IchimokuSenkouA}),
                capture({vroom::LineKind::IchimokuSenkouB}), morph_t);
        }
    }
}

void VroomChart::draw_overlay_lines(SkCanvas* canvas, const vroom::Layout& lay,
                                    const vroom::PriceBounds& bounds,
                                    const VroomCandle* visible, std::size_t n,
                                    std::size_t first, int64_t window_ms,
                                    float candle_right, float candle_area_h,
                                    bool use_capture, float morph_t) {
    const std::size_t sz = candles.size();
    const auto capture = [&](vroom::LineKey key) -> const vroom::LineMorph* {
        return use_capture ? vroom::find_line_morph(morph_lines, key) : nullptr;
    };
    // One price-pane line against the visible slice. `n == 0` (a fade's
    // outgoing half) leaves only the capture to draw.
    const auto stroke = [&](vroom::LineKey key, const std::vector<double>& cache,
                            uint32_t color, float width,
                            const unsigned char* breaks) {
        if (cache.size() != sz) return;
        vroom::ma_overlay::draw(canvas, lay, bounds, visible, n,
                                cache.data() + first, window_ms,
                                visible_start_ms, candle_duration_ms,
                                candle_right, candle_area_h, color, width,
                                breaks, 1.f, 0, capture(key), morph_t);
    };

    if (!overlays.empty()) {
        ensure_overlays();
        for (std::size_t k = 0; k < overlays.size(); ++k) {
            stroke(vroom::overlay_line_key(overlays[k], k), overlay_caches[k],
                   overlays[k].color, overlays[k].width, nullptr);
        }
    }

    if (vwap.enabled) {
        ensure_vwap();
        stroke({vroom::LineKind::Vwap}, vwap_cache,
               vroom::style::color_or(vwap.color, kVwapLine),
               vroom::style::width_or(vwap.width, 1.5f),
               vwap_breaks.size() == sz ? vwap_breaks.data() + first : nullptr);
    }

    if (bollinger.enabled) {
        ensure_bollinger();
        stroke({vroom::LineKind::BollingerUpper}, bb_upper_cache,
               bollinger.upper_color, bollinger.upper_width, nullptr);
        stroke({vroom::LineKind::BollingerLower}, bb_lower_cache,
               bollinger.lower_color, bollinger.lower_width, nullptr);
        stroke({vroom::LineKind::BollingerMiddle}, bb_middle_cache,
               bollinger.middle_color, bollinger.middle_width, nullptr);
    }

    if (ichimoku.enabled) {
        ensure_ichimoku();
        const int64_t shift_ms =
            static_cast<int64_t>(ichimoku.displacement) * candle_duration_ms;
        // The displaced spans read from their own source slice, so they can't
        // go through `stroke`: the query window shifts back by however far the
        // drawing shifts forward.
        const auto span = [&](vroom::LineKey key,
                              const std::vector<double>& cache, uint32_t color,
                              float width, int64_t line_shift_ms) {
            if (cache.size() != sz) return;
            const auto r = ichimoku_source_range(line_shift_ms);
            const bool has_new = n > 0 && r.end > r.start;
            vroom::ma_overlay::draw(
                canvas, lay, bounds,
                has_new ? candles.data() + r.start : candles.data(),
                has_new ? r.end - r.start : 0,
                has_new ? cache.data() + r.start : nullptr, window_ms,
                visible_start_ms, candle_duration_ms, candle_right,
                candle_area_h, color, width, nullptr, 1.f, line_shift_ms,
                capture(key), morph_t);
        };
        // Span B under span A where the two edges touch, then the unshifted
        // signal lines, then chikou on top of everything.
        if (ichimoku.senkou_b_enabled) {
            span({vroom::LineKind::IchimokuSenkouB}, ich_senkou_b_cache,
                 ichimoku.senkou_b_color, ichimoku.senkou_b_width, shift_ms);
        }
        if (ichimoku.senkou_a_enabled) {
            span({vroom::LineKind::IchimokuSenkouA}, ich_senkou_a_cache,
                 ichimoku.senkou_a_color, ichimoku.senkou_a_width, shift_ms);
        }
        if (ichimoku.kijun_enabled) {
            span({vroom::LineKind::IchimokuKijun}, ich_kijun_cache,
                 ichimoku.kijun_color, ichimoku.kijun_width, 0);
        }
        if (ichimoku.tenkan_enabled) {
            span({vroom::LineKind::IchimokuTenkan}, ich_tenkan_cache,
                 ichimoku.tenkan_color, ichimoku.tenkan_width, 0);
        }
        if (ichimoku.chikou_enabled) {
            span({vroom::LineKind::IchimokuChikou}, ich_chikou_cache,
                 ichimoku.chikou_color, ichimoku.chikou_width, -shift_ms);
        }
    }
}

void VroomChart::capture_morph_lines(const vroom::Layout& lay,
                                     const vroom::PriceBounds& bounds,
                                     vroom::IndexRange range,
                                     int64_t window_ms) {
    morph_lines.clear();
    const float area_w = vroom::candle_area_width(lay);
    const std::size_t n = range.end - range.start;
    if (area_w <= 0.f || window_ms <= 0 || n == 0) return;
    const ::VroomCandle* visible = candles.data() + range.start;
    const std::size_t sz = candles.size();

    // Captures one series newest-first, matching the slot indexing every morph
    // draw pairs on. `src` is the slice the values were drawn against — the
    // visible one for everything except Ichimoku's displaced spans, which plot
    // at `time_ms + shift_ms` from a window of their own. `frac` maps a value
    // into its band; it's the only part that differs between the price pane and
    // an indicator pane.
    const auto capture = [&](vroom::LineKey key, const ::VroomCandle* src,
                             std::size_t src_n, const double* values,
                             int64_t shift_ms, double scale, auto&& frac) {
        if (!values || src_n == 0) return;
        vroom::LineMorph line;
        line.key = key;
        line.scale = scale;
        line.pts.resize(src_n);
        for (std::size_t k = 0; k < src_n; ++k) {
            const std::size_t i = src_n - 1 - k;
            const double v = values[i];
            const bool defined = std::isfinite(v);
            line.pts[k] = vroom::LineSnapshot{
                vroom::candle_center_x(lay, src[i].time_ms + shift_ms,
                                       candle_duration_ms, visible_start_ms,
                                       window_ms) /
                    area_w,
                defined ? static_cast<float>(frac(v)) : 0.f,
                defined,
            };
        }
        morph_lines.push_back(std::move(line));
    };

    // Price-pane series share the band the candle capture was taken against, so
    // they keep their position relative to the candles through the switch.
    const auto price_frac = [&](double v) {
        return vroom::price_fraction(bounds, v);
    };
    const auto price_series = [&](vroom::LineKey key,
                                  const std::vector<double>& cache) {
        if (cache.size() != sz) return;
        capture(key, visible, n, cache.data() + range.start, 0, 0.0, price_frac);
    };

    if (!overlays.empty()) {
        ensure_overlays();
        for (std::size_t k = 0; k < overlays.size(); ++k) {
            price_series(vroom::overlay_line_key(overlays[k], k),
                         overlay_caches[k]);
        }
    }

    if (vwap.enabled) {
        ensure_vwap();
        price_series({vroom::LineKind::Vwap}, vwap_cache);
    }

    if (bollinger.enabled) {
        ensure_bollinger();
        price_series({vroom::LineKind::BollingerUpper}, bb_upper_cache);
        price_series({vroom::LineKind::BollingerMiddle}, bb_middle_cache);
        price_series({vroom::LineKind::BollingerLower}, bb_lower_cache);
    }

    if (ichimoku.enabled) {
        ensure_ichimoku();
        const int64_t shift_ms =
            static_cast<int64_t>(ichimoku.displacement) * candle_duration_ms;
        // Same source windows draw_chart uses, so slot 0 is the same rightmost
        // vertex on both sides of the switch even for a span running past the
        // newest candle into empty time.
        const auto span = [&](vroom::LineKey key,
                              const std::vector<double>& cache,
                              int64_t line_shift_ms) {
            if (cache.size() != sz) return;
            const auto r = ichimoku_source_range(line_shift_ms);
            if (r.end <= r.start) return;
            capture(key, candles.data() + r.start, r.end - r.start,
                    cache.data() + r.start, line_shift_ms, 0.0, price_frac);
        };
        span({vroom::LineKind::IchimokuTenkan}, ich_tenkan_cache, 0);
        span({vroom::LineKind::IchimokuKijun}, ich_kijun_cache, 0);
        span({vroom::LineKind::IchimokuSenkouA}, ich_senkou_a_cache, shift_ms);
        span({vroom::LineKind::IchimokuSenkouB}, ich_senkou_b_cache, shift_ms);
        span({vroom::LineKind::IchimokuChikou}, ich_chikou_cache, -shift_ms);
    }

    // Pane series normalize against their pane's own band rather than the price
    // one, so each pane's auto-fit is free to change across the switch without
    // the capture moving.
    const auto pane_slice = [&](const std::vector<double>& cache) -> const double* {
        return cache.size() == sz ? cache.data() + range.start : nullptr;
    };

    if (rsi.enabled) {
        ensure_rsi();
        const auto frac = [&](double v) {
            return vroom::rsi::band_fraction(v, rsi_y_scale);
        };
        capture({vroom::LineKind::Rsi}, visible, n, pane_slice(rsi_cache), 0, 0.0,
                frac);
        if (rsi.ma_visible) {
            capture({vroom::LineKind::RsiMa}, visible, n,
                    pane_slice(rsi_ma_cache), 0, 0.0, frac);
        }
    }

    if (macd.enabled) {
        ensure_macd();
        const double* line = pane_slice(macd_cache);
        const double* signal = pane_slice(macd_signal_cache);
        const double* hist = pane_slice(macd_hist_cache);
        const double scale = vroom::macd::autoscale(
            macd.line_visible ? line : nullptr,
            macd.signal_visible ? signal : nullptr,
            macd.hist_visible ? hist : nullptr, n);
        const auto frac = [&](double v) {
            return vroom::macd::band_fraction(v, scale, macd_y_scale);
        };
        capture({vroom::LineKind::Macd}, visible, n, line, 0, scale, frac);
        capture({vroom::LineKind::MacdSignal}, visible, n, signal, 0, scale, frac);
        capture({vroom::LineKind::MacdHistogram}, visible, n, hist, 0, scale,
                frac);
    }

    if (atr.enabled) {
        ensure_atr();
        const double* line = pane_slice(atr_cache);
        const double scale = vroom::atr::autoscale(line, n);
        capture({vroom::LineKind::Atr}, visible, n, line, 0, scale,
                [&](double v) {
                    return vroom::atr::band_fraction(v, scale, atr_y_scale);
                });
    }
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
        // First half: only what was captured, at the axis envelope opacity.
        // Layers with no snapshot (volume, liquidity, FVG, drawings) would
        // already be the new data, so they stay hidden until the incoming half.
        // morph_t = 0 draws every capture pixel-identically.
        const float layer = axis_phase.opacity;
        // Groups the captured indicators into one layer rather than threading
        // the envelope through as a per-paint alpha, so overlapping strokes
        // composite once and the panes keep their signatures. The price series
        // below fades per-paint, as it always has.
        const auto push_fade_layer = [&]() -> bool {
            if (layer >= 0.999f) return false;
            canvas->saveLayerAlpha(nullptr,
                                   static_cast<U8CPU>(layer * 255.f + 0.5f));
            return true;
        };
        if (layer > 0.f) {
            // Behind the candles, as in the settled z-order.
            const bool fills_wrapped = push_fade_layer();
            draw_overlay_fills(canvas, lay, bounds, visible, 0, range.start,
                               window_ms, candle_right, candle_area_h, true, 0.f);
            if (fills_wrapped) canvas->restore();

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

            // Over the candles, and the panes below them. One layer for both:
            // they occupy disjoint bands, so nothing overlaps across the two.
            const bool lines_wrapped = push_fade_layer();
            draw_overlay_lines(canvas, lay, bounds, visible, 0, range.start,
                               window_ms, candle_right, candle_area_h, true, 0.f);
            if (lay.indicator_area_h > 0.f) {
                draw_indicator_panes(canvas, lay, visible, 0, range.start,
                                     window_ms, candle_right, candle_area_h,
                                     true, 0.f);
            }
            if (lines_wrapped) canvas->restore();
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

            // 4.65. Fair Value Gap boxes — behind the candles, so the bars that
            // formed each imbalance still read over their own shading.
            if (fvg.enabled) {
                ensure_fvg();
                vroom::fvg_overlay::draw_boxes(canvas, *this, lay, bounds,
                                               window_ms, candle_right,
                                               candle_area_h);
            }

            // 4.7 / 4.75. Bollinger fill and Ichimoku cloud, behind the candles.
            draw_overlay_fills(canvas, lay, bounds, visible, n, range.start,
                               window_ms, candle_right, candle_area_h, reshape,
                               morph_t);

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

            // 5.5 - 5.67. Moving averages, VWAP, Bollinger and Ichimoku lines.
            draw_overlay_lines(canvas, lay, bounds, visible, n, range.start,
                               window_ms, candle_right, candle_area_h, reshape,
                               morph_t);

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
                draw_indicator_panes(canvas, lay, visible, n, range.start,
                                     window_ms, candle_right, candle_area_h,
                                     false, 1.f);
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

        // 7.54. Fair Value Gap labels — the boxes themselves are back behind
        //       the candles, but their text has to clear the bars it sits over.
        vroom::fvg_overlay::draw_labels(canvas, *this, lay, bounds, window_ms,
                                        candle_right, candle_area_h);

        // 7.55. Consumer-supplied price status lines — same tier as the
        //       current-price indicator (their badges must cover the labels
        //       underneath), but after it so a resting order at the last close
        //       stays readable.
        vroom::price_lines::draw(canvas, *this, lay, bounds, candle_right,
                                   candle_area_h);

        // 7.56. Footprint badges — data-anchored chrome that must not be hidden
        //       by candles or overlays, so it draws with the price lines rather
        //       than back at the drawings layer. Below the crosshair, which the
        //       user is actively pointing with.
        ensure_footprint_buckets();
        vroom::footprints::draw(canvas, *this, lay, bounds, window_ms);
    }

    // 7.6. Indicator panes stacked below the candles, ordered by enable
    //      sequence (most recently enabled at the bottom). Each pane is
    //      INDICATOR_HEIGHT_FRAC of the height; the candle pane already shrank
    //      to fit them (see layout()). A fade already drew them with the scene.
    if (!fade_swap && lay.indicator_area_h > 0.f) {
        // candle_area_h == price_pane_bottom(lay), the top of the band.
        draw_indicator_panes(canvas, lay, visible, n, range.start, window_ms,
                             candle_right, candle_area_h, reshape, morph_t);
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
