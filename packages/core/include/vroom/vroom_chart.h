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

// Bollinger Bands overlay drawn on the price pane: a basis MA of `source` over
// `period`, banded at ± `mult` × population standard deviation of the same
// window. Per the standard semantics the stdev always uses the window's
// arithmetic mean, even when `basis_kind` selects an EMA basis line.
typedef struct VroomBollinger {
    int32_t  enabled;       // 0/1
    int32_t  period;        // lookback in candles (clamped >= 1; default 20)
    float    mult;          // stdev multiplier (clamped >= 0; default 2)
    int32_t  source;        // 0=close,1=open,2=high,3=low,4=hl2,5=hlc3,6=ohlc4
    int32_t  basis_kind;    // 0 = SMA, 1 = EMA
    uint32_t upper_color;   // 0xAARRGGBB
    float    upper_width;   // stroke px
    uint32_t middle_color;
    float    middle_width;
    uint32_t lower_color;
    float    lower_width;
    int32_t  fill_enabled;  // 0/1: translucent fill between upper and lower
    float    fill_opacity;  // 0..1, multiplied into upper_color's alpha
} VroomBollinger;

// MACD, drawn in its own pane below the candles: the difference between a fast
// and a slow moving average of `source`, a signal line smoothing that
// difference, and a histogram of the gap between the two.
//
// The style fields carry an inherit sentinel so an untouched config renders the
// stock look: a fully transparent color falls back to the theme (histogram) or
// to the built-in default (lines), and a non-positive width falls back to 1.5.
// The two `fading` histogram colors — used when a bar moves back toward zero,
// i.e. momentum is easing — fall back to their base color at half alpha.
typedef struct VroomMACD {
    int32_t  enabled;         // 0/1
    int32_t  fast;            // fast MA length (clamped >= 1; default 12)
    int32_t  slow;            // slow MA length (forced > fast; default 26)
    int32_t  signal;          // signal MA length (clamped >= 1; default 9)
    int32_t  source;          // 0=close,1=open,2=high,3=low,4=hl2,5=hlc3,6=ohlc4
    int32_t  ma_kind;         // fast/slow legs: 0 = SMA, 1 = EMA
    int32_t  signal_ma_kind;  // signal line: 0 = SMA, 1 = EMA

    uint32_t line_color;    // MACD line; 0xAARRGGBB, 0 inherits the default blue
    float    line_width;    // stroke px; <= 0 inherits 1.5
    int32_t  line_visible;  // 0/1
    uint32_t signal_color;  // 0 inherits the default orange
    float    signal_width;
    int32_t  signal_visible;

    int32_t  hist_visible;
    uint32_t hist_up_color;           // 0 inherits VROOM_COLOR_ACCENT_BULL
    uint32_t hist_up_fading_color;    // 0 inherits hist_up_color at half alpha
    uint32_t hist_down_color;         // 0 inherits VROOM_COLOR_ACCENT_BEAR
    uint32_t hist_down_fading_color;  // 0 inherits hist_down_color at half alpha

    uint32_t zero_color;    // 0 inherits the default gray
    int32_t  zero_visible;  // 0/1
} VroomMACD;

// RSI, drawn in its own pane below the candles: the index line, an optional
// moving-average trendline over it, and two dashed rules at the overbought and
// oversold levels. Close-only by design (Wilder's definition), so unlike the
// other averaged indicators it takes no `source`.
//
// The style fields carry the same inherit sentinel as VroomMACD: a fully
// transparent color falls back to the built-in default and a non-positive width
// falls back to 1.5.
typedef struct VroomRSI {
    int32_t  enabled;     // 0/1
    int32_t  period;      // lookback in candles (clamped >= 2; default 14)
    double   upper_band;  // overbought level (0..100; default 70)
    double   lower_band;  // oversold level (0..100; default 30)
    int32_t  ma_period;   // trendline length (clamped >= 1; default 14)
    int32_t  ma_kind;     // trendline: 0 = SMA, 1 = EMA
    int32_t  ma_visible;  // 0/1

    uint32_t line_color;    // 0 inherits the default violet
    float    line_width;    // stroke px; <= 0 inherits 1.5
    int32_t  line_visible;  // 0/1
    uint32_t ma_color;      // 0 inherits the default amber
    float    ma_width;
    uint32_t band_color;     // both dashed rules; 0 inherits the default gray
    int32_t  bands_visible;  // 0/1
} VroomRSI;

