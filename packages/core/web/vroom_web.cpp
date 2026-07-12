// vroom_web.cpp — the WebAssembly (A1c) wrapper around the C++ chart core.
//
// Builds ONLY under emscripten (see CMakeLists.txt `VROOM_WASM`). It links the
// platform-agnostic core (chart.cpp et al.) together with a Skia-for-WASM static
// lib, owns a GPU (WebGL) Skia surface bound to an HTML <canvas>, and paints it
// directly — so the web core renders pixel-identically to native without any
// SkPicture serialization. JS reaches this through embind (see the bindings at
// the bottom), adapted to the VroomChartHandle contract in @vroomchart/core-wasm.
//
// Surface attachment follows CanvasKit's MakeOnScreenGLSurface pattern.
//
// NOTE: Skia's GPU API namespaces were reorganized (the "Ganesh" move). The
// includes/calls below target a recent Skia (matching @shopify/react-native-skia
// 2.x). If the WASM build uses a different Skia revision, a few symbol names may
// need adjusting — they are called out inline.

#include <emscripten/bind.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgl.h>
#include <emscripten/val.h>

#include <GLES3/gl3.h>  // WebGL2 — defines GL_RGBA8 (GLES2 only has GL_RGBA8_OES)

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "vroom/vroom_chart.h"

// Skia — core
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypeface.h"

// Skia — Ganesh GL backend (recent layout)
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"

// FreeType-backed "empty" font manager so we can build typefaces from raw
// .ttf bytes (the WASM sandbox has no system font manager). This is what
// CanvasKit uses; requires Skia built with skia_enable_fontmgr_custom_empty.
#include "include/ports/SkFontMgr_empty.h"

// The core keeps the axis typeface as a process global (see fonts.cpp). It's set
// by the host; declared here to avoid pulling in the internal header.
namespace vroom {
void set_axis_typeface(sk_sp<SkTypeface> typeface);
}

namespace {

namespace em = emscripten;

class WebChart {
 public:
  explicit WebChart(std::string canvas_selector)
      : canvas_selector_(std::move(canvas_selector)) {
    chart_ = vroom_chart_create(nullptr, nullptr);
    initGL();
  }

  ~WebChart() {
    surface_.reset();
    gr_context_.reset();
    if (gl_) {
      emscripten_webgl_destroy_context(gl_);
      gl_ = 0;
    }
    if (chart_) {
      vroom_chart_destroy(chart_);
      chart_ = nullptr;
    }
  }

  // --- data -------------------------------------------------------------
  // `bytes` is a JS Uint8Array of packed VroomCandle structs (see packCandles).
  void setCandles(const em::val& bytes) {
    const size_t len = bytes["length"].as<size_t>();
    std::vector<uint8_t> buf(len);
    // Copy the JS typed array into our vector via a memory view over it.
    em::val memory_view{em::typed_memory_view(len, buf.data())};
    memory_view.call<void>("set", bytes);

    const size_t n = len / sizeof(VroomCandle);
    if (n == 0 || n * sizeof(VroomCandle) != len) return;
    vroom_chart_set_candles(
        chart_, reinterpret_cast<const VroomCandle*>(buf.data()), n);
  }

  // --- sizing -----------------------------------------------------------
  void setSize(float width, float height, float dpr) {
    width_css_ = width;
    height_css_ = height;
    dpr_ = dpr > 0.f ? dpr : 1.f;
    const int pw = static_cast<int>(width_css_ * dpr_);
    const int ph = static_cast<int>(height_css_ * dpr_);
    emscripten_set_canvas_element_size(canvas_selector_.c_str(), pw, ph);
    // The core works in CSS px and scales by px_ratio internally.
    vroom_chart_set_size(chart_, width_css_, height_css_, dpr_);
    surface_.reset();  // force a resized surface on next present()
  }

