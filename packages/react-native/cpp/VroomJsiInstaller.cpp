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
#include "VroomSkiaContext.h"

#include "chart_internal.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkTypeface.h"
#pragma clang diagnostic pop

namespace vroom {

namespace jsi = facebook::jsi;

// Loads the system typeface once via RN-Skia's platform context and hands
// it to the chart core. Retries on each create() until it succeeds (in case
// SkiaApi isn't installed yet when our first chart instance is created).
static void ensureAxisTypeface(jsi::Runtime& runtime) {
  static bool done = false;
  if (done) return;
  auto ctx = vroom::getRNSkContext(runtime);
  if (!ctx) return;
  auto mgr = ctx->createFontMgr();
  if (!mgr) return;

  // A null family name is meant to request "the platform default" — CoreText
  // (iOS) honors that, but Android's SkFontMgr_New_Android does not: it
  // returns null unless given an actual family name, even though its
  // underlying font set (sans-serif, arial, ...) is perfectly populated. Fall
  // back to "sans-serif" (Android's standard generic-family alias) whenever
  // the null-family lookup comes back empty.
  auto tf = mgr->matchFamilyStyle(nullptr, SkFontStyle());
  if (!tf) {
    tf = mgr->matchFamilyStyle("sans-serif", SkFontStyle());
  }
  if (!tf) return;
  vroom::set_axis_typeface(tf);
  done = true;
}

void installJsi(jsi::Runtime& runtime) {
  auto create = jsi::Function::createFromHostFunction(
      runtime,
      jsi::PropNameID::forAscii(runtime, "create"),
      0,
      [](jsi::Runtime& rt,
         const jsi::Value& /*thisVal*/,
         const jsi::Value* /*args*/,
         size_t /*count*/) -> jsi::Value {
        ensureAxisTypeface(rt);
        auto host = std::make_shared<ChartHostObject>();
        return jsi::Object::createFromHostObject(rt, host);
      });

  jsi::Object api(runtime);
  api.setProperty(runtime, "create", std::move(create));

  runtime.global().setProperty(runtime, "VroomChartJSI", std::move(api));
}

}  // namespace vroom