// Session VWAP, drawn as a single line on the price pane. The session resets
// each UTC day shifted by `reset_offset_min` minutes, and the line lifts its pen
// at each reset. Color 0 inherits the default cyan; a non-positive width
// inherits 1.5.
typedef struct VroomVWAP {
    int32_t  enabled;           // 0/1
    int32_t  reset_offset_min;  // session boundary offset from UTC midnight
    uint32_t color;             // 0xAARRGGBB; 0 inherits the default
    float    width;             // stroke px; <= 0 inherits 1.5
} VroomVWAP;

// Volume bars on the price pane, one per candle, drawn under the candles.
//
// `height_frac` is a ceiling, not a reservation: raising it lets the tallest bar
// reach further up over the candles rather than compressing them (matching the
// conventional volume overlay). Bar heights always auto-fit the loudest volume
// in view, so the tallest bar sits exactly at the ceiling.
//
// The style fields carry an inherit sentinel so the theme keeps supplying them
// when the consumer doesn't: a negative float or a fully transparent color falls
// back to the corresponding theme key.
typedef struct VroomVolume {
    int32_t  enabled;      // 0/1 — default 1: bars draw unless turned off
    float    height_frac;  // tallest bar as a fraction of the price pane; < 0 inherits 0.2
    float    opacity;      // 0..1 (1 = opaque); < 0 inherits VROOM_FLOAT_VOLUME_OPACITY
    float    radius_px;    // top-corner radius; < 0 inherits VROOM_FLOAT_VOLUME_RADIUS_PX
    uint32_t up_color;     // 0xAARRGGBB; 0 inherits VROOM_COLOR_ACCENT_BULL
    uint32_t down_color;   // 0xAARRGGBB; 0 inherits VROOM_COLOR_ACCENT_BEAR
} VroomVolume;

// A drawing anchor in data space (so a drawing tracks the candles on pan/zoom).
typedef struct VroomDrawPoint {
    int64_t time_ms;  // epoch milliseconds (not snapped to a candle slot)
    double  price;
} VroomDrawPoint;

// A committed drawing on the price pane. `kind` selects the geometry:
//   0 = line   — a two-point trendline from `a` to `b`.
//   1 = box    — an axis-aligned rectangle whose two opposite corners are `a`
//                and `b`; the other two corners (a.x,b.y) and (b.x,a.y) derive.
//   2 = pencil — a freehand path through `points` (in draw order). `a`/`b` mirror
//                the first/last point so bounds and handle code stay uniform.
//   3 = path   — straight segments through `points` (in draw order), ending in
//                an arrowhead on the last one. Same storage as a pencil, but
//                rendered unsmoothed and with every vertex a grab handle.
//
// `points`/`point_count` are only read for kinds 2 and 3; line and box leave
// them null/0. Like the rest of this API the points are copied internally, so
// the caller may free them as soon as the call returns.
typedef struct VroomDrawing {
    VroomDrawPoint        a;
    VroomDrawPoint        b;
    uint32_t              color;        // 0xAARRGGBB
    float                 width;        // stroke width in px
    int32_t               kind;         // 0 = line, 1 = box, 2 = pencil, 3 = path
    const VroomDrawPoint* points;       // pencil/path points (kind 2/3), else null
    int32_t               point_count;  // number of `points`, else 0
} VroomDrawing;

