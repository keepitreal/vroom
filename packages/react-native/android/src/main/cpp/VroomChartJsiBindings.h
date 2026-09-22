#pragma once

#include <ReactCommon/BindingsInstallerHolder.h>
#include <fbjni/fbjni.h>

namespace vroom {

// Binds com.vroom.chart.VroomChartModule.getBindingsInstaller() (see
// ../java/com/vroom/chart/VroomChartModule.kt) to C++. The holder it returns
// is invoked by TurboModuleManager with the JSI runtime, which is where
// vroom::installJsi() runs. This is the Android counterpart of iOS's
// -installJSIBindingsWithRuntime:callInvoker: in ../../ios/VroomChartModule.mm.
class VroomChartJsiBindings
    : public facebook::jni::JavaClass<VroomChartJsiBindings> {
 public:
  static constexpr const char* kJavaDescriptor =
      "Lcom/vroom/chart/VroomChartModule;";

  static void registerNatives();

 private:
  static facebook::jni::local_ref<
      facebook::react::BindingsInstallerHolder::javaobject>
  getBindingsInstaller(facebook::jni::alias_ref<VroomChartJsiBindings> jobj);
};

}  // namespace vroom
