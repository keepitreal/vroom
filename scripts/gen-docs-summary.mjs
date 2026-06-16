// Rewrites the auto-generated API-Reference block in docs/SUMMARY.md so the
// TypeDoc-generated pages under docs/reference/ show up in GitBook's nav.
//
// Only the lines between the BEGIN/END markers are touched; all hand-written
// sections of SUMMARY.md are left exactly as-is. Run via `pnpm docs:gen`
// (after TypeDoc has populated docs/reference/).

import { readdirSync, readFileSync, writeFileSync, existsSync } from 'node:fs';
import { join, relative, dirname, basename } from 'node:path';
import { fileURLToPath } from 'node:url';

const repoRoot = join(dirname(fileURLToPath(import.meta.url)), '..');
const docsDir = join(repoRoot, 'docs');
const refDir = join(docsDir, 'reference');
const summaryPath = join(docsDir, 'SUMMARY.md');

const BEGIN = '<!-- BEGIN:reference -->';
const END = '<!-- END:reference -->';

/** Collect every .md file under refDir, as paths relative to docs/. */
function collectMarkdown(dir) {
  const out = [];
  for (const entry of readdirSync(dir, { withFileTypes: true })) {
    const full = join(dir, entry.name);
    if (entry.isDirectory()) out.push(...collectMarkdown(full));
    else if (entry.name.endsWith('.md')) out.push(relative(docsDir, full));
  }
  return out;
}

if (!existsSync(refDir)) {
  console.error(`No generated reference at ${refDir}. Run TypeDoc first (pnpm --filter react-native-vroom-chart docs).`);
  process.exit(1);
}

const files = collectMarkdown(refDir);
// Always link with forward slashes so SUMMARY.md is portable across platforms.
const linkPath = (p) => p.split(/[\\/]/).join('/');
const title = (p) => basename(p, '.md');

const entry = files.find((f) => basename(f) === 'index.md');
const rest = files
  .filter((f) => f !== entry)
  .sort((a, b) => title(a).localeCompare(title(b)));

const lines = [];
if (entry) lines.push(`* [Overview](${linkPath(entry)})`);
for (const f of rest) lines.push(`* [${title(f)}](${linkPath(f)})`);

const block = `${BEGIN}\n${lines.join('\n')}\n${END}`;

const summary = readFileSync(summaryPath, 'utf8');
const beginIdx = summary.indexOf(BEGIN);
const endIdx = summary.indexOf(END);
if (beginIdx === -1 || endIdx === -1 || endIdx < beginIdx) {
  console.error(`Could not find ${BEGIN} … ${END} markers in ${summaryPath}.`);
  process.exit(1);
}

const next = summary.slice(0, beginIdx) + block + summary.slice(endIdx + END.length);
if (next !== summary) {
  writeFileSync(summaryPath, next);
  console.log(`Updated API-Reference block in SUMMARY.md (${rest.length + (entry ? 1 : 0)} pages).`);
} else {
  console.log('SUMMARY.md API-Reference block already up to date.');
}