// A resting-liquidity band: a price interval carrying a total order size on one
// side of the book. Anchored in price space, so it scales with the y-axis.
typedef struct VroomBand {
    double  min_price;  // bottom of the interval
    double  max_price;  // top of the interval
    int32_t side;       // 0 = buy, 1 = sell (selects the color)
    double  volume;     // total resting size; drives the band's opacity
} VroomBand;

// Style shared by all liquidity bands. `buy_color`/`sell_color` are 0xAARRGGBB;
// the alpha is replaced per band by its volume mapped into
// [min_opacity, max_opacity]. `max_volume <= 0` auto-scales to the largest band.
// The leftward reach is min(width_px, width_frac * pane_width).
typedef struct VroomLiquidityStyle {
    uint32_t buy_color;
    uint32_t sell_color;
    double   max_volume;
    float    min_opacity;
    float    max_opacity;
    float    width_px;
    float    width_frac;
} VroomLiquidityStyle;

// ---- Price status lines ---------------------------------------------------

// Bit flags for VroomPriceLine::flags.
typedef enum {
    VROOM_PRICE_LINE_DRAGGABLE   = 1 << 0,  // the line can be dragged vertically
    VROOM_PRICE_LINE_CLOSABLE    = 1 << 1,  // render the trailing close ("x") button
    VROOM_PRICE_LINE_AXIS_LABEL  = 1 << 2,  // render the price badge in the y-axis strip
    VROOM_PRICE_LINE_EXTEND_LEFT = 1 << 3,  // extend the line to the pane's left edge
} VroomPriceLineFlags;

// A consumer-supplied horizontal status line at a fixed price — the primitive
// behind resting limit orders, take-profits, liquidation levels and the like.
//
// Visually: a line across the price pane ending in a label group made of a body
// pill (`text`), an optional solid-filled `quantity` pill, and an optional close
// button, plus an optional price badge in the y-axis strip.
//
// `text` / `quantity` are UTF-8 and copied internally, so the caller may free
// them as soon as the call returns. A null or empty string hides that segment.
typedef struct VroomPriceLine {
    double      price;
    uint32_t    color;       // 0xAARRGGBB — line, border, body text, close icon
    float       width;       // stroke width in px
    int32_t     line_style;  // 0 = solid, 1 = dotted, 2 = dashed
    const char* text;        // body label
    const char* quantity;    // trailing solid-fill segment
    int32_t     flags;       // bitwise-or of VroomPriceLineFlags
} VroomPriceLine;

// Layout/style shared by every price line, so the per-line struct stays small.
typedef struct VroomPriceLineStyle {
    uint32_t body_bg;           // translucent body/close-button pill fill (0xAARRGGBB)
    float    font_size_px;      // 0 = inherit VROOM_FLOAT_AXIS_FONT_SIZE_PX
    float    line_length_frac;  // 0..1 of pane width: the label group's right-edge inset
    int32_t  align;             // 0 = left, 1 = center, 2 = right
    float    hover_boost;       // brightness multiplier for the hovered segment (1 = flat)
} VroomPriceLineStyle;

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
    VROOM_COLOR_LINE,              // line-chart-mode close-price polyline
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
    VROOM_FLOAT_CANDLE_RADIUS_PX,        // candle body corner radius px (0 = square)
    VROOM_FLOAT_WICK_ROUND_CAP,          // 0/1: round the wick end caps
    VROOM_FLOAT_VOLUME_RADIUS_PX,        // volume bar top-corner radius px (0 = square)
    VROOM_FLOAT_LINE_WIDTH_PX,           // line-chart-mode polyline stroke width px
    VROOM_FLOAT_LINE_GRADIENT_OPACITY,   // fill under the line at its strongest (0 disables)
    VROOM_FLOAT_LINE_TENSION,            // 0..1 line-chart corner smoothing (0 = straight)
    VROOM_FLOAT_LINE_TIP_DOT,            // 0/1: dot at the line's newest end (default on)
    VROOM_FLOAT_LINE_TIP_PULSE,          // 0/1: expanding ring around that dot
    VROOM_FLOAT_COUNT_
} VroomFloatKey;

