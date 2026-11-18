import type { OrbEvent } from '@vroomchart/types';

/** Collects events; ids are stable across re-runs so hosts can dedupe them. */
export class EventLog {
  readonly events: OrbEvent[] = [];

  add(e: Omit<OrbEvent, 'id'>, detail: string): void {
    this.events.push({ id: `${e.date}:${e.type}:${detail}`, ...e });
  }

  /** In time order; same-time events keep the order they were added. */
  sorted(): OrbEvent[] {
    return this.events
      .map((e, i) => ({ e, i }))
      .sort((a, b) => a.e.timeMs - b.e.timeMs || a.i - b.i)
      .map(({ e }) => e);
  }
}
