// Ramer–Douglas–Peucker polyline simplification, used to thin a freehand pencil
// stroke before it is committed and persisted.
//
// A drag captures a sample every few pixels, so a single stroke can arrive with
// hundreds of points — most of them redundant along near-straight runs. RDP
// keeps only the points that actually carry the shape: it finds the sample
// farthest from the chord between the current endpoints and, if that distance
// exceeds `epsilon`, keeps it and recurses on both halves; otherwise the whole
// span collapses to its endpoints.
//
// We run this in *pixel* space (not data space) so `epsilon` means "visible
// deviation on screen" regardless of the chart's zoom or price scale, then map
// the surviving indices back to the captured data-space points.

export type Pt = { x: number; y: number };

/** Perpendicular distance from `p` to the (infinite) line through `a` and `b`. */
function perpendicularDistance(p: Pt, a: Pt, b: Pt): number {
  const dx = b.x - a.x;
  const dy = b.y - a.y;
  // Degenerate chord (a === b): fall back to plain point distance.
  if (dx === 0 && dy === 0) return Math.hypot(p.x - a.x, p.y - a.y);
  // |cross product| / |chord| — the triangle-area formulation, no division by
  // zero and no need to normalize the direction vector first.
  return Math.abs(dy * p.x - dx * p.y + b.x * a.y - b.y * a.x) / Math.hypot(dx, dy);
}

/**
 * Indices of the points to keep, in ascending order, always including the first
 * and last. `epsilon` is the maximum allowed deviation in the same units as the
 * input (px); `0` keeps everything.
 *
 * Iterative rather than recursive so a pathological stroke can't blow the stack.
 */
export function simplifyIndices(points: readonly Pt[], epsilon: number): number[] {
  const n = points.length;
  if (n <= 2) return points.map((_, i) => i);
  if (epsilon <= 0) return points.map((_, i) => i);

  const keep = new Uint8Array(n);
  keep[0] = 1;
  keep[n - 1] = 1;

  // Explicit stack of [start, end] spans still to examine.
  const stack: [number, number][] = [[0, n - 1]];
  while (stack.length > 0) {
    const [start, end] = stack.pop()!;
    if (end <= start + 1) continue; // nothing between the endpoints

    let farthest = -1;
    let maxDist = -1;
    for (let i = start + 1; i < end; i++) {
      const d = perpendicularDistance(points[i]!, points[start]!, points[end]!);
      if (d > maxDist) {
        maxDist = d;
        farthest = i;
      }
    }

    // Within tolerance: every point between start and end is redundant.
    if (maxDist <= epsilon || farthest < 0) continue;

    keep[farthest] = 1;
    stack.push([start, farthest], [farthest, end]);
  }

  const out: number[] = [];
  for (let i = 0; i < n; i++) if (keep[i]) out.push(i);
  return out;
}

/** Convenience wrapper returning the kept points themselves. */
export function simplify(points: readonly Pt[], epsilon: number): Pt[] {
  return simplifyIndices(points, epsilon).map((i) => points[i]!);
}