// ---- Animation ------------------------------------------------------------

// Easing curves for the animations the core paces itself. Mirrors the
// TransitionEasing union in @vroomchart/types, index for index.
//
// Most animations are eased by the host before it hands the core a progress
// value; this enum exists for the ones the core has to ease internally because
// each element runs on its own slice of the timeline.
typedef enum {
    VROOM_EASING_LINEAR = 0,
    VROOM_EASING_IN,
    VROOM_EASING_OUT,
    VROOM_EASING_IN_OUT,  // smoothstep; the default for unknown values
} VroomEasing;

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

// Target candle *body* width in px for the default/reset framing. Larger = more
// zoomed in (fewer candles), smaller = more zoomed out. 0 restores the legacy
// "most recent ~80 candles" default. Affects initial framing only; an explicit
// set_visible_range still overrides it.
void vroom_chart_set_default_candle_width(VroomChart* chart, float px);

// Chart render mode: 0 = candlesticks (default), 1 = line chart (a polyline
// through each candle's close). Other layers (volume, indicators, overlays,
// crosshair, drawings) are unaffected.
void vroom_chart_set_chart_type(VroomChart* chart, int32_t mode);

// Candle↔line morph blend for animated transitions. `collapse` folds candles
// toward their close price; `fade` crossfades candles→line. Both 0 = candles,
// both 1 = line. Driven per-frame by the host animation loop; set_chart_type
// snaps both to the target.
void vroom_chart_set_morph(VroomChart* chart, float collapse, float fade);

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

// Reads the visible price *envelope* — the min low / max high across the
// currently visible candles. This is the extent the candles actually occupy, not
// the axis range (which is wider: see vroom_chart_preserve_price_envelope).
// Returns false (out params untouched) when no candles are visible. Either out
// pointer may be null.
bool vroom_chart_get_visible_price_envelope(VroomChart* chart,
                                            double* out_low, double* out_high);

// "Scale lock" for a same-asset data swap that re-buckets the same price action
// into a different high-low span (a timeframe switch). Rescales a *manual* price
// range so the visible envelope keeps the exact pixel height and position that
// the [prev_low, prev_high] envelope had before the swap — so candles don't
// suddenly shrink or grow when the interval changes.
//
// Call after set_candles + set_visible_range, passing the envelope read by
// vroom_chart_get_visible_price_envelope *before* the swap.
//
// No-op in auto mode: auto-fit already widens the envelope by a fixed factor, so
// its pixel height is invariant. Falls back to reset_price_scale semantics when
// either envelope is degenerate.
void vroom_chart_preserve_price_envelope(VroomChart* chart,
                                         double prev_low, double prev_high);

// Captures the currently visible candle geometry so the next data swap can be
// animated as a reshape rather than a jump: each candle's wick and body slide
// and stretch into the shape of its counterpart in the new data.
//
// Candles are paired by *slot* — position counting back from the right edge of
// the visible window, which a timeframe switch preserves. Call before
// set_candles, then drive vroom_chart_set_interval_morph from 0 to 1.
// No-op when nothing is visible.
void vroom_chart_begin_interval_morph(VroomChart* chart);

// Advances the interval morph started by vroom_chart_begin_interval_morph. `t`
// (clamped to 0..1) is the eased progress: 0 renders the captured geometry
// pixel-identically to the pre-swap frame, 1 renders the new candles and
// releases the capture. Driven per-frame by the host animation loop.
void vroom_chart_set_interval_morph(VroomChart* chart, float t);

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

