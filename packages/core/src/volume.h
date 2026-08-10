// Volume bars — one bottom-anchored bar per candle, centered on the candle and
// matching its body width, growing upward with the candle's volume. Drawn under
// the candles (candles z-above). Its own module alongside candles / labels so
// the chart orchestrator stays thin.

#pragma once

#include <cstddef>
#include <cstdint>

#include "vroom/vroom_chart.h"

class SkCanvas;

namespace vroom {
struct Layout;
struct Theme;
}  // namespace vroom

namespace vroom::volume {

// Draws a volume bar for each candle in [visible, visible + n). Bars share the
// candles' x/width (via candle_center_x / candle_body_width) so they scroll and
// resize with the candles. Heights auto-fit to the max volume in the slice, so
// the loudest bar lands on cfg.height_frac of the pane.
//
// `cfg` supplies height/opacity/radius/colors; each of its style fields falls
// back to `theme` when left on its inherit sentinel. Callers gate on
// cfg.enabled — this draws whenever it's called.
void draw(SkCanvas* canvas,
          const ::VroomCandle* visible,
          std::size_t n,
          const Layout& lay,
          const Theme& theme,
          const ::VroomVolume& cfg,
          int64_t window_ms,
          int64_t visible_start_ms,
          int64_t candle_duration_ms);

}  // namespace vroom::volume
