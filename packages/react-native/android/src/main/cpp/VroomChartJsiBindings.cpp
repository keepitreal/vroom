#include "VroomChartJsiBindings.h"

#include <memory>

#include <jsi/jsi.h>

#include "VroomJsiInstaller.h"

namespace vroom {

namespace jni = facebook::jni;
namespace jsi = facebook::jsi;
using facebook::react::BindingsInstallerHolder;
using facebook::react::CallInvoker;

void VroomChartJsiBindings::registerNatives() {
  // makeNativeMethod is a macro, so it cannot be namespace-qualified.
  javaClassLocal()->registerNatives({
      makeNativeMethod(
          "getBindingsInstaller",
          VroomChartJsiBindings::getBindingsInstaller),
  });
}

jni::local_ref<BindingsInstallerHolder::javaobject>
VroomChartJsiBindings::getBindingsInstaller(
    jni::alias_ref<VroomChartJsiBindings> /*jobj*/) {
  // callInvoker is unused: every chart call is synchronous on the JS thread.
  return BindingsInstallerHolder::newObjectCxxArgs(
      [](jsi::Runtime& runtime, const std::shared_ptr<CallInvoker>&) {
        installJsi(runtime);
      });
}

}  // namespace vroom
