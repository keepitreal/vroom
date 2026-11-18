import type { OrbEvent, OrbResult } from '@vroomchart/types';

/**
 * Decides which events of a re-run are new, so a live chart reports each event
 * once. The first run for a context (series, config) only records what's
 * there: history loading in is not news. After that an event fires when its id
 * is unseen and it happened at or after the previous run's newest bar, which
 * also keeps older history paged in on the left from firing.
 */
export class OrbEventTracker {
  private seen = new Set<string>();
  private context: string | null = null;
  private newestMs = Infinity;

  /** Returns the events to fire, oldest first. */
  update(result: OrbResult, newestBarMs: number, context: string): OrbEvent[] {
    if (context !== this.context) {
      this.context = context;
      this.seen = new Set();
      this.newestMs = Infinity;
    }
    const fresh: OrbEvent[] = [];
    for (const e of result.events) {
      if (this.seen.has(e.id)) continue;
      this.seen.add(e.id);
      if (e.timeMs >= this.newestMs) fresh.push(e);
    }
    this.newestMs = newestBarMs;
    return fresh;
  }

  reset(): void {
    this.context = null;
    this.seen = new Set();
    this.newestMs = Infinity;
  }
}
