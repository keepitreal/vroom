// Helpers for reaching into RN-Skia's installed JSI bindings to grab its
// RNSkPlatformContext — which we need so we can use JsiSkFontMgrFactory to
// get the system font manager cross-platform.
//
// JsiSkHostObject::getContext() is protected, so we use a "friend helper"
// subclass that exposes it via `using`. We never construct an instance —
// we just static_cast existing host objects to the accessor type.

#pragma once

#include <memory>

#include <jsi/jsi.h>

#include "JsiSkHostObjects.h"      // RNSkia::JsiSkHostObject
#include "RNSkPlatformContext.h"   // RNSkia::RNSkPlatformContext

namespace vroom {

class HostObjectAccessor : public RNSkia::JsiSkHostObject {
 public:
  using RNSkia::JsiSkHostObject::getContext;
};

// Returns the RN-Skia platform context by reading global.SkiaApi.
// Returns nullptr if SkiaApi isn't on the global yet (RN-Skia not installed).
std::shared_ptr<RNSkia::RNSkPlatformContext> getRNSkContext(
    facebook::jsi::Runtime& runtime);

}  // namespace vroom
