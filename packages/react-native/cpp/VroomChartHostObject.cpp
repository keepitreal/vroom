#include "VroomChartHostObject.h"

#include <cstring>

#include "chart_internal.h"
#include "vroom/vroom_chart.h"

// RN-Skia headers.
#include "JsiSkHostObjects.h"
#include "JsiSkPicture.h"

namespace vroom {

namespace jsi = facebook::jsi;

ChartHostObject::ChartHostObject() {
  chart_ = vroom_chart_create(nullptr, nullptr);
}

ChartHostObject::~ChartHostObject() {
  if (chart_) {
    vroom_chart_destroy(chart_);
    chart_ = nullptr;
  }
}

std::vector<jsi::PropNameID> ChartHostObject::getPropertyNames(
    jsi::Runtime& rt) {
  std::vector<jsi::PropNameID> out;
  out.reserve(3);
  out.push_back(jsi::PropNameID::forAscii(rt, "setCandles"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setSize"));
  out.push_back(jsi::PropNameID::forAscii(rt, "render"));
  return out;
}

jsi::Value ChartHostObject::get(jsi::Runtime& rt,
                                const jsi::PropNameID& propName) {
  const std::string name = propName.utf8(rt);

  if (name == "setCandles") {
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "setCandles"),
        1,
        [this](jsi::Runtime& rt2,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 1 || !args[0].isObject()) return jsi::Value::undefined();
          auto obj = args[0].asObject(rt2);
          if (!obj.isArrayBuffer(rt2)) return jsi::Value::undefined();
          auto buf = obj.getArrayBuffer(rt2);

          const size_t bytes = buf.size(rt2);
          const size_t n = bytes / sizeof(VroomCandle);
          if (n == 0 || n * sizeof(VroomCandle) != bytes) {
            return jsi::Value::undefined();
          }
          const auto* data =
              reinterpret_cast<const VroomCandle*>(buf.data(rt2));
          vroom_chart_set_candles(chart_, data, n);
          return jsi::Value::undefined();
        });
  }

  if (name == "setSize") {
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "setSize"),
        3,
        [this](jsi::Runtime& /*rt2*/,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 3) return jsi::Value::undefined();
          const float w = static_cast<float>(args[0].asNumber());
          const float h = static_cast<float>(args[1].asNumber());
          const float r = static_cast<float>(args[2].asNumber());
          vroom_chart_set_size(chart_, w, h, r);
          return jsi::Value::undefined();
        });
  }

  if (name == "render") {
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "render"),
        0,
        [this](jsi::Runtime& rt2,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* /*args*/,
               size_t /*count*/) -> jsi::Value {
          sk_sp<SkPicture> pic = render_chart_picture(chart_);
          if (!pic) return jsi::Value::null();

          auto host = std::make_shared<RNSkia::JsiSkPicture>(
              /*context=*/nullptr, pic);
          return JSI_CREATE_HOST_OBJECT_WITH_MEMORY_PRESSURE(
              rt2, host, /*context=*/nullptr);
        });
  }

  return jsi::Value::undefined();
}

}  // namespace vroom
