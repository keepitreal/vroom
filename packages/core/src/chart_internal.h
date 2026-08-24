// Internal C++ helpers — not part of the public C facade.
// Consumers: VroomPictureBridge / VroomChartHostObject in the RN package.

#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkRefCnt.h"
#pragma clang diagnostic pop

class SkPicture;
class SkTypeface;
struct VroomChart;

namespace vroom {

// Returns the cached chart picture, rebuilding it if dirty. Caller must hold a
// ref or assign into something that does (e.g. sk_sp = ...).
sk_sp<SkPicture> render_chart_picture(VroomChart* chart);

// True if any axis label is mid-fade. JS uses this to know whether to keep
// re-rendering on a RAF loop until things settle.
bool is_animating(VroomChart* chart);

// Sets the typeface used for axis labels. Bridge layer calls this once at
// startup with a system typeface from the platform SkFontMgr. Labels are
// skipped if no typeface is set.
void set_axis_typeface(sk_sp<SkTypeface> typeface);

}  // namespace vroom
