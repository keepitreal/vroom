#include "volume_anim.h"

#include <algorithm>

#include "vroom/vroom_chart.h"

namespace vroom::volume_anim {

float ease(int32_t kind, float p) {
    const float t = std::clamp(p, 0.f, 1.f);
    switch (kind) {
        case VROOM_EASING_LINEAR:
            return t;
        case VROOM_EASING_IN:
            return t * t;
        case VROOM_EASING_OUT:
            return t * (2.f - t);
        default:
            return t * t * (3.f - 2.f * t);  // smoothstep
    }
}

float bar_collapse(float frac, float t, int32_t easing) {
    const float progress = std::clamp(t, 0.f, 1.f);
    const float window = std::clamp(frac, kMinWindow, 1.f);
    // The window occupies the tail of the timeline, so a taller bar (bigger
    // window) opens earlier. Every window ends at 1.
    const float start = 1.f - window;
    return ease(easing, (progress - start) / window);
}

}  // namespace vroom::volume_anim
