// Resolution of the indicator style "inherit sentinels".
//
// Every indicator style field is optional at the public API, so the marshalled
// struct needs a value that means "the host didn't set this". A fully
// transparent color and a non-positive width are those sentinels: they can't
// name anything drawable, so they're free to carry the meaning. The renderer
// swaps them for the theme value or the built-in default at draw time, which
// keeps the fallback in one place instead of freezing it into every bridge.

#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkColor.h"
#pragma clang diagnostic pop

namespace vroom::style {

inline SkColor color_or(uint32_t configured, SkColor fallback) {
    return SkColorGetA(configured) == 0 ? fallback : configured;
}

// For an element whose default is another element's color dimmed — the easing
// MACD histogram bars, say.
inline SkColor faded_or(uint32_t configured, SkColor base, float alpha_frac) {
    if (SkColorGetA(configured) != 0) return configured;
    return SkColorSetA(base,
                       static_cast<U8CPU>(SkColorGetA(base) * alpha_frac));
}

inline float width_or(float configured, float fallback) {
    return configured > 0.f ? configured : fallback;
}

}  // namespace vroom::style
