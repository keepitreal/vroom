// VroomChart core — Phase 1.
//
// Owns the candle buffer, layout, theme, and a cached sk_sp<SkPicture> built
// by SkPictureRecorder. The chart picture is rebuilt only when input state
// dirties (new candles, new size). The JSI bridge calls `render_picture()` to
// get the latest picture and wraps it as a JsiSkPicture for JS.

#include "vroom/vroom_chart.h"

#include <cstdlib>
#include <utility>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRect.h"
#pragma clang diagnostic pop

#include "chart_internal.h"
#include "viewport.h"

#include <algorithm>

namespace vroom {
namespace {

// Default theme — match the test-bench dark background.
constexpr uint32_t kDefaultColors[VROOM_COLOR_COUNT_] = {
    0xff0d1117,  // BACKGROUND
    0xff26a69a,  // BULL
    0xffef5350,  // BEAR
    0xffaaaaaa,  // WICK
    0xff21262d,  // GRID
    0xffc9d1d9,  // AXIS_TEXT
    0xffc9d1d9,  // CROSSHAIR
    0xff161b22,  // TOOLTIP_BG
    0xffc9d1d9,  // TOOLTIP_TEXT
};

constexpr float kDefaultFloats[VROOM_FLOAT_COUNT_] = {
    0.8f,   // CANDLE_WIDTH_RATIO
    1.0f,   // WICK_WIDTH_PX
    8.f,    // RIGHT_PADDING_PX
    12.f,   // AXIS_FONT_SIZE_PX
};

}  // namespace
}  // namespace vroom

struct VroomChart {
    VroomCallbacks cb{};
    void* user_ctx = nullptr;

    std::vector<VroomCandle> candles;

    float width_px = 0.f;
    float height_px = 0.f;
    float px_ratio = 1.f;

    bool   crosshair_active = false;
    float  crosshair_x_px = 0.f;
    float  crosshair_y_px = 0.f;

    uint32_t colors[VROOM_COLOR_COUNT_]{};
    float    floats[VROOM_FLOAT_COUNT_]{};

    sk_sp<SkPicture> chart_picture;
    bool chart_dirty = true;

    VroomChart() {
        for (int i = 0; i < VROOM_COLOR_COUNT_; ++i) colors[i] = vroom::kDefaultColors[i];
        for (int i = 0; i < VROOM_FLOAT_COUNT_; ++i) floats[i] = vroom::kDefaultFloats[i];
    }

    void mark_dirty() {
        chart_dirty = true;
        if (cb.on_redraw_requested) cb.on_redraw_requested(user_ctx);
    }

    vroom::Layout layout() const {
        return vroom::Layout{
            width_px,
            height_px,
            floats[VROOM_FLOAT_RIGHT_PADDING_PX],
            floats[VROOM_FLOAT_CANDLE_WIDTH_RATIO],
            0.05f,
            0.05f,
        };
    }

    void draw_chart(SkCanvas* canvas) const {
        const auto lay = layout();

        SkPaint bg;
        bg.setColor(colors[VROOM_COLOR_BACKGROUND]);
        canvas->drawRect(SkRect::MakeWH(width_px, height_px), bg);

        if (candles.empty()) return;

        const auto bounds = vroom::price_bounds(candles.data(), candles.size());
        const float body_w = vroom::candle_body_width(lay, candles.size());

        SkPaint bull_paint;
        bull_paint.setAntiAlias(true);
        bull_paint.setColor(colors[VROOM_COLOR_BULL]);

        SkPaint bear_paint;
        bear_paint.setAntiAlias(true);
        bear_paint.setColor(colors[VROOM_COLOR_BEAR]);

        SkPaint wick_bull;
        wick_bull.setAntiAlias(true);
        wick_bull.setColor(colors[VROOM_COLOR_BULL]);
        wick_bull.setStrokeWidth(floats[VROOM_FLOAT_WICK_WIDTH_PX]);
        wick_bull.setStyle(SkPaint::kStroke_Style);

        SkPaint wick_bear = wick_bull;
        wick_bear.setColor(colors[VROOM_COLOR_BEAR]);

        const float half_body = body_w * 0.5f;

        for (size_t i = 0; i < candles.size(); ++i) {
            const auto& c = candles[i];
            const bool bull = c.close >= c.open;

            const float cx = vroom::candle_center_x(lay, candles.size(), i);
            const float y_high = vroom::price_to_y(lay, bounds, c.high);
            const float y_low = vroom::price_to_y(lay, bounds, c.low);
            const float y_open = vroom::price_to_y(lay, bounds, c.open);
            const float y_close = vroom::price_to_y(lay, bounds, c.close);

            // Wick: thin vertical line from low to high.
            canvas->drawLine(cx, y_high, cx, y_low, bull ? wick_bull : wick_bear);

            // Body: rect from open to close.
            const float y_top = std::min(y_open, y_close);
            const float y_bot = std::max(y_open, y_close);
            const float h = std::max(1.f, y_bot - y_top);  // never collapse to 0
            canvas->drawRect(
                SkRect::MakeXYWH(cx - half_body, y_top, body_w, h),
                bull ? bull_paint : bear_paint);
        }
    }

