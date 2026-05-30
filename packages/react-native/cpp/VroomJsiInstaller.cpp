// Installs global.VroomChartJSI on the JSI runtime. Phase 0: one method,
// helloPicture(), which builds a tiny SkPicture in C++ and returns it wrapped
// as a JsiSkPicture host object that RN-Skia's <Picture> component can render.

#include "VroomJsiInstaller.h"

#include <memory>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRect.h"
#pragma clang diagnostic pop

// RN-Skia headers (resolved via the podspec's HEADER_SEARCH_PATHS pointing at
// node_modules/@shopify/react-native-skia/cpp).
#include "JsiSkHostObjects.h"
#include "JsiSkPicture.h"

namespace vroom {

namespace jsi = facebook::jsi;

namespace {

// Records a 100x100 red rectangle into a new SkPicture and returns it. The
// rectangle is the de-risking marker for Phase 0 — if we see it on screen, the
// whole C++ → JsiSkPicture → SharedValue → <Picture> chain works.
sk_sp<SkPicture> buildHelloPicture() {
  SkPictureRecorder recorder;
  SkCanvas* canvas = recorder.beginRecording(SkRect::MakeWH(100.f, 100.f));

  SkPaint paint;
  paint.setColor(SK_ColorRED);
  paint.setAntiAlias(true);
  canvas->drawRect(SkRect::MakeXYWH(0.f, 0.f, 100.f, 100.f), paint);

  return recorder.finishRecordingAsPicture();
}

}  // namespace

void installJsi(jsi::Runtime& runtime) {
  // helloPicture(): () -> JsiSkPicture
  auto hello = jsi::Function::createFromHostFunction(
      runtime,
      jsi::PropNameID::forAscii(runtime, "helloPicture"),
      0,
      [](jsi::Runtime& rt,
         const jsi::Value& /*thisVal*/,
         const jsi::Value* /*args*/,
         size_t /*count*/) -> jsi::Value {
        sk_sp<SkPicture> picture = buildHelloPicture();

        // nullptr platform context is safe here: ViewProperty::getPicture()
        // only reads the underlying sk_sp via getObject(), and the only API
        // that touches the context (makeShader) isn't called by <Picture>.
        auto host = std::make_shared<RNSkia::JsiSkPicture>(
            /*context=*/nullptr, picture);

        // The MEMORY_PRESSURE macro reports SkPicture::approximateBytesUsed()
        // to Hermes so the GC accounts for the native allocation. Critical
        // once we start churning pictures every frame in later phases; cheap
        // to include now.
        return JSI_CREATE_HOST_OBJECT_WITH_MEMORY_PRESSURE(
            rt, host, /*context=*/nullptr);
      });

  jsi::Object api(runtime);
  api.setProperty(runtime, "helloPicture", std::move(hello));

  runtime.global().setProperty(runtime, "VroomChartJSI", std::move(api));
}

}  // namespace vroom
