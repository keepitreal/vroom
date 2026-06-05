#include "fonts.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkTypeface.h"
#pragma clang diagnostic pop

#include <utility>

namespace vroom {

namespace {
sk_sp<SkTypeface> g_axis_typeface;
}  // namespace

sk_sp<SkTypeface> axis_typeface() { return g_axis_typeface; }

void set_axis_typeface(sk_sp<SkTypeface> typeface) {
    g_axis_typeface = std::move(typeface);
}

}  // namespace vroom
