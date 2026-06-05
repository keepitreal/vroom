#include "theme.h"

namespace vroom {

namespace {

// GitHub-dark palette.
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
    0.55f,  // CANDLE_WIDTH_RATIO — body fills 55% of slot, 45% gap
    1.0f,   // WICK_WIDTH_PX
    6.f,    // RIGHT_PADDING_PX — small gutter between candles and y-axis
    11.f,   // AXIS_FONT_SIZE_PX
    0.18f,  // Y_AXIS_WIDTH_RATIO — fallback before font is loaded
    22.f,   // X_AXIS_HEIGHT_PX — bottom strip for time labels
};

}  // namespace

Theme default_theme() {
    Theme t;
    for (int i = 0; i < VROOM_COLOR_COUNT_; ++i) t.colors[i] = kDefaultColors[i];
    for (int i = 0; i < VROOM_FLOAT_COUNT_; ++i) t.floats[i] = kDefaultFloats[i];
    return t;
}

}  // namespace vroom