// Show the crosshair at a data-space position (epoch ms + price) rather than
// pixels — used to drive one chart's crosshair from another's (cross-chart
// sync). Converts to pixels via the current visible window and price scale;
// the vertical line then snaps to the nearest candle center at draw time, so
// the crosshair lands on the candle for `time_ms`. The horizontal line sits at
// `price` (clamped to the pane when off-scale).
void vroom_chart_set_crosshair_data(VroomChart* chart, int64_t time_ms, double price);

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
    // Free price under the crosshair's horizontal line — the value drawn on the
    // price badge (not snapped to any candle). Valid whenever the call returns
    // true, including in the future region where `has_candle` is false.
    double      price;
    bool        has_candle;
    VroomCandle candle;
} VroomCrosshairInfo;

// Fills *out with the snapped slot the crosshair currently sits on and returns
// true. Returns false (leaving *out untouched) when the crosshair is inactive
// or there are no visible candles. Stateless — recomputes the snap from the
// current crosshair x and visible window, matching what's rendered.
bool vroom_chart_get_crosshair_info(VroomChart* chart, VroomCrosshairInfo* out);

// ---- Indicators -----------------------------------------------------------

// Configures the RSI indicator (rendered in a pane below the candles). When
// enabled, the candle pane shrinks by VROOM_FLOAT_INDICATOR_HEIGHT_FRAC. Length
// and MA-kind changes recompute the series; band levels, colors, widths, and
// visibility changes only re-render.
void vroom_chart_set_rsi(VroomChart* chart, const VroomRSI* cfg);

// Configures the MACD indicator (its own pane below the candles). Panes stack
// in enable order, most recently enabled at the bottom. Length, source, and MA
// kind changes recompute the series; color, width, and visibility changes only
// re-render.
void vroom_chart_set_macd(VroomChart* chart, const VroomMACD* cfg);

// Replaces the full set of moving-average overlay lines (SMA/EMA) drawn on the
// price pane. Pass count 0 to clear them. Overlays don't reserve a pane.
void vroom_chart_set_overlays(VroomChart* chart, const VroomOverlay* overlays,
                              size_t count);

// Configures the session VWAP overlay (a single price-pane line). Enabled and
// reset-time changes recompute the series; color and width changes only
// re-render.
void vroom_chart_set_vwap(VroomChart* chart, const VroomVWAP* cfg);

// Configures the Bollinger Bands overlay (three price-pane lines + an optional
// translucent fill between the bands; no pane is reserved). Color/width/fill
// changes only re-render; enabled/period/mult/source/basis changes recompute.
void vroom_chart_set_bollinger(VroomChart* chart, const VroomBollinger* cfg);

// Configures the volume bars. Render-only — the bars come straight off each
// candle's volume, so nothing is recomputed. Bars are enabled by default; pass
// a config with `enabled` 0 to hide them.
void vroom_chart_set_volume(VroomChart* chart, const VroomVolume* cfg);

// Advances the staggered volume-bar collapse: 0 leaves every bar at full height,
// 1 has them all flat. Driven per-frame by the host animation loop.
//
// `t` is *linear* progress, unlike the other morph setters. Each bar falls over
// its own slice of the timeline — the tallest starting first and every bar
// landing at 1 — so `easing` (a VroomEasing) is applied per bar inside the core;
// pre-easing here would compound the two curves.
//
// Symmetric, so driving 1 -> 0 reveals the bars: the shortest rises first and
// the tallest arrives last. vroom_chart_set_volume snaps this to match its
// `enabled`, so the host only needs it while animating.
void vroom_chart_set_volume_collapse(VroomChart* chart, float t, int32_t easing);

// ---- Drawings (line annotations) ------------------------------------------

// Replaces the full set of committed line drawings (data-anchored, so they track
// the candles on pan/zoom). Pass count 0 to clear. Drawings render on the price
// pane above the candles/overlays and below the axis labels & crosshair.
void vroom_chart_set_drawings(VroomChart* chart, const VroomDrawing* drawings,
                              size_t count);

