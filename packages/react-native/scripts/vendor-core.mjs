// Vendor the C++ core into the package so it ships in the published tarball.
//
// In the monorepo the podspec mirrors ../core at pod-install time, but a
// published package has no sibling ../core — so we copy packages/core/{src,
// include} into cpp/_core_{src,include} at pack/publish time (prepack). The
// podspec falls back to these bundled copies when ../core is absent.

import { cpSync, existsSync, rmSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const here = dirname(fileURLToPath(import.meta.url));
const pkgDir = join(here, '..');
const coreRoot = join(pkgDir, '..', 'core');

if (!existsSync(coreRoot)) {
  console.log('vendor-core: ../core not found — assuming already vendored. Skipping.');
  process.exit(0);
}

for (const sub of ['src', 'include']) {
  const src = join(coreRoot, sub);
  const dst = join(pkgDir, 'cpp', `_core_${sub}`);
  rmSync(dst, { recursive: true, force: true });
  cpSync(src, dst, { recursive: true });
  console.log(`vendor-core: ${src} -> ${dst}`);
}
