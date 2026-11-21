// Regenerates content/docs/reference from the public types. Never hand-edit the
// generated files; platform-differences.mdx and meta.json are the only
// hand-maintained entries in that folder.
import { rm, writeFile } from 'node:fs/promises';
import { join } from 'node:path';
import { Application } from 'typedoc';

const root = new URL('..', import.meta.url).pathname;
const out = join(root, 'content/docs/reference');

for (const generated of ['functions', 'type-aliases', 'index.md']) {
  await rm(join(out, generated), { recursive: true, force: true });
}

const app = await Application.bootstrapWithPlugins({
  entryPoints: [join(root, '../../packages/react-native/src/index.ts')],
  tsconfig: join(root, '../../packages/react-native/tsconfig.json'),
  plugin: ['typedoc-plugin-markdown', join(root, 'scripts/typedoc-frontmatter.mjs')],
  out,
  cleanOutputDir: false,
  gitRevision: 'main',
  readme: 'none',
  excludeInternal: true,
  excludePrivate: true,
  entryFileName: 'index.md',
  hidePageTitle: true,
  hideBreadcrumbs: true,
  hidePageHeader: true,
  useCodeBlocks: true,
  expandObjects: true,
  indexFormat: 'table',
  parametersFormat: 'table',
  typeDeclarationFormat: 'table',
  locales: { en: { theme_defined_in: 'Source' } },
});

const project = await app.convert();
if (!project) throw new Error('TypeDoc conversion failed');
await app.generateOutputs(project);

await writeFile(join(out, 'type-aliases/meta.json'), `${JSON.stringify({ title: 'Type aliases' }, null, 2)}\n`);
await writeFile(join(out, 'functions/meta.json'), `${JSON.stringify({ title: 'Functions' }, null, 2)}\n`);
