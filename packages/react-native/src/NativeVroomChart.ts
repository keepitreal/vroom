import type { TurboModule } from 'react-native';
import { TurboModuleRegistry } from 'react-native';

// TurboModule spec consumed by codegen. The native side installs the
// `global.VroomChartJSI` host object when React Native instantiates this
// module, not when `install` is called — see the JSI-bindings hooks in
// ../ios/VroomChartModule.mm and ../android/src/main/cpp/VroomChartJsiBindings.cpp.
// `install` remains so the spec has a method to generate and so JS has a way
// to force instantiation; all real chart operations go through the host
// object, not through the TurboModule itself.
export interface Spec extends TurboModule {
  install(): boolean;
}

export default TurboModuleRegistry.getEnforcing<Spec>('VroomChartModule');
