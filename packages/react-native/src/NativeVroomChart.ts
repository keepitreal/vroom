import type { TurboModule } from 'react-native';
import { TurboModuleRegistry } from 'react-native';

// TurboModule spec consumed by codegen. The only method is `install`, which
// the native side uses to install the `global.VroomChartJSI` host object the
// first time it's called from JS. All real chart operations go through that
// host object, not through the TurboModule itself.
export interface Spec extends TurboModule {
  install(): boolean;
}

export default TurboModuleRegistry.getEnforcing<Spec>('VroomChartModule');
