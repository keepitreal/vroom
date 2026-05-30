#import <Foundation/Foundation.h>
#import <React/RCTBridgeModule.h>

#ifdef RCT_NEW_ARCH_ENABLED
#import <VroomChartSpec/VroomChartSpec.h>
@interface VroomChartModule : NSObject <NativeVroomChartSpec>
#else
@interface VroomChartModule : NSObject <RCTBridgeModule>
#endif

@end
