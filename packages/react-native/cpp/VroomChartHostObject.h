// Per-chart JSI HostObject. One instance is created by
// VroomChartJSI.create() and held by the React component. When the JS object
// is garbage collected, this destructor runs and deletes the underlying
// VroomChart, freeing the candle buffer and any cached pictures.
//
// Methods exposed to JS:
//   setCandles(arrayBuffer)   — copies packed VroomCandle[] into the core
//   setSize(width, height, pxRatio)
//   render() -> SkPicture (iOS) / SkImage (Android)

#pragma once

#include <memory>

#include <jsi/jsi.h>

struct VroomChart;

namespace vroom {

class ChartHostObject : public facebook::jsi::HostObject {
 public:
  ChartHostObject();
  ~ChartHostObject() override;

  facebook::jsi::Value get(facebook::jsi::Runtime& runtime,
                           const facebook::jsi::PropNameID& name) override;

  std::vector<facebook::jsi::PropNameID> getPropertyNames(
      facebook::jsi::Runtime& runtime) override;

 private:
  VroomChart* chart_ = nullptr;
};

}  // namespace vroom
