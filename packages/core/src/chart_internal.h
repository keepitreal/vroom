// Internal C++ helpers — not part of the public C facade.
// Consumers: VroomPictureBridge / VroomChartHostObject in the RN package.

#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkRefCnt.h"
#pragma clang diagnostic pop

class SkPicture;
struct VroomChart;

namespace vroom {

// Returns the cached chart picture, rebuilding it if dirty. Caller must hold a
// ref or assign into something that does (e.g. sk_sp = ...).
sk_sp<SkPicture> render_chart_picture(VroomChart* chart);

}  // namespace vroom
