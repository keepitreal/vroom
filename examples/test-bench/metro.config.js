// Metro config for the vroom test bench (Expo + pnpm monorepo).
// Without this, Metro's default resolver can't follow pnpm's nested symlinks
// when packages inside node_modules/.pnpm/* require their peer deps.

const { getDefaultConfig } = require('expo/metro-config');
const path = require('path');

const projectRoot = __dirname;
const workspaceRoot = path.resolve(projectRoot, '../..');

const config = getDefaultConfig(projectRoot);

// Watch the whole monorepo so edits to packages/* hot-reload.
config.watchFolders = [workspaceRoot];

// Resolve modules from the test-bench AND from the workspace root.
config.resolver.nodeModulesPaths = [
  path.resolve(projectRoot, 'node_modules'),
  path.resolve(workspaceRoot, 'node_modules'),
];

// pnpm uses symlinks heavily; Metro needs both of these on.
config.resolver.unstable_enableSymlinks = true;
config.resolver.unstable_enablePackageExports = true;

module.exports = config;
