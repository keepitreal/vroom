#import "VroomChartModule.h"

#import <ReactCommon/CallInvoker.h>
#import <jsi/jsi.h>

#include "VroomJsiInstaller.h"

using namespace facebook;

@implementation VroomChartModule {
  BOOL _didInstall;
}

RCT_EXPORT_MODULE(VroomChartModule)

// RCTTurboModuleManager calls this as soon as it instantiates the module —
// before the module object reaches JS — so global.VroomChartJSI always exists
// by the time anything imports NativeVroomChart.
//
// callInvoker is unused: every chart call is synchronous on the JS thread.
- (void)installJSIBindingsWithRuntime:(jsi::Runtime &)runtime
                          callInvoker:(const std::shared_ptr<react::CallInvoker> &)callInvoker
{
  vroom::installJsi(runtime);
  _didInstall = YES;
}

- (NSNumber *)install
{
  return @(_didInstall);
}

- (std::shared_ptr<react::TurboModule>)getTurboModule:(const react::ObjCTurboModule::InitParams &)params
{
  return std::make_shared<react::NativeVroomChartSpecJSI>(params);
}

@end
