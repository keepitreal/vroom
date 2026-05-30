// vroom_chart.h — public C facade for the vroom chart core.
//
// Threading: All calls must happen on a single thread (the render thread the
// host picked). The core does no internal locking.
//
// Rendering model: push. The core fires `on_redraw_requested` when something
// visual changed. The host marks dirty and calls `vroom_chart_draw` on the
// next vsync — never re-enter draw from the callback.
//
// Gestures: host-driven. The host wires its native gesture recognizers
// (UIPanGestureRecognizer / Android GestureDetector / RNGH) and calls
// `vroom_chart_pan`, `vroom_chart_zoom`, `vroom_chart_set_crosshair`,
// `vroom_chart_clear_crosshair`. The core has no gesture state machine.

#ifndef VROOM_CHART_H
#define VROOM_CHART_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward-declare Skia types as opaque so this header has no Skia dependency.
// The implementation casts back to SkCanvas* internally.
typedef struct SkCanvas SkCanvas;

typedef struct VroomChart VroomChart;

// ---- Data types -----------------------------------------------------------

typedef struct {
    int64_t time_ms;   // epoch milliseconds, ascending, non-overlapping
    double  open;
    double  high;
    double  low;
    double  close;
    double  volume;
} VroomCandle;

// ---- Styling keys ---------------------------------------------------------

typedef enum {
    VROOM_COLOR_BACKGROUND = 0,
    VROOM_COLOR_BULL,
    VROOM_COLOR_BEAR,
    VROOM_COLOR_WICK,
    VROOM_COLOR_GRID,
    VROOM_COLOR_AXIS_TEXT,
    VROOM_COLOR_CROSSHAIR,
    VROOM_COLOR_TOOLTIP_BG,
    VROOM_COLOR_TOOLTIP_TEXT,
    VROOM_COLOR_COUNT_
} VroomColorKey;

typedef enum {
    VROOM_FLOAT_CANDLE_WIDTH_RATIO = 0,  // 0..1 of slot width
    VROOM_FLOAT_WICK_WIDTH_PX,
    VROOM_FLOAT_RIGHT_PADDING_PX,
    VROOM_FLOAT_AXIS_FONT_SIZE_PX,
    VROOM_FLOAT_COUNT_
} VroomFloatKey;

// ---- Callbacks ------------------------------------------------------------

typedef struct {
    void (*on_redraw_requested)(void* ctx);
    void (*on_viewport_changed)(void* ctx, int64_t start_ms, int64_t end_ms);
    void (*on_crosshair_changed)(void* ctx, bool active, int64_t ms, double price);
} VroomCallbacks;

// ---- Lifecycle ------------------------------------------------------------

VroomChart* vroom_chart_create(const VroomCallbacks* cb, void* user_ctx);
void        vroom_chart_destroy(VroomChart* chart);

// ---- Data (copied internally; caller may free immediately) ----------------

void vroom_chart_set_candles(VroomChart* chart, const VroomCandle* data, size_t count);
void vroom_chart_append_candle(VroomChart* chart, const VroomCandle* c);
void vroom_chart_update_last(VroomChart* chart, const VroomCandle* c);

// ---- Sizing ---------------------------------------------------------------

void vroom_chart_set_size(VroomChart* chart, float width_px, float height_px, float px_ratio);

// ---- Viewport -------------------------------------------------------------

void vroom_chart_set_visible_range(VroomChart* chart, int64_t start_ms, int64_t end_ms);
void vroom_chart_pan(VroomChart* chart, float dx_px, float dy_px);
void vroom_chart_zoom(VroomChart* chart, float scale, float focus_x_px, float focus_y_px);

// ---- Crosshair ------------------------------------------------------------

void vroom_chart_set_crosshair(VroomChart* chart, float x_px, float y_px);
void vroom_chart_clear_crosshair(VroomChart* chart);

// ---- Rendering ------------------------------------------------------------

void vroom_chart_draw(VroomChart* chart, SkCanvas* canvas);

// ---- Styling --------------------------------------------------------------

void vroom_chart_set_color(VroomChart* chart, VroomColorKey key, uint32_t argb);
void vroom_chart_set_float(VroomChart* chart, VroomFloatKey key, float value);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VROOM_CHART_H
