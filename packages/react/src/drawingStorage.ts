// Versioned envelope for persisted drawings. The `DrawingStore` adapter is an
// opaque string key-value store; this module owns everything inside that string
// so the adapter never has to change as the schema evolves:
//
//   drawings ──serialize──▶  '{"v":1,"drawings":[…]}'  ──store.save──▶ bytes
//   bytes ──store.load──▶ string  ──deserialize (+migrate)──▶ drawings
//
// Evolving the schema without breaking clients:
//   • New drawing tool  → add its `type` to KNOWN_TYPES + the Drawing union (and
//     to FIXED_POINT_TYPES if it has exactly two anchors). Old payloads still
//     parse; payloads carrying an unknown tool (e.g. written by a newer version,
//     then read by an older one) are dropped, not crashed.
//   • New persisted field → add it to the envelope, bump DRAWINGS_VERSION, and
//     register a MIGRATIONS[oldVersion] that upgrades an old envelope to the new
//     shape. Old stored strings migrate transparently on the next load.
// The adapter interface (string in / string out) stays fixed through all of it.

import type { Drawing } from '@vroomchart/types';

/** Current envelope schema version. Bump when the persisted shape changes. */
export const DRAWINGS_VERSION = 1;

/** Drawing `type`s this build understands. Extend as tools are added. */
const KNOWN_TYPES = new Set<Drawing['type']>(['line', 'box', 'pencil']);

/** How many points each type must carry; pencil is variable (2 or more). */
const FIXED_POINT_TYPES = new Set(['line', 'box']);

function isPoint(p: unknown): boolean {
  if (p == null || typeof p !== 'object') return false;
  const { timeMs, price } = p as { timeMs?: unknown; price?: unknown };
  return (
    typeof timeMs === 'number' &&
    Number.isFinite(timeMs) &&
    typeof price === 'number' &&
    Number.isFinite(price)
  );
}

/**
 * Whether a parsed entry is a drawing this build can actually render. Checks the
 * `type` *and* the shape of `points` — the store is an opaque, consumer-supplied
 * blob, and since a pencil stroke is a variable-length array we can no longer
 * assume two well-formed anchors. Anything malformed is dropped exactly like an
 * unknown type, rather than reaching the renderer as garbage.
 */
export function isValidDrawing(d: unknown): d is Drawing {
  if (d == null || typeof d !== 'object') return false;
  const { type, points } = d as { type?: unknown; points?: unknown };
  if (typeof type !== 'string' || !KNOWN_TYPES.has(type as Drawing['type'])) {
    return false;
  }
  if (!Array.isArray(points)) return false;
  if (FIXED_POINT_TYPES.has(type) ? points.length !== 2 : points.length < 2) {
    return false;
  }
  return points.every(isPoint);
}

type Envelope = { v: number; drawings: Drawing[] };

/**
 * Migration map: `MIGRATIONS[n]` upgrades a version-`n` envelope to version
 * `n+1`. Applied in sequence until the payload reaches DRAWINGS_VERSION, so a
 * very old blob walks up one step at a time. Add an entry each time the schema
 * changes; never mutate the input.
 *
 * Example (when v2 arrives):
 *   [1]: (e) => ({ v: 2, drawings: e.drawings.map((d) => ({ ...d, opacity: 1 })) }),
 */
const MIGRATIONS: Record<number, (e: Envelope) => Envelope> = {};

/** Wrap the current drawings in a versioned envelope and stringify. */
export function serializeDrawings(drawings: Drawing[]): string {
  const env: Envelope = { v: DRAWINGS_VERSION, drawings };
  return JSON.stringify(env);
}

/**
 * Parse a stored string back into drawings, migrating older envelopes and
 * dropping anything malformed or of an unknown drawing `type`. Never throws —
 * returns `[]` for null/empty/garbage input.
 */
export function deserializeDrawings(raw: string | null | undefined): Drawing[] {
  if (raw == null || raw === '') return [];

  let parsed: unknown;
  try {
    parsed = JSON.parse(raw);
  } catch {
    return [];
  }

  // Accept a bare array as the pre-envelope v0 shape, so payloads written before
  // versioning (or by a hand-rolled adapter) still load and get upgraded.
  let env: Envelope;
  if (Array.isArray(parsed)) {
    env = { v: 0, drawings: parsed as Drawing[] };
  } else if (parsed != null && typeof parsed === 'object') {
    const obj = parsed as Partial<Envelope>;
    env = {
      v: typeof obj.v === 'number' ? obj.v : 0,
      drawings: Array.isArray(obj.drawings) ? obj.drawings : [],
    };
  } else {
    return [];
  }

  // Walk the migration chain up to the current version. Stop (keep what we have)
  // if a step is missing or the payload is somehow newer than we understand.
  let guard = 0;
  while (env.v < DRAWINGS_VERSION && guard++ < 100) {
    const up = MIGRATIONS[env.v];
    if (!up) break;
    env = up(env);
  }

  // Forward-compat: keep only drawings this build can render — a known `type`
  // carrying a well-formed `points` array. A newer file with an unknown tool (or
  // a corrupt entry) loads cleanly minus that entry, rather than throwing or
  // drawing garbage.
  return (Array.isArray(env.drawings) ? env.drawings : []).filter(isValidDrawing);
}
