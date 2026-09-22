#import <Foundation/Foundation.h>
#import <React/RCTBridgeModule.h>

#ifdef RCT_NEW_ARCH_ENABLED
#import <VroomChartSpec/VroomChartSpec.h>
#import <ReactCommon/RCTTurboModuleWithJSIBindings.h>

@interface VroomChartModule : NSObject <NativeVroomChartSpec, RCTTurboModuleWithJSIBindings>
#else
@interface VroomChartModule : NSObject <RCTBridgeModule>
#endif

@end
