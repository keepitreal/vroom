#import "VroomChartModule.h"

#import <React/RCTBridge+Private.h>
#import <ReactCommon/CallInvoker.h>
#import <jsi/jsi.h>

#include "VroomJsiInstaller.h"

using namespace facebook;

@implementation VroomChartModule

RCT_EXPORT_MODULE(VroomChartModule)

// Called from JS via NativeVroomChart.install(). Grabs the JSI runtime from the
// bridge and asks the C++ installer to expose global.VroomChartJSI.
//
// The TurboModule version (new arch) and the legacy version both end up here.
- (NSNumber *)install
{
  RCTBridge *bridge = [RCTBridge currentBridge];
  RCTCxxBridge *cxxBridge = (RCTCxxBridge *)bridge;
  if (cxxBridge == nil) {
    return @NO;
  }

  jsi::Runtime *runtime = (jsi::Runtime *)cxxBridge.runtime;
  if (runtime == nullptr) {
    return @NO;
  }

  vroom::installJsi(*runtime);
  return @YES;
}

#ifdef RCT_NEW_ARCH_ENABLED
- (std::shared_ptr<facebook::react::TurboModule>)getTurboModule:
    (const facebook::react::ObjCTurboModule::InitParams &)params
{
  return std::make_shared<facebook::react::NativeVroomChartSpecJSI>(params);
}
#endif

@end