    void rebuild_chart_picture() {
        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(SkRect::MakeWH(width_px, height_px));
        draw_chart(canvas);
        chart_picture = recorder.finishRecordingAsPicture();
        chart_dirty = false;
    }
};

// ---- C facade ----

VroomChart* vroom_chart_create(const VroomCallbacks* cb, void* user_ctx) {
    auto* c = new VroomChart();
    if (cb) c->cb = *cb;
    c->user_ctx = user_ctx;
    return c;
}

void vroom_chart_destroy(VroomChart* chart) { delete chart; }

void vroom_chart_set_candles(VroomChart* chart, const VroomCandle* data, size_t count) {
    if (!chart) return;
    chart->candles.assign(data, data + count);
    chart->mark_dirty();
}

void vroom_chart_append_candle(VroomChart* chart, const VroomCandle* c) {
    if (!chart || !c) return;
    chart->candles.push_back(*c);
    chart->mark_dirty();
}

void vroom_chart_update_last(VroomChart* chart, const VroomCandle* c) {
    if (!chart || !c || chart->candles.empty()) return;
    chart->candles.back() = *c;
    chart->mark_dirty();
}

void vroom_chart_set_size(VroomChart* chart, float w, float h, float ratio) {
    if (!chart) return;
    if (chart->width_px == w && chart->height_px == h && chart->px_ratio == ratio) return;
    chart->width_px = w;
    chart->height_px = h;
    chart->px_ratio = ratio;
    chart->mark_dirty();
}

void vroom_chart_set_visible_range(VroomChart* chart, int64_t /*start_ms*/, int64_t /*end_ms*/) {
    // Phase 1: viewport is always the full data range; ignore.
    if (chart) chart->mark_dirty();
}

void vroom_chart_pan(VroomChart* chart, float, float) { if (chart) chart->mark_dirty(); }
void vroom_chart_zoom(VroomChart* chart, float, float, float) { if (chart) chart->mark_dirty(); }

void vroom_chart_set_crosshair(VroomChart* chart, float x, float y) {
    if (!chart) return;
    chart->crosshair_active = true;
    chart->crosshair_x_px = x;
    chart->crosshair_y_px = y;
    chart->mark_dirty();
}

void vroom_chart_clear_crosshair(VroomChart* chart) {
    if (!chart) return;
    chart->crosshair_active = false;
    chart->mark_dirty();
}

void vroom_chart_draw(VroomChart* chart, SkCanvas* canvas) {
    if (!chart || !canvas) return;
    chart->draw_chart(canvas);
}

void vroom_chart_set_color(VroomChart* chart, VroomColorKey key, uint32_t argb) {
    if (!chart || key < 0 || key >= VROOM_COLOR_COUNT_) return;
    if (chart->colors[key] == argb) return;
    chart->colors[key] = argb;
    chart->mark_dirty();
}

void vroom_chart_set_float(VroomChart* chart, VroomFloatKey key, float value) {
    if (!chart || key < 0 || key >= VROOM_FLOAT_COUNT_) return;
    if (chart->floats[key] == value) return;
    chart->floats[key] = value;
    chart->mark_dirty();
}

namespace vroom {

// Declared in chart_internal.h; implemented here so it has direct access to
// VroomChart's private state.
sk_sp<SkPicture> render_chart_picture(VroomChart* chart) {
    if (!chart) return nullptr;
    if (chart->chart_dirty || !chart->chart_picture) {
        chart->rebuild_chart_picture();
    }
    return chart->chart_picture;
}

}  // namespace vroom
