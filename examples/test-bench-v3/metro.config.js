// Same Metro setup as ../test-bench (Expo + pnpm monorepo); see that file for why.

const { getDefaultConfig } = require('expo/metro-config');
const path = require('path');

const projectRoot = __dirname;
const workspaceRoot = path.resolve(projectRoot, '../..');

const config = getDefaultConfig(projectRoot);

config.watchFolders = [workspaceRoot];
config.resolver.nodeModulesPaths = [
  path.resolve(projectRoot, 'node_modules'),
  path.resolve(workspaceRoot, 'node_modules'),
];
// vroom's own devDependency pulls the v2 package into the workspace. Hide it so
// this app behaves like a v3-only consumer (src/skia.ts then resolves v3 only).
config.resolver.blockList = [].concat(config.resolver.blockList ?? [], /@shopify[+/]react-native-skia/);

config.resolver.unstable_enableSymlinks = true;
config.resolver.unstable_enablePackageExports = true;

module.exports = config;
