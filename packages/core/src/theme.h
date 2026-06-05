// Theme — colors and float-typed style parameters for a single chart.
//
// Layout: indexed arrays keyed by the public C-facade enums (VroomColorKey,
// VroomFloatKey) so we can stay ABI-stable while adding new keys. Wrapped in
// a struct so future expansion (booleans for visibility toggles, strings for
// label formats) lands as new sub-arrays without churning every field access.

#pragma once

#include "vroom/vroom_chart.h"

namespace vroom {

struct Theme {
    uint32_t colors[VROOM_COLOR_COUNT_]{};
    float    floats[VROOM_FLOAT_COUNT_]{};
};

// Returns a Theme pre-filled with the default dark palette + sensible
// layout defaults. Used by `VroomChart`'s constructor.
Theme default_theme();

}  // namespace vroom