// Hit-tests pixel (x_px, y_px) against the committed drawings. On a hit, fills
// *out_index with the drawing index, *out_part with the sub-part that was hit,
// *out_t with the 0..1 grab position along the segment (A→B; 0/1 for handle
// hits, 0 for the shapes that have no meaningful t), and returns true. Handle
// hits are only reported for the currently selected drawing (whose handles are
// visible). Returns false on a miss (out params untouched). Any out pointer may
// be null.
//
// `out_part` is per-kind (see VroomDrawing.kind):
//   line   — 0 (endpoint A), 1 (endpoint B), 2 (body).
//   box    — 0..3 (corners, in the order a, (b.x,a.y), b, (a.x,b.y)), 4 (body).
//   pencil — 5 (body; a stroke has no handles).
//   path   — 6 (body), or VROOM_DRAW_PART_VERTEX + i for vertex i. The offset
//            keeps a vertex index from colliding with the small part numbers
//            above, and is why a path is capped at VROOM_PATH_MAX_POINTS.
#define VROOM_DRAW_PART_VERTEX 100
#define VROOM_PATH_MAX_POINTS  64
bool vroom_chart_hit_test_drawing(VroomChart* chart, float x_px, float y_px,
                                  int32_t* out_index, int32_t* out_part,
                                  float* out_t);

// Selects a committed drawing (renders its handles). `index` -1 clears the
// selection. `grabbed_endpoint` renders that handle 50% larger while it's being
// dragged (0/1 for a line, 0 for the box corner the host normalized to, the
// vertex index for a path); -1 for none. An out-of-range index clears the
// selection.
void vroom_chart_set_selected_drawing(VroomChart* chart, int32_t index,
                                      int32_t grabbed_endpoint);

// Moves one endpoint of a committed drawing to a new data-space anchor (for live
// handle dragging). `endpoint` is 0 (A) or 1 (B). No-op for an out-of-range index.
void vroom_chart_move_drawing_endpoint(VroomChart* chart, int32_t index,
                                       int32_t endpoint, int64_t time_ms,
                                       double price);

// Moves one vertex of a committed path (kind 3) to a new data-space anchor, for
// live handle dragging. `vertex` indexes `points`; a/b are re-mirrored onto the
// first/last point afterwards. No-op for an out-of-range drawing/vertex index or
// a drawing that isn't a path.
void vroom_chart_move_drawing_vertex(VroomChart* chart, int32_t index,
                                     int32_t vertex, int64_t time_ms,
                                     double price);

// Shifts a whole committed drawing by a *relative* data-space delta — `a`, `b`,
// and (for a pencil or path) every point. Used for live body dragging of shapes
// whose points can't be restated cheaply. No-op for an out-of-range index.
void vroom_chart_translate_drawing(VroomChart* chart, int32_t index,
                                   int64_t d_time_ms, double d_price);

// ---- Liquidity bands (order-book depth overlay) ---------------------------

// Replaces the full set of resting-liquidity bands and their shared style.
// Bands render behind the candles, anchored at the inner edge of the price axis
// and fading left. Pass count 0 to clear; `style` may be null when count is 0.
void vroom_chart_set_liquidity(VroomChart* chart, const VroomBand* bands,
                               size_t count, const VroomLiquidityStyle* style);

// ---- Price status lines ---------------------------------------------------

// Replaces the full set of price status lines and their shared style. Lines
// render on the price pane above the axis labels (so their badges cover any
// label they overlap) and below the crosshair. A line whose price maps outside
// the price pane is skipped rather than clamped. Pass count 0 to clear;
// `style` may be null when count is 0.
void vroom_chart_set_price_lines(VroomChart* chart, const VroomPriceLine* lines,
                                 size_t count,
                                 const VroomPriceLineStyle* style);

