// Global typeface for axis/label text rendering.
//
// Loaded once at startup by the bridge layer (which uses RN-Skia's platform-
// context font manager to grab the system default). Held as a process-global
// so all charts share it; per-chart font configuration is not modeled yet.

#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkRefCnt.h"
#pragma clang diagnostic pop

class SkTypeface;

namespace vroom {

// Returns the currently-installed axis typeface, or nullptr if none has been
// set yet. Drawing code should null-check.
sk_sp<SkTypeface> axis_typeface();

// Replaces the axis typeface. Charts pick it up on their next draw.
void set_axis_typeface(sk_sp<SkTypeface> typeface);

}  // namespace vroom
