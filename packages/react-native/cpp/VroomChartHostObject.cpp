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
  out.reserve(17);
  out.push_back(jsi::PropNameID::forAscii(rt, "setCandles"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setSize"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setColor"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setVisibleRange"));
  out.push_back(jsi::PropNameID::forAscii(rt, "pan"));
  out.push_back(jsi::PropNameID::forAscii(rt, "translate"));
  out.push_back(jsi::PropNameID::forAscii(rt, "zoom"));
  out.push_back(jsi::PropNameID::forAscii(rt, "scalePriceAxis"));
  out.push_back(jsi::PropNameID::forAscii(rt, "scaleTimeAxis"));
  out.push_back(jsi::PropNameID::forAscii(rt, "getAxisMetrics"));
  out.push_back(jsi::PropNameID::forAscii(rt, "isAnimating"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setCrosshair"));
  out.push_back(jsi::PropNameID::forAscii(rt, "clearCrosshair"));
  out.push_back(jsi::PropNameID::forAscii(rt, "getCrosshairCandle"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setRSI"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setMACD"));
  out.push_back(jsi::PropNameID::forAscii(rt, "render"));
  return out;
}

// Shared helper: wraps a fresh picture for return to JS, with memory pressure
// reported to Hermes so GC keeps up under gesture-rate churn.
static facebook::jsi::Value wrapPicture(facebook::jsi::Runtime& rt,
                                       const sk_sp<SkPicture>& pic) {
  if (!pic) return facebook::jsi::Value::null();
  auto host =
      std::make_shared<RNSkia::JsiSkPicture>(/*context=*/nullptr, pic);
  return JSI_CREATE_HOST_OBJECT_WITH_MEMORY_PRESSURE(rt, host,
                                                     /*context=*/nullptr);
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

  if (name == "setColor") {
    // setColor(keyIndex, argb) — keyIndex is a VroomColorKey; argb is a packed
    // 0xAARRGGBB integer. The core bounds-checks the key and marks dirty.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "setColor"),
        2,
        [this](jsi::Runtime& /*rt2*/,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 2) return jsi::Value::undefined();
          const int key = static_cast<int>(args[0].asNumber());
          const uint32_t argb =
              static_cast<uint32_t>(args[1].asNumber());
          vroom_chart_set_color(chart_, static_cast<VroomColorKey>(key), argb);
          return jsi::Value::undefined();
        });
  }

  if (name == "setVisibleRange") {
    // Pass 0,0 to clear (show all).
    // Timestamps are Number on the JS side — fine for ms-since-epoch since
    // those fit in 53 bits well past year 285,000.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "setVisibleRange"),
        2,
        [this](jsi::Runtime& /*rt2*/,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 2) return jsi::Value::undefined();
          const int64_t s = static_cast<int64_t>(args[0].asNumber());
          const int64_t e = static_cast<int64_t>(args[1].asNumber());
          vroom_chart_set_visible_range(chart_, s, e);
          return jsi::Value::undefined();
        });
  }

  if (name == "pan") {
    // pan(dx, dy) -> JsiSkPicture
    // Mutates the visible range and renders in one JSI call so gesture
    // handlers don't pay round-trip overhead twice per frame.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "pan"),
        2,
        [this](jsi::Runtime& rt2,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 2) return jsi::Value::null();
          const float dx = static_cast<float>(args[0].asNumber());
          const float dy = static_cast<float>(args[1].asNumber());
          vroom_chart_pan(chart_, dx, dy);
          return wrapPicture(rt2, render_chart_picture(chart_));
        });
  }

  if (name == "translate") {
    // translate(dx, dy) -> JsiSkPicture — shifts time + price bounds
    // without rescaling. Wired to the two-finger pan gesture in JS.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "translate"),
        2,
        [this](jsi::Runtime& rt2,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 2) return jsi::Value::null();
          const float dx = static_cast<float>(args[0].asNumber());
          const float dy = static_cast<float>(args[1].asNumber());
          vroom_chart_translate(chart_, dx, dy);
          return wrapPicture(rt2, render_chart_picture(chart_));
        });
  }

  if (name == "zoom") {
    // zoom(scaleX, scaleY, fx, fy) -> JsiSkPicture
    // Per-axis multiplicative factors since the last call (> 1 = zoom in).
    // scaleX scales the time window around fx; scaleY scales price around fy.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "zoom"),
        4,
        [this](jsi::Runtime& rt2,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 4) return jsi::Value::null();
          const float sx = static_cast<float>(args[0].asNumber());
          const float sy = static_cast<float>(args[1].asNumber());
          const float fx = static_cast<float>(args[2].asNumber());
          const float fy = static_cast<float>(args[3].asNumber());
          vroom_chart_zoom(chart_, sx, sy, fx, fy);
          return wrapPicture(rt2, render_chart_picture(chart_));
        });
  }

  if (name == "scalePriceAxis") {
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "scalePriceAxis"),
        1,
        [this](jsi::Runtime& rt2,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 1) return jsi::Value::null();
          const float dy = static_cast<float>(args[0].asNumber());
          vroom_chart_scale_price_axis(chart_, dy);
          return wrapPicture(rt2, render_chart_picture(chart_));
        });
  }

  if (name == "scaleTimeAxis") {
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "scaleTimeAxis"),
        1,
        [this](jsi::Runtime& rt2,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 1) return jsi::Value::null();
          const float dx = static_cast<float>(args[0].asNumber());
          vroom_chart_scale_time_axis(chart_, dx);
          return wrapPicture(rt2, render_chart_picture(chart_));
        });
  }

  if (name == "getAxisMetrics") {
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "getAxisMetrics"),
        0,
        [this](jsi::Runtime& rt2,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* /*args*/,
               size_t /*count*/) -> jsi::Value {
          float yw = 0.f, xh = 0.f, ih = 0.f;
          vroom_chart_get_axis_metrics(chart_, &yw, &xh, &ih);
          jsi::Object obj(rt2);
          obj.setProperty(rt2, "yAxisWidth", static_cast<double>(yw));
          obj.setProperty(rt2, "xAxisHeight", static_cast<double>(xh));
          obj.setProperty(rt2, "indicatorHeight", static_cast<double>(ih));
          return obj;
        });
  }

  if (name == "isAnimating") {
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "isAnimating"),
        0,
        [this](jsi::Runtime& /*rt2*/,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* /*args*/,
               size_t /*count*/) -> jsi::Value {
          return jsi::Value(is_animating(chart_));
        });
  }

  if (name == "setCrosshair") {
    // setCrosshair(x, y) -> JsiSkPicture. Activates the crosshair at (x, y) in
    // pixels (y already lifted above the touch on the JS side) and renders.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "setCrosshair"),
        2,
        [this](jsi::Runtime& rt2,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 2) return jsi::Value::null();
          const float x = static_cast<float>(args[0].asNumber());
          const float y = static_cast<float>(args[1].asNumber());
          vroom_chart_set_crosshair(chart_, x, y);
          return wrapPicture(rt2, render_chart_picture(chart_));
        });
  }

  if (name == "clearCrosshair") {
    // clearCrosshair() -> JsiSkPicture. Hides the crosshair and renders.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "clearCrosshair"),
        0,
        [this](jsi::Runtime& rt2,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* /*args*/,
               size_t /*count*/) -> jsi::Value {
          vroom_chart_clear_crosshair(chart_);
          return wrapPicture(rt2, render_chart_picture(chart_));
        });
  }

  if (name == "getCrosshairCandle") {
    // getCrosshairCandle() -> { timeMs, open, high, low, close, volume } | null.
    // The OHLCV of the candle the crosshair currently snaps to; null when the
    // crosshair is inactive. Cheap to call at gesture rate — no rendering.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "getCrosshairCandle"),
        0,
        [this](jsi::Runtime& rt2,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* /*args*/,
               size_t /*count*/) -> jsi::Value {
          VroomCandle c{};
          if (!vroom_chart_get_crosshair_candle(chart_, &c)) {
            return jsi::Value::null();
          }
          jsi::Object obj(rt2);
          // time_ms fits in 53 bits well past year 285,000, so Number is exact.
          obj.setProperty(rt2, "timeMs", static_cast<double>(c.time_ms));
          obj.setProperty(rt2, "open", c.open);
          obj.setProperty(rt2, "high", c.high);
          obj.setProperty(rt2, "low", c.low);
          obj.setProperty(rt2, "close", c.close);
          obj.setProperty(rt2, "volume", c.volume);
          return obj;
        });
  }

  if (name == "setRSI") {
    // setRSI(enabled, period, upperBand, lowerBand, maEnabled, maPeriod) —
    // configures the RSI pane. No render; the next render() picks it up.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "setRSI"),
        6,
        [this](jsi::Runtime& /*rt2*/,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 6) return jsi::Value::undefined();
          const bool enabled = args[0].asBool();
          const int period = static_cast<int>(args[1].asNumber());
          const double upper = args[2].asNumber();
          const double lower = args[3].asNumber();
          const bool ma_enabled = args[4].asBool();
          const int ma_period = static_cast<int>(args[5].asNumber());
          vroom_chart_set_rsi(chart_, enabled, period, upper, lower, ma_enabled,
                              ma_period);
          return jsi::Value::undefined();
        });
  }

  if (name == "setMACD") {
    // setMACD(enabled, fast, slow, signal) — configures the MACD pane. No
    // render; the next render() picks it up.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "setMACD"),
        4,
        [this](jsi::Runtime& /*rt2*/,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 4) return jsi::Value::undefined();
          const bool enabled = args[0].asBool();
          const int fast = static_cast<int>(args[1].asNumber());
          const int slow = static_cast<int>(args[2].asNumber());
          const int signal = static_cast<int>(args[3].asNumber());
          vroom_chart_set_macd(chart_, enabled, fast, slow, signal);
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
          return wrapPicture(rt2, render_chart_picture(chart_));
        });
  }

  return jsi::Value::undefined();
}

}  // namespace vroom