  // --- pass-through mutators -------------------------------------------
  void setColor(int key, uint32_t argb) {
    vroom_chart_set_color(chart_, static_cast<VroomColorKey>(key), argb);
  }
  void setVisibleRange(double start_ms, double end_ms) {
    vroom_chart_set_visible_range(chart_, static_cast<int64_t>(start_ms),
                                  static_cast<int64_t>(end_ms));
  }
  em::val getVisibleRange() {
    int64_t start_ms = 0, end_ms = 0;
    vroom_chart_get_visible_range(chart_, &start_ms, &end_ms);
    em::val o = em::val::object();
    // Epoch ms fits a double exactly (< 2^53).
    o.set("startMs", static_cast<double>(start_ms));
    o.set("endMs", static_cast<double>(end_ms));
    return o;
  }
  void resetView() { vroom_chart_reset_view(chart_); }
  void resetPriceScale() { vroom_chart_reset_price_scale(chart_); }
  void pan(float dx, float dy) { vroom_chart_pan(chart_, dx, dy); }
  void translate(float dx, float dy) { vroom_chart_translate(chart_, dx, dy); }
  void zoom(float sx, float sy, float fx, float fy) {
    vroom_chart_zoom(chart_, sx, sy, fx, fy);
  }
  void scalePriceAxis(float dy) { vroom_chart_scale_price_axis(chart_, dy); }
  void scaleTimeAxis(float dx) { vroom_chart_scale_time_axis(chart_, dx); }
  void resizeIndicatorPane(float dy) {
    vroom_chart_resize_indicator_pane(chart_, dy);
  }
  void scaleIndicatorAxis(float y, float dy) {
    vroom_chart_scale_indicator_axis(chart_, y, dy);
  }

  void setCrosshair(float x, float y) { vroom_chart_set_crosshair(chart_, x, y); }
  void setCrosshairData(double timeMs, double price) {
    vroom_chart_set_crosshair_data(chart_, static_cast<int64_t>(timeMs), price);
  }
  void clearCrosshair() { vroom_chart_clear_crosshair(chart_); }

  void setRSI(bool enabled, int period, double upper, double lower,
              bool ma_enabled, int ma_period) {
    vroom_chart_set_rsi(chart_, enabled, period, upper, lower, ma_enabled,
                        ma_period);
  }
  void setMACD(bool enabled, int fast, int slow, int signal) {
    vroom_chart_set_macd(chart_, enabled, fast, slow, signal);
  }
  void setVWAP(bool enabled, int reset_offset_min, uint32_t color, float width) {
    vroom_chart_set_vwap(chart_, enabled, reset_offset_min, color, width);
  }
  // `overlays` is a JS array of {kind, period, source, color, width}.
  void setOverlays(const em::val& overlays) {
    const size_t n = overlays["length"].as<size_t>();
    std::vector<VroomOverlay> out(n);
    for (size_t i = 0; i < n; ++i) {
      em::val o = overlays[i];
      out[i].kind = o["kind"].as<int32_t>();
      out[i].period = o["period"].as<int32_t>();
      out[i].source = o["source"].as<int32_t>();
      out[i].color = o["color"].as<uint32_t>();
      out[i].width = o["width"].as<float>();
    }
    vroom_chart_set_overlays(chart_, out.data(), n);
  }

  // `drawings` is a JS array of {aTime, aPrice, bTime, bPrice, color, width}.
  void setDrawings(const em::val& drawings) {
    const size_t n = drawings["length"].as<size_t>();
    std::vector<VroomDrawing> out(n);
    for (size_t i = 0; i < n; ++i) {
      em::val d = drawings[i];
      out[i].a.time_ms = static_cast<int64_t>(d["aTime"].as<double>());
      out[i].a.price = d["aPrice"].as<double>();
      out[i].b.time_ms = static_cast<int64_t>(d["bTime"].as<double>());
      out[i].b.price = d["bPrice"].as<double>();
      out[i].color = d["color"].as<uint32_t>();
      out[i].width = d["width"].as<float>();
    }
    vroom_chart_set_drawings(chart_, out.data(), n);
  }
  // `cfg` is a JS object { bands: [{minPrice, maxPrice, side, volume}, ...],
  // buyColor, sellColor, maxVolume, minOpacity, maxOpacity, widthPx, widthFrac }.
  void setLiquidity(const em::val& cfg) {
    em::val bands = cfg["bands"];
    const size_t n = bands["length"].as<size_t>();
    std::vector<VroomBand> out(n);
    for (size_t i = 0; i < n; ++i) {
      em::val b = bands[i];
      out[i].min_price = b["minPrice"].as<double>();
      out[i].max_price = b["maxPrice"].as<double>();
      out[i].side = b["side"].as<int32_t>();
      out[i].volume = b["volume"].as<double>();
    }
    VroomLiquidityStyle style{};
    style.buy_color = cfg["buyColor"].as<uint32_t>();
    style.sell_color = cfg["sellColor"].as<uint32_t>();
    style.max_volume = cfg["maxVolume"].as<double>();
    style.min_opacity = cfg["minOpacity"].as<float>();
    style.max_opacity = cfg["maxOpacity"].as<float>();
    style.width_px = cfg["widthPx"].as<float>();
    style.width_frac = cfg["widthFrac"].as<float>();
    vroom_chart_set_liquidity(chart_, out.data(), n, &style);
  }
  void setDraft(double a_time, double a_price, bool has_b, double b_time,
                double b_price, bool guide, uint32_t color, float width) {
    vroom_chart_set_draft(chart_, static_cast<int64_t>(a_time), a_price, has_b,
                          static_cast<int64_t>(b_time), b_price, guide, color,
                          width);
  }
  void clearDraft() { vroom_chart_clear_draft(chart_); }

