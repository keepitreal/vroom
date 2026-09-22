#include <jni.h>

#include <fbjni/fbjni.h>
#include <jsi/jsi.h>
#include <ReactCommon/CallInvoker.h>
#include <react/jni/ReadableNativeMap.h>
#include <ReactCommon/BindingsInstallerHolder.h>

#include "VroomJsiInstaller.h"

using namespace facebook;

extern "C" JNIEXPORT jboolean JNICALL
Java_com_vroom_chart_VroomChartModule_nativeInstall(JNIEnv* /*env*/,
                                                      jobject /*thiz*/,
                                                      jlong runtimePointer) {
  auto* runtime = reinterpret_cast<jsi::Runtime*>(runtimePointer);
  if (runtime == nullptr) {
    return JNI_FALSE;
  }
  vroom::installJsi(*runtime);
  return JNI_TRUE;
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_vroom_chart_VroomChartModule_nativeCreateBindingsInstaller(
    JNIEnv* env,
    jobject thiz) {
  auto ref = react::BindingsInstallerHolder::newObjectCxxArgs(
      [finalThiz = jni::make_global(jni::adopt_local(jni::Environment::current()->NewLocalRef(thiz)))](
          jsi::Runtime& runtime,
          const std::shared_ptr<react::CallInvoker>& /*callInvoker*/) {
        vroom::installJsi(runtime);
        
        auto cls = jni::findClassStatic("com/vroom/chart/VroomChartModule");
        auto markInstalled = cls->getMethod<void()>("markInstalled");
        markInstalled(finalThiz);
      });
  return ref.release();
}
