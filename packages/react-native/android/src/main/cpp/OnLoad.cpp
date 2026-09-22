#include <jni.h>

#include <fbjni/fbjni.h>

#include "VroomChartJsiBindings.h"

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* /*reserved*/) {
  return facebook::jni::initialize(
      vm, [] { vroom::VroomChartJsiBindings::registerNatives(); });
}
