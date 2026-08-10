// Linear-gradient helper shared by the layers that fade a fill out.
//
// Skia's linear-gradient API differs across the Skia versions we build against:
// the WASM build uses a newer Skia (SkGradient.h + SkShaders::LinearGradient),
// while react-native-skia bundles an older one (SkGradientShader::MakeLinear).
// Neither ships both headers, so the implementation selects whichever is present
// and callers get one signature regardless of platform.

#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkColor.h"
#include "include/core/SkRefCnt.h"
#pragma clang diagnostic pop

struct SkPoint;
class SkShader;

namespace vroom {

// A gradient along pts[0] -> pts[1] holding `base`'s RGB while ramping alpha
// from `a0` to `a1`. RGB is held constant on purpose: interpolating toward a
// transparent black instead would darken the faded end into a grey smear.
// Clamps at both ends, so the shader can safely cover more than the fill does.
sk_sp<SkShader> linear_alpha_ramp(const SkPoint pts[2],
                                  SkColor base,
                                  float a0,
                                  float a1);

}  // namespace vroom
