#import <Foundation/Foundation.h>
#import <React/RCTBridgeModule.h>
#import <ReactCommon/RCTTurboModuleWithJSIBindings.h>
#import <VroomChartSpec/VroomChartSpec.h>

@interface VroomChartModule : NSObject <NativeVroomChartSpec, RCTTurboModuleWithJSIBindings>

@end