  // --- reads ------------------------------------------------------------
  // Returns the continuous {timeMs, price} at pixel (x, y), or null.
  em::val coordAt(float x, float y) {
    VroomCoord c{};
    if (!vroom_chart_coord_at(chart_, x, y, &c)) return em::val::null();
    em::val o = em::val::object();
    o.set("timeMs", static_cast<double>(c.time_ms));
    o.set("price", c.price);
    return o;
  }

  em::val getAxisMetrics() {
    float yw = 0, xh = 0, ind = 0;
    vroom_chart_get_axis_metrics(chart_, &yw, &xh, &ind);
    em::val o = em::val::object();
    o.set("yAxisWidth", yw);
    o.set("xAxisHeight", xh);
    o.set("indicatorHeight", ind);
    return o;
  }

  // Returns a {timeMs, open, high, low, close, volume} object, or null.
  em::val getCrosshairCandle() {
    VroomCandle c{};
    if (!vroom_chart_get_crosshair_candle(chart_, &c)) return em::val::null();
    em::val o = em::val::object();
    o.set("timeMs", static_cast<double>(c.time_ms));
    o.set("open", c.open);
    o.set("high", c.high);
    o.set("low", c.low);
    o.set("close", c.close);
    o.set("volume", c.volume);
    return o;
  }

  // Returns { timeMs, candle: {timeMs, open, high, low, close, volume} | null },
  // or null when the crosshair is inactive. `candle` is null when the crosshair
  // sits on a future candle-aligned slot past the last bar.
  em::val getCrosshairInfo() {
    VroomCrosshairInfo info{};
    if (!vroom_chart_get_crosshair_info(chart_, &info)) return em::val::null();
    em::val o = em::val::object();
    o.set("timeMs", static_cast<double>(info.time_ms));
    o.set("price", info.price);
    if (info.has_candle) {
      em::val c = em::val::object();
      c.set("timeMs", static_cast<double>(info.candle.time_ms));
      c.set("open", info.candle.open);
      c.set("high", info.candle.high);
      c.set("low", info.candle.low);
      c.set("close", info.candle.close);
      c.set("volume", info.candle.volume);
      o.set("candle", c);
    } else {
      o.set("candle", em::val::null());
    }
    return o;
  }

  bool isAnimating() { return vroom_chart_is_animating(chart_); }

  // --- font -------------------------------------------------------------
  // `bytes` is a JS Uint8Array holding a .ttf/.otf file. Installs it as the
  // process-global axis typeface (shared by all charts; see fonts.cpp).
  void setTypeface(const em::val& bytes) {
    const size_t len = bytes["length"].as<size_t>();
    if (len == 0) return;
    auto data = SkData::MakeUninitialized(len);
    em::val view{em::typed_memory_view(len, static_cast<uint8_t*>(data->writable_data()))};
    view.call<void>("set", bytes);
    sk_sp<SkFontMgr> mgr = SkFontMgr_New_Custom_Empty();
    if (!mgr) return;
    sk_sp<SkTypeface> tf = mgr->makeFromData(data);
    if (tf) vroom::set_axis_typeface(std::move(tf));
  }

  // --- render -----------------------------------------------------------
  void present() {
    if (!chart_ || width_css_ <= 0 || height_css_ <= 0) return;
    emscripten_webgl_make_context_current(gl_);
    if (!ensureSurface()) return;
    SkCanvas* canvas = surface_->getCanvas();
    canvas->save();
    // The core draws in CSS px; scale up to the device-pixel framebuffer.
    canvas->scale(dpr_, dpr_);
    vroom_chart_draw(chart_, canvas);
    canvas->restore();
    if (gr_context_) gr_context_->flushAndSubmit();
  }

