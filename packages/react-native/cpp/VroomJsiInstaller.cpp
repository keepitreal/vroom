// Installs global.VroomChartJSI on the JSI runtime.
//
// API:
//   VroomChartJSI.create() -> ChartHostObject
//     methods on the returned object:
//       setCandles(arrayBuffer)
//       setSize(w, h, pxRatio)
//       render() -> SkPicture (iOS) / SkImage (Android)
//
// The host object's lifetime is managed by JS GC — when the React component
// drops its reference, the underlying VroomChart is destroyed.

#include "VroomJsiInstaller.h"

#include <memory>

#include "VroomChartHostObject.h"
#include "VroomFontMgr.h"

#include "chart_internal.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkTypeface.h"
#pragma clang diagnostic pop

#if defined(__ANDROID__)
#include <android/log.h>
#define VROOM_WARN(...) \
  __android_log_print(ANDROID_LOG_WARN, "VroomChart", __VA_ARGS__)
#elif defined(__APPLE__)
#include <os/log.h>
#define VROOM_WARN(fmt, ...) \
  os_log_error(OS_LOG_DEFAULT, "VroomChart: " fmt, ##__VA_ARGS__)
#else
#define VROOM_WARN(...)
#endif

namespace vroom {

namespace jsi = facebook::jsi;

// Loads the system typeface once via the platform SkFontMgr and hands it to
// the chart core. Built only once — constructing a font manager enumerates
// the system font set and is not cheap. Failure is logged once; we do not
// retry, because this no longer depends on RN-Skia's JSI bindings appearing.
static void ensureAxisTypeface() {
  static bool done = false;
  if (done) return;
  done = true;

  auto mgr = vroom::makePlatformFontMgr();
  if (!mgr) {
    VROOM_WARN("axis typeface: platform font manager unavailable");
    return;
  }

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
  if (!tf) {
    VROOM_WARN("axis typeface: no default or sans-serif typeface");
    return;
  }
  vroom::set_axis_typeface(tf);
}

void installJsi(jsi::Runtime& runtime) {
  ensureAxisTypeface();

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
