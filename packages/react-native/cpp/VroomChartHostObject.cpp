#include "VroomChartHostObject.h"

#include <cstring>
#include <string>
#include <vector>

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
  out.reserve(29);
  out.push_back(jsi::PropNameID::forAscii(rt, "setCandles"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setSize"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setColor"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setFloat"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setVisibleRange"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setDefaultCandleWidth"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setChartType"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setMorph"));
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
  out.push_back(jsi::PropNameID::forAscii(rt, "getCrosshairInfo"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setRSI"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setMACD"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setOverlays"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setVWAP"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setBollinger"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setVolume"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setVolumeCollapse"));
  out.push_back(jsi::PropNameID::forAscii(rt, "coordAt"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setPriceLines"));
  out.push_back(jsi::PropNameID::forAscii(rt, "hitTestPriceLine"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setPriceLineHover"));
  out.push_back(jsi::PropNameID::forAscii(rt, "setPriceLineDrag"));
  out.push_back(jsi::PropNameID::forAscii(rt, "render"));
  return out;
}

#if defined(__ANDROID__)
// Android only: RN-Skia is compiled into its own librnskia.so, separate from
// libvroomchart.so. Constructing a RNSkia::JsiSkPicture directly here (as iOS
// does, below) gives it a vtable/typeinfo from *our* .so; when RN-Skia's own
// compiled code later consumes it (e.g. Convertor.h's
// `getPropertyValue<sk_sp<SkPicture>>`, which does
// `value.asObject(rt).asHostObject<JsiSkPicture>(rt)` — a dynamic_pointer_cast
// under the hood), the cast fails cross-.so with "Object is not a HostObject
// of desired type", because the object's *runtime* type was never actually
// compiled inside librnskia.so. iOS statically links everything into one
// binary, so no such mismatch exists there.
//
// The fix: build the picture through RN-Skia's own public JS API instead
// (`Skia.Picture.MakePicture(bytes)`, the same call `require(...).png`-style
// static pictures use), so the resulting JsiSkPicture is genuinely
// constructed by librnskia.so's own code. This costs a serialize +
// re-parse of the picture's draw ops per call — real overhead on a gesture
// hot path — but is the only ABI-safe option found so far. Revisit if
// profiling shows this mattering (e.g. a merged-.so build, or a lighter
// bridge that avoids the round trip).
#include "include/core/SkData.h"

namespace {
class SkDataMutableBuffer : public facebook::jsi::MutableBuffer {
 public:
  explicit SkDataMutableBuffer(sk_sp<SkData> data) : data_(std::move(data)) {}
  size_t size() const override { return data_->size(); }
  uint8_t* data() override {
    return const_cast<uint8_t*>(
        static_cast<const uint8_t*>(data_->data()));
  }

 private:
  sk_sp<SkData> data_;
};
}  // namespace

static facebook::jsi::Value wrapPicture(facebook::jsi::Runtime& rt,
                                       const sk_sp<SkPicture>& pic) {
  if (!pic) return facebook::jsi::Value::null();
  sk_sp<SkData> serialized = pic->serialize();
  if (!serialized) return facebook::jsi::Value::null();

  auto buffer = std::make_shared<SkDataMutableBuffer>(std::move(serialized));
  jsi::ArrayBuffer arrayBuffer(rt, buffer);

  auto skiaApi = rt.global().getProperty(rt, "SkiaApi");
  if (!skiaApi.isObject()) return facebook::jsi::Value::null();
  auto pictureFactory = skiaApi.asObject(rt).getProperty(rt, "Picture");
  if (!pictureFactory.isObject()) return facebook::jsi::Value::null();
  auto makePicture =
      pictureFactory.asObject(rt).getPropertyAsFunction(rt, "MakePicture");

  // Mirrors JsiSkPictureFactory::MakePicture's expected argument shape: an
  // object with a `.buffer` property holding the ArrayBuffer (matching a
  // Uint8Array-like value; the factory ignores everything else about it).
  jsi::Object arg(rt);
  arg.setProperty(rt, "buffer", arrayBuffer);
  return makePicture.call(rt, arg);
}
#else
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
#endif

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

  if (name == "setFloat") {
    // setFloat(keyIndex, value) — keyIndex is a VroomFloatKey (e.g. wick width).
    // The core bounds-checks the key and marks dirty.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "setFloat"),
        2,
        [this](jsi::Runtime& /*rt2*/,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 2) return jsi::Value::undefined();
          const int key = static_cast<int>(args[0].asNumber());
          vroom_chart_set_float(chart_, static_cast<VroomFloatKey>(key),
                                static_cast<float>(args[1].asNumber()));
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

  if (name == "setDefaultCandleWidth") {
    // setDefaultCandleWidth(px) — target candle body width for initial framing.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "setDefaultCandleWidth"),
        1,
        [this](jsi::Runtime& /*rt2*/,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 1) return jsi::Value::undefined();
          vroom_chart_set_default_candle_width(
              chart_, static_cast<float>(args[0].asNumber()));
          return jsi::Value::undefined();
        });
  }

  if (name == "setChartType") {
    // setChartType(mode) — 0 = candlesticks (default), 1 = line chart.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "setChartType"),
        1,
        [this](jsi::Runtime& /*rt2*/,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 1) return jsi::Value::undefined();
          vroom_chart_set_chart_type(
              chart_, static_cast<int32_t>(args[0].asNumber()));
          return jsi::Value::undefined();
        });
  }

  if (name == "setMorph") {
    // setMorph(collapse, fade) — candle↔line blend for animated transitions.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "setMorph"),
        2,
        [this](jsi::Runtime& /*rt2*/,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 2) return jsi::Value::undefined();
          vroom_chart_set_morph(chart_,
                                static_cast<float>(args[0].asNumber()),
                                static_cast<float>(args[1].asNumber()));
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

  if (name == "getCrosshairInfo") {
    // getCrosshairInfo() -> { timeMs, candle: {...} | null } | null. The slot
    // the crosshair snaps to (real candle or a future candle-aligned slot past
    // the last bar); null when the crosshair is inactive. `candle` is null in
    // the empty space ahead of the most recent candle. No rendering.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "getCrosshairInfo"),
        0,
        [this](jsi::Runtime& rt2,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* /*args*/,
               size_t /*count*/) -> jsi::Value {
          VroomCrosshairInfo info{};
          if (!vroom_chart_get_crosshair_info(chart_, &info)) {
            return jsi::Value::null();
          }
          jsi::Object obj(rt2);
          obj.setProperty(rt2, "timeMs", static_cast<double>(info.time_ms));
          if (info.has_candle) {
            jsi::Object c(rt2);
            c.setProperty(rt2, "timeMs",
                          static_cast<double>(info.candle.time_ms));
            c.setProperty(rt2, "open", info.candle.open);
            c.setProperty(rt2, "high", info.candle.high);
            c.setProperty(rt2, "low", info.candle.low);
            c.setProperty(rt2, "close", info.candle.close);
            c.setProperty(rt2, "volume", info.candle.volume);
            obj.setProperty(rt2, "candle", c);
          } else {
            obj.setProperty(rt2, "candle", jsi::Value::null());
          }
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
    // setMACD({enabled, fast, slow, signal, source, maKind, signalMaKind,
    // macdColor, macdWidth, macdVisible, signalColor, signalWidth,
    // signalVisible, histVisible, histUpColor, histUpFadingColor,
    // histDownColor, histDownFadingColor, zeroColor, zeroVisible}) — configures
    // the MACD pane. No render; the next render() picks it up.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "setMACD"),
        1,
        [this](jsi::Runtime& rt2,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 1 || !args[0].isObject()) return jsi::Value::undefined();
          auto s = args[0].asObject(rt2);
          VroomMACD cfg{};
          cfg.enabled = s.getProperty(rt2, "enabled").asBool() ? 1 : 0;
          cfg.fast = static_cast<int32_t>(
              s.getProperty(rt2, "fast").asNumber());
          cfg.slow = static_cast<int32_t>(
              s.getProperty(rt2, "slow").asNumber());
          cfg.signal = static_cast<int32_t>(
              s.getProperty(rt2, "signal").asNumber());
          cfg.source = static_cast<int32_t>(
              s.getProperty(rt2, "source").asNumber());
          cfg.ma_kind = static_cast<int32_t>(
              s.getProperty(rt2, "maKind").asNumber());
          cfg.signal_ma_kind = static_cast<int32_t>(
              s.getProperty(rt2, "signalMaKind").asNumber());
          cfg.macd_color = static_cast<uint32_t>(
              s.getProperty(rt2, "macdColor").asNumber());
          cfg.macd_width = static_cast<float>(
              s.getProperty(rt2, "macdWidth").asNumber());
          cfg.macd_visible =
              s.getProperty(rt2, "macdVisible").asBool() ? 1 : 0;
          cfg.signal_color = static_cast<uint32_t>(
              s.getProperty(rt2, "signalColor").asNumber());
          cfg.signal_width = static_cast<float>(
              s.getProperty(rt2, "signalWidth").asNumber());
          cfg.signal_visible =
              s.getProperty(rt2, "signalVisible").asBool() ? 1 : 0;
          cfg.hist_visible =
              s.getProperty(rt2, "histVisible").asBool() ? 1 : 0;
          cfg.hist_up_color = static_cast<uint32_t>(
              s.getProperty(rt2, "histUpColor").asNumber());
          cfg.hist_up_fading_color = static_cast<uint32_t>(
              s.getProperty(rt2, "histUpFadingColor").asNumber());
          cfg.hist_down_color = static_cast<uint32_t>(
              s.getProperty(rt2, "histDownColor").asNumber());
          cfg.hist_down_fading_color = static_cast<uint32_t>(
              s.getProperty(rt2, "histDownFadingColor").asNumber());
          cfg.zero_color = static_cast<uint32_t>(
              s.getProperty(rt2, "zeroColor").asNumber());
          cfg.zero_visible =
              s.getProperty(rt2, "zeroVisible").asBool() ? 1 : 0;
          vroom_chart_set_macd(chart_, &cfg);
          return jsi::Value::undefined();
        });
  }

  if (name == "setOverlays") {
    // setOverlays([{ kind, period, source, color, width }, ...]) — replaces the
    // full set of MA/EMA overlay lines. No render; the next render() picks it up.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "setOverlays"),
        1,
        [this](jsi::Runtime& rt2,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 1 || !args[0].isObject()) return jsi::Value::undefined();
          auto obj = args[0].asObject(rt2);
          if (!obj.isArray(rt2)) return jsi::Value::undefined();
          auto arr = obj.asArray(rt2);
          const size_t len = arr.size(rt2);
          std::vector<VroomOverlay> overlays;
          overlays.reserve(len);
          for (size_t i = 0; i < len; ++i) {
            auto o = arr.getValueAtIndex(rt2, i).asObject(rt2);
            VroomOverlay ov;
            ov.kind = static_cast<int32_t>(
                o.getProperty(rt2, "kind").asNumber());
            ov.period = static_cast<int32_t>(
                o.getProperty(rt2, "period").asNumber());
            ov.source = static_cast<int32_t>(
                o.getProperty(rt2, "source").asNumber());
            ov.color = static_cast<uint32_t>(
                o.getProperty(rt2, "color").asNumber());
            ov.width = static_cast<float>(
                o.getProperty(rt2, "width").asNumber());
            overlays.push_back(ov);
          }
          vroom_chart_set_overlays(chart_, overlays.data(), overlays.size());
          return jsi::Value::undefined();
        });
  }

  if (name == "setVWAP") {
    // setVWAP(enabled, resetOffsetMin, color, width) — session VWAP overlay.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "setVWAP"),
        4,
        [this](jsi::Runtime& /*rt2*/,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 4) return jsi::Value::undefined();
          const bool enabled = args[0].asBool();
          const int reset = static_cast<int>(args[1].asNumber());
          const uint32_t color = static_cast<uint32_t>(args[2].asNumber());
          const float width = static_cast<float>(args[3].asNumber());
          vroom_chart_set_vwap(chart_, enabled, reset, color, width);
          return jsi::Value::undefined();
        });
  }

  if (name == "setBollinger") {
    // setBollinger({enabled, period, mult, source, basisKind, upperColor,
    // upperWidth, middleColor, middleWidth, lowerColor, lowerWidth,
    // fillEnabled, fillOpacity}) — Bollinger Bands overlay. No render; the
    // next render() picks it up.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "setBollinger"),
        1,
        [this](jsi::Runtime& rt2,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 1 || !args[0].isObject()) return jsi::Value::undefined();
          auto s = args[0].asObject(rt2);
          VroomBollinger cfg{};
          cfg.enabled = s.getProperty(rt2, "enabled").asBool() ? 1 : 0;
          cfg.period = static_cast<int32_t>(
              s.getProperty(rt2, "period").asNumber());
          cfg.mult = static_cast<float>(
              s.getProperty(rt2, "mult").asNumber());
          cfg.source = static_cast<int32_t>(
              s.getProperty(rt2, "source").asNumber());
          cfg.basis_kind = static_cast<int32_t>(
              s.getProperty(rt2, "basisKind").asNumber());
          cfg.upper_color = static_cast<uint32_t>(
              s.getProperty(rt2, "upperColor").asNumber());
          cfg.upper_width = static_cast<float>(
              s.getProperty(rt2, "upperWidth").asNumber());
          cfg.middle_color = static_cast<uint32_t>(
              s.getProperty(rt2, "middleColor").asNumber());
          cfg.middle_width = static_cast<float>(
              s.getProperty(rt2, "middleWidth").asNumber());
          cfg.lower_color = static_cast<uint32_t>(
              s.getProperty(rt2, "lowerColor").asNumber());
          cfg.lower_width = static_cast<float>(
              s.getProperty(rt2, "lowerWidth").asNumber());
          cfg.fill_enabled =
              s.getProperty(rt2, "fillEnabled").asBool() ? 1 : 0;
          cfg.fill_opacity = static_cast<float>(
              s.getProperty(rt2, "fillOpacity").asNumber());
          vroom_chart_set_bollinger(chart_, &cfg);
          return jsi::Value::undefined();
        });
  }

  if (name == "setVolume") {
    // setVolume({enabled, heightFrac, opacity, radiusPx, upColor, downColor})
    // — the volume bars. Negative floats / zero colors inherit the theme. No
    // render; the next render() picks it up.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "setVolume"),
        1,
        [this](jsi::Runtime& rt2,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 1 || !args[0].isObject()) return jsi::Value::undefined();
          auto s = args[0].asObject(rt2);
          VroomVolume cfg{};
          cfg.enabled = s.getProperty(rt2, "enabled").asBool() ? 1 : 0;
          cfg.height_frac = static_cast<float>(
              s.getProperty(rt2, "heightFrac").asNumber());
          cfg.opacity = static_cast<float>(
              s.getProperty(rt2, "opacity").asNumber());
          cfg.radius_px = static_cast<float>(
              s.getProperty(rt2, "radiusPx").asNumber());
          cfg.up_color = static_cast<uint32_t>(
              s.getProperty(rt2, "upColor").asNumber());
          cfg.down_color = static_cast<uint32_t>(
              s.getProperty(rt2, "downColor").asNumber());
          vroom_chart_set_volume(chart_, &cfg);
          return jsi::Value::undefined();
        });
  }

  if (name == "setVolumeCollapse") {
    // setVolumeCollapse(t, easing) — staggered collapse of the volume bars. `t`
    // is LINEAR progress; the core eases each bar over its own window.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "setVolumeCollapse"),
        2,
        [this](jsi::Runtime& /*rt2*/,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 2) return jsi::Value::undefined();
          vroom_chart_set_volume_collapse(
              chart_,
              static_cast<float>(args[0].asNumber()),
              static_cast<int32_t>(args[1].asNumber()));
          return jsi::Value::undefined();
        });
  }

  if (name == "coordAt") {
    // coordAt(x, y) -> { timeMs, price } | null. The continuous data coordinate
    // at a pixel — not snapped to a candle slot. Null when there are no candles
    // or the viewport is degenerate. No rendering.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "coordAt"),
        2,
        [this](jsi::Runtime& rt2,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 2) return jsi::Value::null();
          VroomCoord c{};
          if (!vroom_chart_coord_at(chart_,
                                    static_cast<float>(args[0].asNumber()),
                                    static_cast<float>(args[1].asNumber()), &c)) {
            return jsi::Value::null();
          }
          jsi::Object obj(rt2);
          obj.setProperty(rt2, "timeMs", static_cast<double>(c.time_ms));
          obj.setProperty(rt2, "price", c.price);
          return obj;
        });
  }

  if (name == "setPriceLines") {
    // setPriceLines({ lines: [{ price, color, width, lineStyle, text, quantity,
    // flags }, ...], bodyBg, fontSizePx, lineLengthFrac, align, hoverBoost }) —
    // replaces the full set of price status lines. No render; the next render()
    // picks it up.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "setPriceLines"),
        1,
        [this](jsi::Runtime& rt2,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 1 || !args[0].isObject()) return jsi::Value::undefined();
          auto cfg = args[0].asObject(rt2);
          auto lines_val = cfg.getProperty(rt2, "lines");
          if (!lines_val.isObject()) return jsi::Value::undefined();
          auto lines_obj = lines_val.asObject(rt2);
          if (!lines_obj.isArray(rt2)) return jsi::Value::undefined();
          auto arr = lines_obj.asArray(rt2);
          const size_t len = arr.size(rt2);
          std::vector<VroomPriceLine> lines(len);
          // Label storage, kept alive until set_price_lines has copied it.
          std::vector<std::string> texts(len);
          std::vector<std::string> quantities(len);
          for (size_t i = 0; i < len; ++i) {
            auto l = arr.getValueAtIndex(rt2, i).asObject(rt2);
            lines[i].price = l.getProperty(rt2, "price").asNumber();
            lines[i].color = static_cast<uint32_t>(
                l.getProperty(rt2, "color").asNumber());
            lines[i].width = static_cast<float>(
                l.getProperty(rt2, "width").asNumber());
            lines[i].line_style = static_cast<int32_t>(
                l.getProperty(rt2, "lineStyle").asNumber());
            texts[i] = l.getProperty(rt2, "text").asString(rt2).utf8(rt2);
            quantities[i] =
                l.getProperty(rt2, "quantity").asString(rt2).utf8(rt2);
            lines[i].text = texts[i].c_str();
            lines[i].quantity = quantities[i].c_str();
            lines[i].flags = static_cast<int32_t>(
                l.getProperty(rt2, "flags").asNumber());
          }
          VroomPriceLineStyle style{};
          style.body_bg = static_cast<uint32_t>(
              cfg.getProperty(rt2, "bodyBg").asNumber());
          style.font_size_px = static_cast<float>(
              cfg.getProperty(rt2, "fontSizePx").asNumber());
          style.line_length_frac = static_cast<float>(
              cfg.getProperty(rt2, "lineLengthFrac").asNumber());
          style.align = static_cast<int32_t>(
              cfg.getProperty(rt2, "align").asNumber());
          style.hover_boost = static_cast<float>(
              cfg.getProperty(rt2, "hoverBoost").asNumber());
          vroom_chart_set_price_lines(chart_, lines.data(), lines.size(), &style);
          return jsi::Value::undefined();
        });
  }

  if (name == "hitTestPriceLine") {
    // hitTestPriceLine(x, y) -> { index, part } | null. `part` is 0 for the line
    // or its label body (the drag target) and 1 for the close button. Cheap
    // enough to call at gesture rate — no rendering.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "hitTestPriceLine"),
        2,
        [this](jsi::Runtime& rt2,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 2) return jsi::Value::null();
          int32_t index = -1, part = -1;
          if (!vroom_chart_hit_test_price_line(
                  chart_, static_cast<float>(args[0].asNumber()),
                  static_cast<float>(args[1].asNumber()), &index, &part)) {
            return jsi::Value::null();
          }
          jsi::Object obj(rt2);
          obj.setProperty(rt2, "index", index);
          obj.setProperty(rt2, "part", part);
          return obj;
        });
  }

  if (name == "setPriceLineHover") {
    // setPriceLineHover(index, part) — highlight a price line's segment; -1
    // clears. No hover on touch, so this exists for parity/pointer devices.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "setPriceLineHover"),
        2,
        [this](jsi::Runtime& /*rt2*/,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 2) return jsi::Value::undefined();
          vroom_chart_set_price_line_hover(
              chart_, static_cast<int32_t>(args[0].asNumber()),
              static_cast<int32_t>(args[1].asNumber()));
          return jsi::Value::undefined();
        });
  }

  if (name == "setPriceLineDrag") {
    // setPriceLineDrag(index, price) — live drag preview; index -1 ends it. The
    // committed price is untouched: restate setPriceLines to apply a move.
    return jsi::Function::createFromHostFunction(
        rt,
        jsi::PropNameID::forAscii(rt, "setPriceLineDrag"),
        2,
        [this](jsi::Runtime& /*rt2*/,
               const jsi::Value& /*thisVal*/,
               const jsi::Value* args,
               size_t count) -> jsi::Value {
          if (count < 2) return jsi::Value::undefined();
          vroom_chart_set_price_line_drag(
              chart_, static_cast<int32_t>(args[0].asNumber()),
              args[1].asNumber());
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