// Hit-tests pixel (x_px, y_px) against the price lines. On a hit, fills
// *out_index with the line index and *out_part with 0 (the line or its label
// body — the drag target) or 1 (the close button), and returns true. When
// several lines are within tolerance the nearest in y wins. Only draggable
// lines report part 0 and only closable lines report part 1. Returns false on a
// miss (out params untouched). Either out pointer may be null.
bool vroom_chart_hit_test_price_line(VroomChart* chart, float x_px, float y_px,
                                     int32_t* out_index, int32_t* out_part);

// Marks a price line's segment as hovered so it renders highlighted. `index` -1
// clears the hover. `part` matches vroom_chart_hit_test_price_line.
void vroom_chart_set_price_line_hover(VroomChart* chart, int32_t index,
                                      int32_t part);

// Drives the live drag preview: the line, its label and its badge render at
// `price` instead of the committed one, and a faint ghost marks where it
// started. `index` -1 ends the preview. The committed price is never mutated —
// the host applies (or rejects) the new price by restating its lines.
void vroom_chart_set_price_line_drag(VroomChart* chart, int32_t index,
                                     double price);

// Sets the transient in-progress "draft" the drawing tool shows while the user
// places points. Node A is always shown; when `has_b`, node B is shown too.
// `guide != 0` also draws the live preview (a guideline for a line, or a preview
// rectangle for a box); `guide == 0` draws node dots only (the committed shape
// already renders via set_drawings). `kind` matches VroomDrawing (0 = line,
// 1 = box) so the preview geometry matches the eventual shape. `color`/`width`
// style the preview to match the eventual drawing.
void vroom_chart_set_draft(VroomChart* chart, int64_t a_time, double a_price,
                           bool has_b, int64_t b_time, double b_price,
                           bool guide, uint32_t color, float width,
                           int32_t kind);

// Begins a freehand (pencil) draft stroke, clearing any previous draft points.
// Follow with vroom_chart_append_draft_point per captured sample; the growing
// path renders live. `color`/`width` style it to match the eventual stroke.
void vroom_chart_start_draft_stroke(VroomChart* chart, uint32_t color,
                                    float width);

// Appends one point to the in-progress freehand draft. Cheap (O(1) amortized) so
// it can be called on every pointer move without restating the whole path.
// No-op unless a draft stroke was started.
void vroom_chart_append_draft_point(VroomChart* chart, int64_t time_ms,
                                    double price);

// Restates the whole in-progress path (kind 3) draft: `count` vertices placed so
// far, plus — when `has_cursor` — a rubber-band segment from the last vertex to
// (cursor_time, cursor_price) carrying the arrow tip. Each placed vertex renders
// a node dot. A path is placed one deliberate click at a time (so `count` stays
// small, see VROOM_PATH_MAX_POINTS), which is why this restates rather than
// appending: the host stays the sole owner of the vertex list, and undoing a
// point is just a shorter array. Pass count 0 with has_cursor false to show
// nothing; the points are copied, so the caller may free them on return.
void vroom_chart_set_draft_path(VroomChart* chart, const VroomDrawPoint* points,
                                int32_t count, bool has_cursor,
                                int64_t cursor_time, double cursor_price,
                                uint32_t color, float width);

// Clears the draft (hides the in-progress node dots / guideline / stroke).
void vroom_chart_clear_draft(VroomChart* chart);

// Fills *out with the continuous data coordinate (time_ms, price) at pixel
// (x_px, y_px) using the free (non-snapped) mapping, and returns true. Returns
// false (leaving *out untouched) when there are no candles or the viewport is
// degenerate.
bool vroom_chart_coord_at(VroomChart* chart, float x_px, float y_px,
                          VroomCoord* out);

// Projects a data coordinate (time_ms, price) to its pixel position, filling
// *out_x / *out_y (either may be null) with the free (non-snapped) mapping — the
// inverse of vroom_chart_coord_at, matching the rendered drawing geometry.
// Returns false (out params untouched) when there are no candles / degenerate.
bool vroom_chart_project(VroomChart* chart, int64_t time_ms, double price,
                         float* out_x, float* out_y);

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
