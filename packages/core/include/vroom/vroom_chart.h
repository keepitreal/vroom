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

typedef struct VroomCandle {
    int64_t time_ms;   // epoch milliseconds, ascending, non-overlapping
    double  open;
    double  high;
    double  low;
    double  close;
    double  volume;
} VroomCandle;

// A moving-average overlay line drawn on the price pane.
typedef struct VroomOverlay {
    int32_t  kind;    // 0 = SMA, 1 = EMA
    int32_t  period;  // lookback in candles (>= 1)
    int32_t  source;  // 0=close,1=open,2=high,3=low,4=hl2,5=hlc3,6=ohlc4
    uint32_t color;   // 0xAARRGGBB
    float    width;   // stroke width in px
} VroomOverlay;

// A drawing anchor in data space (so a drawing tracks the candles on pan/zoom).
typedef struct VroomDrawPoint {
    int64_t time_ms;  // epoch milliseconds (not snapped to a candle slot)
    double  price;
} VroomDrawPoint;

// A committed line drawing: a two-point trendline on the price pane.
typedef struct VroomDrawing {
    VroomDrawPoint a;
    VroomDrawPoint b;
    uint32_t       color;  // 0xAARRGGBB
    float          width;  // stroke width in px
} VroomDrawing;

// A continuous data coordinate at a pixel position (no candle snapping). Used to
// translate a drawing-tool click into a data-space anchor.
typedef struct VroomCoord {
    int64_t time_ms;
    double  price;
} VroomCoord;

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
    VROOM_COLOR_CROSSHAIR_TARGET,  // the hollow ring/dot at the intersection
    VROOM_COLOR_BORDER_BULL,       // bull body 1px outline; 0 alpha => inherit BULL fill
    VROOM_COLOR_BORDER_BEAR,       // bear body 1px outline; 0 alpha => inherit BEAR fill
    VROOM_COLOR_WICK_BULL,         // bull wick; 0 alpha => inherit BULL fill
    VROOM_COLOR_WICK_BEAR,         // bear wick; 0 alpha => inherit BEAR fill
    VROOM_COLOR_ACCENT_BULL,       // generic up color: price indicator, volume, MACD
    VROOM_COLOR_ACCENT_BEAR,       // generic down color
    VROOM_COLOR_COUNT_
} VroomColorKey;

