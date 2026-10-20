import { registerRootComponent } from 'expo';

// Same app as ../test-bench; only the Skia dependency differs (v3 here, v2 there).
import App from '../test-bench/App';

registerRootComponent(App);
