// JNI entry point for VroomChartModule.install() (see
// ../java/com/vroom/chart/VroomChartModule.kt). This is the Android
// equivalent of ../../ios/VroomChartModule.mm's `-install` method: it
// reinterprets the JSI runtime pointer handed over from Java and calls the
// same platform-agnostic vroom::installJsi() the iOS bridge uses.

#include <jni.h>

#include <jsi/jsi.h>

#include "VroomJsiInstaller.h"

extern "C" JNIEXPORT jboolean JNICALL
Java_com_vroom_chart_VroomChartModule_nativeInstall(JNIEnv* /*env*/,
                                                      jobject /*thiz*/,
                                                      jlong runtimePointer) {
  auto* runtime =
      reinterpret_cast<facebook::jsi::Runtime*>(runtimePointer);
  if (runtime == nullptr) {
    return JNI_FALSE;
  }
  vroom::installJsi(*runtime);
  return JNI_TRUE;
}
