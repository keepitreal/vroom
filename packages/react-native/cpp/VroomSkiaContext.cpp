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

  // `global.SkiaApi` is always installed by RN-Skia's own
  // RNSkManager::installBindings() as exactly a `JsiSkApi` host object — never
  // any other JsiSkHostObject subclass — so this cast is safe as a plain
  // static_cast, without any RTTI check. We deliberately avoid
  // dynamic_pointer_cast here: on Android, RN-Skia is compiled into its own
  // librnskia.so, separate from libvroomchart.so, so a `JsiSkApi` constructed
  // by librnskia.so's code carries typeinfo from that .so; a dynamic_cast
  // performed here (in libvroomchart.so) sees an unmerged RTTI record for
  // "the same" class and always fails, even though the object is perfectly
  // valid to use. (Same underlying issue as the JsiSkPicture cross-.so cast
  // documented in VroomChartHostObject.cpp — but there RN-Skia's own compiled
  // code does the cast on an object *we* construct, so we can't avoid it;
  // here *we* do the cast on an object *they* construct, so we can just skip
  // the runtime check we don't need.)
  auto* accessor = static_cast<HostObjectAccessor*>(host.get());
  return accessor->getContext();
}

}  // namespace vroom
