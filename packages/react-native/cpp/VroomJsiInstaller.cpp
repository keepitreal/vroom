// Installs global.VroomChartJSI on the JSI runtime.
//
// API:
//   VroomChartJSI.create() -> ChartHostObject
//     methods on the returned object:
//       setCandles(arrayBuffer)
//       setSize(w, h, pxRatio)
//       render() -> JsiSkPicture
//
// The host object's lifetime is managed by JS GC — when the React component
// drops its reference, the underlying VroomChart is destroyed.

#include "VroomJsiInstaller.h"

#include <memory>

#include "VroomChartHostObject.h"

namespace vroom {

namespace jsi = facebook::jsi;

void installJsi(jsi::Runtime& runtime) {
  auto create = jsi::Function::createFromHostFunction(
      runtime,
      jsi::PropNameID::forAscii(runtime, "create"),
      0,
      [](jsi::Runtime& rt,
         const jsi::Value& /*thisVal*/,
         const jsi::Value* /*args*/,
         size_t /*count*/) -> jsi::Value {
        auto host = std::make_shared<ChartHostObject>();
        return jsi::Object::createFromHostObject(rt, host);
      });

  jsi::Object api(runtime);
  api.setProperty(runtime, "create", std::move(create));

  runtime.global().setProperty(runtime, "VroomChartJSI", std::move(api));
}

}  // namespace vroom