 private:
  void initGL() {
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.alpha = false;
    attrs.depth = false;
    attrs.stencil = true;     // Skia clips/paths want a stencil buffer
    attrs.antialias = false;  // Skia does its own AA
    attrs.majorVersion = 2;   // WebGL2
    attrs.premultipliedAlpha = true;
    gl_ = emscripten_webgl_create_context(canvas_selector_.c_str(), &attrs);
    if (gl_ <= 0) return;
    emscripten_webgl_make_context_current(gl_);
    // Native interface resolves WebGL entry points under emscripten (works
    // across Skia revisions; the GrGLInterfaces::MakeWebGL helper only exists in
    // newer trees).
    auto interface = GrGLMakeNativeInterface();
    gr_context_ = GrDirectContexts::MakeGL(interface);
  }

  // (Re)create the on-screen surface wrapping the canvas's default framebuffer.
  bool ensureSurface() {
    if (!gr_context_) return false;
    const int pw = static_cast<int>(width_css_ * dpr_);
    const int ph = static_cast<int>(height_css_ * dpr_);
    if (surface_ && surface_->width() == pw && surface_->height() == ph) {
      return true;
    }

    GrGLFramebufferInfo fb_info;
    fb_info.fFBOID = 0;            // default framebuffer (the canvas)
    fb_info.fFormat = GL_RGBA8;    // matches the requested context

    GrBackendRenderTarget target =
        GrBackendRenderTargets::MakeGL(pw, ph, /*sampleCnt=*/0,
                                       /*stencilBits=*/8, fb_info);
    SkSurfaceProps props;
    surface_ = SkSurfaces::WrapBackendRenderTarget(
        gr_context_.get(), target, kBottomLeft_GrSurfaceOrigin,
        kRGBA_8888_SkColorType, /*colorSpace=*/nullptr, &props);
    return surface_ != nullptr;
  }

  std::string canvas_selector_;
  VroomChart* chart_ = nullptr;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE gl_ = 0;
  sk_sp<GrDirectContext> gr_context_;
  sk_sp<SkSurface> surface_;
  float width_css_ = 0.f;
  float height_css_ = 0.f;
  float dpr_ = 1.f;
};

}  // namespace

EMSCRIPTEN_BINDINGS(vroom_web) {
  em::class_<WebChart>("WebChart")
      .constructor<std::string>()
      .function("setCandles", &WebChart::setCandles)
      .function("setSize", &WebChart::setSize)
      .function("setColor", &WebChart::setColor)
      .function("setVisibleRange", &WebChart::setVisibleRange)
      .function("getVisibleRange", &WebChart::getVisibleRange)
      .function("resetView", &WebChart::resetView)
      .function("resetPriceScale", &WebChart::resetPriceScale)
      .function("pan", &WebChart::pan)
      .function("translate", &WebChart::translate)
      .function("zoom", &WebChart::zoom)
      .function("scalePriceAxis", &WebChart::scalePriceAxis)
      .function("scaleTimeAxis", &WebChart::scaleTimeAxis)
      .function("resizeIndicatorPane", &WebChart::resizeIndicatorPane)
      .function("scaleIndicatorAxis", &WebChart::scaleIndicatorAxis)
      .function("getAxisMetrics", &WebChart::getAxisMetrics)
      .function("setCrosshair", &WebChart::setCrosshair)
      .function("setCrosshairData", &WebChart::setCrosshairData)
      .function("clearCrosshair", &WebChart::clearCrosshair)
      .function("getCrosshairCandle", &WebChart::getCrosshairCandle)
      .function("getCrosshairInfo", &WebChart::getCrosshairInfo)
      .function("setRSI", &WebChart::setRSI)
      .function("setMACD", &WebChart::setMACD)
      .function("setOverlays", &WebChart::setOverlays)
      .function("setVWAP", &WebChart::setVWAP)
      .function("setDrawings", &WebChart::setDrawings)
      .function("setLiquidity", &WebChart::setLiquidity)
      .function("setDraft", &WebChart::setDraft)
      .function("clearDraft", &WebChart::clearDraft)
      .function("coordAt", &WebChart::coordAt)
      .function("setTypeface", &WebChart::setTypeface)
      .function("isAnimating", &WebChart::isAnimating)
      .function("present", &WebChart::present);
}
