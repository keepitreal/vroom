#import "VroomChartModule.h"

#import <React/RCTBridge+Private.h>
#import <ReactCommon/CallInvoker.h>
#import <jsi/jsi.h>

#include "VroomJsiInstaller.h"

using namespace facebook;

@implementation VroomChartModule {
  BOOL _installed;
}

RCT_EXPORT_MODULE(VroomChartModule)

#ifdef RCT_NEW_ARCH_ENABLED

#pragma mark - RCTTurboModuleWithJSIBindings

// Called automatically by the TurboModule infrastructure in both bridge and
// bridgeless modes. The runtime is passed directly, avoiding the need to access
// [RCTBridge currentBridge] which returns nil in bridgeless mode.
- (void)installJSIBindingsWithRuntime:(jsi::Runtime &)runtime
                          callInvoker:(const std::shared_ptr<react::CallInvoker> &)callInvoker
{
  if (_installed) return;
  vroom::installJsi(runtime);
  _installed = YES;
}

#pragma mark - TurboModule

- (std::shared_ptr<react::TurboModule>)getTurboModule:
    (const react::ObjCTurboModule::InitParams &)params
{
  return std::make_shared<react::NativeVroomChartSpecJSI>(params);
}

#endif

#pragma mark - JS Interface

// Called from JS via NativeVroomChart.install(). In new arch, JSI bindings are
// already installed via installJSIBindingsWithRuntime:callInvoker:, so this
// just returns the installation status. In old arch, it falls back to the
// bridge-based installation.
- (NSNumber *)install
{
#ifdef RCT_NEW_ARCH_ENABLED
  // In new arch, installation happens via installJSIBindingsWithRuntime.
  // This method is called from JS after bindings are already installed.
  return @(_installed);
#else
  // Legacy bridge fallback for old architecture
  if (_installed) return @YES;

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
  _installed = YES;
  return @YES;
#endif
}

@end