typedef enum {
    VROOM_FLOAT_CANDLE_WIDTH_RATIO = 0,  // 0..1 of slot width
    VROOM_FLOAT_WICK_WIDTH_PX,
    VROOM_FLOAT_RIGHT_PADDING_PX,        // small gutter between candles and y-axis
    VROOM_FLOAT_AXIS_FONT_SIZE_PX,
    VROOM_FLOAT_Y_AXIS_WIDTH_RATIO,      // fallback y-axis width when no typeface
    VROOM_FLOAT_X_AXIS_HEIGHT_PX,        // bottom strip reserved for time labels
    VROOM_FLOAT_VOLUME_OPACITY,          // volume bar opacity (1=opaque)
    VROOM_FLOAT_INDICATOR_HEIGHT_FRAC,   // below-chart indicator pane, fraction of height
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

// Reads the current visible time window. Either out pointer may be null.
// Both are 0 when the window is still uninitialized.
void vroom_chart_get_visible_range(VroomChart* chart,
                                   int64_t* out_start_ms, int64_t* out_end_ms);

// Reset to the fresh-mount view: frame the most recent ~80 candles and
// re-enable continuous y auto-fit (the price range follows the visible candles
// until the next manual y gesture). Use when the data series is wholesale
// replaced — e.g. switching assets. With no candles loaded, clears the window
// to 0/0 so the next set_candles applies the default framing.
void vroom_chart_reset_view(VroomChart* chart);

// Re-enable continuous y auto-fit only; the time window is untouched. Use
// after repositioning the window for a same-asset data swap (e.g. a timeframe
// switch) so the price scale re-fits the newly visible candles.
void vroom_chart_reset_price_scale(VroomChart* chart);

void vroom_chart_pan(VroomChart* chart, float dx_px, float dy_px);
// Directional zoom. scale_x scales the time window around focus_x_px (>1 =
// narrower window, wider candles); scale_y scales the price range around
// focus_y_px (>1 = taller candles). Pass 1.0 for an axis to leave it untouched.
void vroom_chart_zoom(VroomChart* chart, float scale_x, float scale_y,
                      float focus_x_px, float focus_y_px);

// Two-finger translation. Shifts the time window like pan (dx) and shifts
// the price bounds vertically (dy) without changing their range — so the
// chart slides without rescaling. dy > 0 (drag down) moves content down
// (price labels move down to higher numbers).
void vroom_chart_translate(VroomChart* chart, float dx_px, float dy_px);

// Axis-drag controls — used by JS gesture handlers that detect a drag
// started on the y-axis or x-axis strip. Scaling pivots around the natural
// anchor (price center for y, right edge for x).
//
// Sign conventions:
//   scale_price_axis: dy > 0 (drag down) widens the price range → candles shrink
//   scale_time_axis:  dx > 0 (drag right) widens the time range → candles thin
void vroom_chart_scale_price_axis(VroomChart* chart, float dy_px);
void vroom_chart_scale_time_axis(VroomChart* chart, float dx_px);

// Scale the y-axis of the below-chart indicator pane that contains y_px.
// dy_px > 0 (drag down) zooms out (widens the visible value range); dy_px < 0
// zooms in. RSI scales about 50, MACD about its zero line. No-op when y_px is
// not over an indicator pane.
void vroom_chart_scale_indicator_axis(VroomChart* chart, float y_px, float dy_px);

// Drag the separator between the price pane and the below-chart indicator band.
// dy_px > 0 (drag down) grows the price pane and shrinks the indicator band.
// Candle pixel scale is preserved: the price range is widened/narrowed in step
// (anchored at the top price), so the viewport reveals/hides price rather than
// rescaling candles. No-op when no indicator pane is shown.
void vroom_chart_resize_indicator_pane(VroomChart* chart, float dy_px);

// Reads the current y-axis width, x-axis height, and below-chart indicator pane
// height in pixels so callers can hit-test gestures against each region on the
// JS side. out_indicator_height_px is 0 when no indicator pane is shown.
void vroom_chart_get_axis_metrics(VroomChart* chart,
                                   float* out_y_axis_width_px,
                                   float* out_x_axis_height_px,
                                   float* out_indicator_height_px);

// ---- Crosshair ------------------------------------------------------------

void vroom_chart_set_crosshair(VroomChart* chart, float x_px, float y_px);
void vroom_chart_clear_crosshair(VroomChart* chart);

// Fills *out with the OHLCV of the candle the crosshair currently snaps to and
// returns true. Returns false (leaving *out untouched) when the crosshair is
// inactive or there are no visible candles. Stateless — recomputes the snap
// from the current crosshair x and visible window, matching what's rendered.
bool vroom_chart_get_crosshair_candle(VroomChart* chart, VroomCandle* out);

// What the crosshair currently snaps to, including future candle-aligned slots
// to the right of the last candle (empty space the chart leaves ahead of the
// most recent bar). `time_ms` is the snapped slot's period-start time — always
// valid. `has_candle` is true when a real candle sits at that slot, in which
// case `candle` holds its OHLCV; in the future region `has_candle` is false and
// `candle` is left untouched.
typedef struct VroomCrosshairInfo {
    int64_t     time_ms;
    bool        has_candle;
    VroomCandle candle;
} VroomCrosshairInfo;

// Fills *out with the snapped slot the crosshair currently sits on and returns
// true. Returns false (leaving *out untouched) when the crosshair is inactive
// or there are no visible candles. Stateless — recomputes the snap from the
// current crosshair x and visible window, matching what's rendered.
bool vroom_chart_get_crosshair_info(VroomChart* chart, VroomCrosshairInfo* out);

// ---- Indicators -----------------------------------------------------------

// Configures the RSI indicator (rendered in a pane below the candles). `period`
// is the RSI lookback in candle counts (clamped >= 2). `upper_band`/`lower_band`
// are the overbought/oversold reference levels (0..100; default 70/30).
// `ma_enabled` toggles the RSI-based moving-average trendline and `ma_period`
// is its length (clamped >= 1). When enabled, the candle pane shrinks by
// VROOM_FLOAT_INDICATOR_HEIGHT_FRAC.
void vroom_chart_set_rsi(VroomChart* chart, bool enabled, int period,
                         double upper_band, double lower_band,
                         bool ma_enabled, int ma_period);

// Configures the MACD indicator (its own pane below the candles). `fast`/`slow`
// are the EMA lengths (clamped >= 1, slow forced > fast; default 12/26) and
// `signal` is the signal-line EMA length (clamped >= 1; default 9). Panes stack
// in enable order, most recently enabled at the bottom.
void vroom_chart_set_macd(VroomChart* chart, bool enabled, int fast, int slow,
                          int signal);

// Replaces the full set of moving-average overlay lines (SMA/EMA) drawn on the
// price pane. Pass count 0 to clear them. Overlays don't reserve a pane.
void vroom_chart_set_overlays(VroomChart* chart, const VroomOverlay* overlays,
                              size_t count);

// Configures the session VWAP overlay (a single price-pane line). The session
// resets each UTC day shifted by `reset_offset_min` minutes (the configurable
// reset time). `color` is 0xAARRGGBB; `width` is the stroke px.
void vroom_chart_set_vwap(VroomChart* chart, bool enabled, int reset_offset_min,
                          uint32_t color, float width);

// ---- Drawings (line annotations) ------------------------------------------

// Replaces the full set of committed line drawings (data-anchored, so they track
// the candles on pan/zoom). Pass count 0 to clear. Drawings render on the price
// pane above the candles/overlays and below the axis labels & crosshair.
void vroom_chart_set_drawings(VroomChart* chart, const VroomDrawing* drawings,
                              size_t count);

// Sets the transient in-progress "draft" the drawing tool shows while the user
// places points. Node A is always shown; when `has_b`, node B is shown too.
// `guide != 0` also draws the guideline A->B (the live line preview); `guide == 0`
// draws node dots only (the committed segment already renders via set_drawings).
// `color`/`width` style the guideline to match the eventual line.
void vroom_chart_set_draft(VroomChart* chart, int64_t a_time, double a_price,
                           bool has_b, int64_t b_time, double b_price,
                           bool guide, uint32_t color, float width);

// Clears the draft (hides the in-progress node dots / guideline).
void vroom_chart_clear_draft(VroomChart* chart);

// Fills *out with the continuous data coordinate (time_ms, price) at pixel
// (x_px, y_px) using the free (non-snapped) mapping, and returns true. Returns
// false (leaving *out untouched) when there are no candles or the viewport is
// degenerate.
bool vroom_chart_coord_at(VroomChart* chart, float x_px, float y_px,
                          VroomCoord* out);

// ---- Rendering ------------------------------------------------------------

void vroom_chart_draw(VroomChart* chart, SkCanvas* canvas);

// True while any axis-label fade is still animating. Hosts that drive their own
// redraw loop (e.g. the web WASM build) poll this to know when to keep ticking.
bool vroom_chart_is_animating(VroomChart* chart);

// ---- Styling --------------------------------------------------------------

void vroom_chart_set_color(VroomChart* chart, VroomColorKey key, uint32_t argb);
void vroom_chart_set_float(VroomChart* chart, VroomFloatKey key, float value);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VROOM_CHART_H
