#pragma once

#include <jsi/jsi.h>

namespace vroom {

// Installs `global.VroomChartJSI` on the given runtime. Idempotent.
void installJsi(facebook::jsi::Runtime& runtime);

}  // namespace vroom
