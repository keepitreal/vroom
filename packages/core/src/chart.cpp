// chart.cpp — stub implementation behind the C facade.
//
// This is a skeleton: every entry point exists and does the minimum needed
// to keep the API contract honest (firing callbacks, storing data) but the
// actual Skia rendering and viewport math live in viewport.cpp / renderer.cpp
// (TODO). Real implementations follow once the test bench is wired up.

#include "vroom/vroom_chart.h"

#include <cstdlib>
#include <vector>

struct VroomChart {
    VroomCallbacks cb{};
    void* user_ctx = nullptr;

    std::vector<VroomCandle> candles;

    float width_px = 0.f;
    float height_px = 0.f;
    float px_ratio = 1.f;

    int64_t visible_start_ms = 0;
    int64_t visible_end_ms = 0;

    bool   crosshair_active = false;
    float  crosshair_x_px = 0.f;
    float  crosshair_y_px = 0.f;

    uint32_t colors[VROOM_COLOR_COUNT_] = {};
    float    floats[VROOM_FLOAT_COUNT_] = {};

    void request_redraw() {
        if (cb.on_redraw_requested) cb.on_redraw_requested(user_ctx);
    }
};

VroomChart* vroom_chart_create(const VroomCallbacks* cb, void* user_ctx) {
    auto* c = new VroomChart();
    if (cb) c->cb = *cb;
    c->user_ctx = user_ctx;
    return c;
}

void vroom_chart_destroy(VroomChart* chart) {
    delete chart;
}

void vroom_chart_set_candles(VroomChart* chart, const VroomCandle* data, size_t count) {
    if (!chart) return;
    chart->candles.assign(data, data + count);
    chart->request_redraw();
}

void vroom_chart_append_candle(VroomChart* chart, const VroomCandle* c) {
    if (!chart || !c) return;
    chart->candles.push_back(*c);
    chart->request_redraw();
}

void vroom_chart_update_last(VroomChart* chart, const VroomCandle* c) {
    if (!chart || !c || chart->candles.empty()) return;
    chart->candles.back() = *c;
    chart->request_redraw();
}

void vroom_chart_set_size(VroomChart* chart, float w, float h, float ratio) {
    if (!chart) return;
    chart->width_px = w;
    chart->height_px = h;
    chart->px_ratio = ratio;
    chart->request_redraw();
}

void vroom_chart_set_visible_range(VroomChart* chart, int64_t start_ms, int64_t end_ms) {
    if (!chart) return;
    chart->visible_start_ms = start_ms;
    chart->visible_end_ms = end_ms;
    if (chart->cb.on_viewport_changed)
        chart->cb.on_viewport_changed(chart->user_ctx, start_ms, end_ms);
    chart->request_redraw();
}

void vroom_chart_pan(VroomChart* chart, float /*dx_px*/, float /*dy_px*/) {
    // TODO: translate pixel delta into time-domain delta via viewport.
    if (chart) chart->request_redraw();
}

void vroom_chart_zoom(VroomChart* chart, float /*scale*/, float /*fx*/, float /*fy*/) {
    // TODO: pinch-zoom around focus point.
    if (chart) chart->request_redraw();
}

void vroom_chart_set_crosshair(VroomChart* chart, float x, float y) {
    if (!chart) return;
    chart->crosshair_active = true;
    chart->crosshair_x_px = x;
    chart->crosshair_y_px = y;
    if (chart->cb.on_crosshair_changed)
        chart->cb.on_crosshair_changed(chart->user_ctx, true, 0 /*TODO*/, 0.0 /*TODO*/);
    chart->request_redraw();
}

void vroom_chart_clear_crosshair(VroomChart* chart) {
    if (!chart) return;
    chart->crosshair_active = false;
    if (chart->cb.on_crosshair_changed)
        chart->cb.on_crosshair_changed(chart->user_ctx, false, 0, 0.0);
    chart->request_redraw();
}

void vroom_chart_draw(VroomChart* /*chart*/, SkCanvas* /*canvas*/) {
    // TODO: real renderer. Stub draws nothing yet so the project builds
    // without a Skia link dependency.
}

void vroom_chart_set_color(VroomChart* chart, VroomColorKey key, uint32_t argb) {
    if (!chart || key < 0 || key >= VROOM_COLOR_COUNT_) return;
    if (chart->colors[key] == argb) return;
    chart->colors[key] = argb;
    chart->request_redraw();
}

void vroom_chart_set_float(VroomChart* chart, VroomFloatKey key, float value) {
    if (!chart || key < 0 || key >= VROOM_FLOAT_COUNT_) return;
    if (chart->floats[key] == value) return;
    chart->floats[key] = value;
    chart->request_redraw();
}
