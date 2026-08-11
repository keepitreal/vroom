#include "gradient.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkPoint.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
// See gradient.h: the two Skia versions we build against ship different
// linear-gradient headers, and neither ships both.
#if __has_include("include/effects/SkGradient.h")
#  include "include/core/SkSpan.h"
#  include "include/effects/SkGradient.h"
#  define VROOM_SK_MODERN_GRADIENT 1
#else
#  include "include/effects/SkGradientShader.h"
#  define VROOM_SK_MODERN_GRADIENT 0
#endif
#pragma clang diagnostic pop

namespace vroom {

sk_sp<SkShader> linear_alpha_ramp(const SkPoint pts[2],
                                  SkColor base,
                                  float a0,
                                  float a1) {
#if VROOM_SK_MODERN_GRADIENT
    const float r = SkColorGetR(base) / 255.f;
    const float g = SkColorGetG(base) / 255.f;
    const float b = SkColorGetB(base) / 255.f;
    const SkColor4f colors[2] = {SkColor4f{r, g, b, a0}, SkColor4f{r, g, b, a1}};
    const SkGradient::Colors grad_colors(
        SkSpan<const SkColor4f>(colors, 2), SkTileMode::kClamp);
    const SkGradient grad(grad_colors, SkGradient::Interpolation{});
    return SkShaders::LinearGradient(pts, grad);
#else
    const auto to_u8 = [](float a) {
        return static_cast<U8CPU>(a * 255.f + 0.5f);
    };
    const SkColor colors[2] = {SkColorSetA(base, to_u8(a0)),
                               SkColorSetA(base, to_u8(a1))};
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2,
                                        SkTileMode::kClamp);
#endif
}

}  // namespace vroom
