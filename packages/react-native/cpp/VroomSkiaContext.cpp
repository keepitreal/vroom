#include "VroomSkiaContext.h"

namespace vroom {

namespace jsi = facebook::jsi;

std::shared_ptr<RNSkia::RNSkPlatformContext> getRNSkContext(
    jsi::Runtime& runtime) {
  auto val = runtime.global().getProperty(runtime, "SkiaApi");
  if (!val.isObject()) return nullptr;
  auto obj = val.asObject(runtime);
  if (!obj.isHostObject(runtime)) return nullptr;
  auto host = obj.asHostObject(runtime);

  auto base = std::dynamic_pointer_cast<RNSkia::JsiSkHostObject>(host);
  if (!base) return nullptr;

  // The accessor class adds no fields; static_cast is safe even though the
  // runtime type is JsiSkApi (a different JsiSkHostObject subclass). We only
  // call the inherited non-virtual getContext() through this view.
  auto* accessor = static_cast<HostObjectAccessor*>(base.get());
  return accessor->getContext();
}

}  // namespace vroom
