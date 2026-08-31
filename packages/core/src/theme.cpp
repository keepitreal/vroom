#include "theme.h"

namespace vroom {

namespace {

// GitHub-dark palette.
constexpr uint32_t kDefaultColors[VROOM_COLOR_COUNT_] = {
    0xff0d1117,  // BACKGROUND
    0xff26a69a,  // BULL
    0xffef5350,  // BEAR
    0xffaaaaaa,  // WICK
    0xff1a1e24,  // GRID — ~20% darker than the crosshair so it recedes
    0xffc9d1d9,  // AXIS_TEXT
    0xff303741,  // CROSSHAIR — grid tone lightened ~44%; reads above GRID
    0x00000000,  // RESERVED_7_ — retired slot, never read
    0xffffffff,  // BADGE_TEXT — text on filled badges/pills
    0xff3e4855,  // CROSSHAIR_TARGET — CROSSHAIR lightened 30% (prior derived ring)
    0x00000000,  // BORDER_BULL — transparent sentinel: inherit BULL fill
    0x00000000,  // BORDER_BEAR — transparent sentinel: inherit BEAR fill
    0x00000000,  // WICK_BULL   — transparent sentinel: inherit BULL fill
    0x00000000,  // WICK_BEAR   — transparent sentinel: inherit BEAR fill
    0xff26a69a,  // ACCENT_BULL — classic teal-green (price indicator, volume, MACD)
    0xffef5350,  // ACCENT_BEAR — classic red
    0xff8957e5,  // LINE — line-chart close polyline; violet, matching the RSI line
};

constexpr float kDefaultFloats[VROOM_FLOAT_COUNT_] = {
    0.68f,  // CANDLE_WIDTH_RATIO — body fills 68% of slot, 32% gap
    1.0f,   // WICK_WIDTH_PX
    6.f,    // RIGHT_PADDING_PX — small gutter between candles and y-axis
    11.f,   // AXIS_FONT_SIZE_PX
    0.18f,  // Y_AXIS_WIDTH_RATIO — fallback before font is loaded
    22.f,   // X_AXIS_HEIGHT_PX — bottom strip for time labels
    0.5f,   // VOLUME_OPACITY — candle color at 50% transparency
    0.20f,  // INDICATOR_HEIGHT_FRAC — below-chart indicator pane = 20% of height
    0.f,    // CANDLE_RADIUS_PX — square by default
    0.f,    // WICK_ROUND_CAP — butt caps by default
    0.f,    // VOLUME_RADIUS_PX — square by default
    1.5f,   // LINE_WIDTH_PX — line-chart polyline stroke width
    0.28f,  // LINE_GRADIENT_OPACITY — fill under the line, ramped to 0 at the pane bottom
    0.f,    // LINE_TENSION — straight segments by default
    1.f,    // LINE_TIP_DOT — the line's leading end is marked by default
    0.f,    // LINE_TIP_PULSE — the ring is opt-in; it never lets the chart idle
};

}  // namespace

Theme default_theme() {
    Theme t;
    for (int i = 0; i < VROOM_COLOR_COUNT_; ++i) t.colors[i] = kDefaultColors[i];
    for (int i = 0; i < VROOM_FLOAT_COUNT_; ++i) t.floats[i] = kDefaultFloats[i];
    return t;
}

}  // namespace vroom
