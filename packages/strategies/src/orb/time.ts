// Wall-clock time in an IANA zone, from epoch ms and back, via Intl. Offsets
// are cached per 15-minute UTC bucket: every real zone's offset is a multiple
// of 15 minutes and changes on such a boundary, so a cached bucket is exact
// across DST transitions while costing one Intl call per bucket rather than
// one per bar.

const BUCKET_MS = 15 * 60_000;
const DAY_MS = 86_400_000;

/** A local calendar position. */
export type LocalTime = {
  /** `'YYYY-MM-DD'`. */
  date: string;
  /** Minutes after local midnight (fractional for sub-minute times). */
  minute: number;
  /** 0 = Sunday … 6 = Saturday. */
  weekday: number;
};

export class Zone {
  private readonly fmt: Intl.DateTimeFormat;
  private readonly offsets = new Map<number, number>();

  /** Throws a RangeError when `timeZone` isn't a zone this runtime knows. */
  constructor(readonly timeZone: string) {
    this.fmt = new Intl.DateTimeFormat('en-US', {
      timeZone,
      hourCycle: 'h23',
      year: 'numeric',
      month: '2-digit',
      day: '2-digit',
      hour: '2-digit',
      minute: '2-digit',
    });
  }

  /** The zone's UTC offset at `ms`, in minutes (New York in winter: -300). */
  offsetMin(ms: number): number {
    const bucket = Math.floor(ms / BUCKET_MS);
    const hit = this.offsets.get(bucket);
    if (hit !== undefined) return hit;
    const at = bucket * BUCKET_MS;
    const p: Record<string, number> = {};
    for (const part of this.fmt.formatToParts(at)) {
      if (part.type !== 'literal') p[part.type] = Number(part.value);
    }
    // Some engines still render midnight as hour 24 under h23.
    const hour = p.hour === 24 ? 0 : p.hour;
    const asUtc = Date.UTC(p.year, p.month - 1, p.day, hour, p.minute);
    const off = Math.round((asUtc - at) / 60_000);
    this.offsets.set(bucket, off);
    return off;
  }

  local(ms: number): LocalTime {
    const localMs = ms + this.offsetMin(ms) * 60_000;
    const d = new Date(localMs);
    const dayStart = Math.floor(localMs / DAY_MS) * DAY_MS;
    return {
      date: ymd(d),
      minute: (localMs - dayStart) / 60_000,
      weekday: d.getUTCDay(),
    };
  }

  /** Epoch ms of `minute` after local midnight on `date`. */
  toUtc(date: string, minute: number): number {
    const [y, m, d] = date.split('-').map(Number);
    const naive = Date.UTC(y, m - 1, d) + minute * 60_000;
    // Two passes: the offset at the naive guess may differ from the one at the
    // answer when a DST change falls between them.
    const guess = naive - this.offsetMin(naive) * 60_000;
    return naive - this.offsetMin(guess) * 60_000;
  }
}

function ymd(d: Date): string {
  const mm = String(d.getUTCMonth() + 1).padStart(2, '0');
  const dd = String(d.getUTCDate()).padStart(2, '0');
  return `${d.getUTCFullYear()}-${mm}-${dd}`;
}
