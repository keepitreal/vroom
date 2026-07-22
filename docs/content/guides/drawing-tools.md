# Drawing tools

vroom ships interactive drawing tools for annotating the chart. Drawings are
anchored in **data space** (time + price), so they stay glued to the candles as
the user pans and zooms — and survive reloads if you persist them.

Drawing is **web only** today (React Native support is planned). Three tools are
available: the **line** (a two-point trendline), the **box** (an axis-aligned
rectangle defined by two opposite corners), and the **pencil** (a freehand
stroke you draw by dragging).

There are two ways to manage the drawings themselves:

- **[Managed persistence](#persisting-drawings)** (recommended) — the chart owns
  the array and saves/loads it through a storage adapter you inject. You write
  almost no glue.
- **[Controlled](#the-controlled-drawings-model)** — you own the array and apply
  each change yourself. Full control, more wiring.

## The controlled `drawings` model

Drawings are a **controlled** prop — your app owns the array. The chart renders
what you pass and hands you every change through callbacks; you apply them to
your state. (Prefer [managed persistence](#persisting-drawings) unless you need
full control.)

```tsx
import { useState } from 'react';
import { VroomChart, type ChartMode, type DrawTool, type Drawing } from '@vroomchart/react';

function Chart({ candles }) {
  const [mode, setMode] = useState<ChartMode>('pan');
  const [tool, setTool] = useState<DrawTool>(null);
  const [drawings, setDrawings] = useState<Drawing[]>([]);

  return (
    <VroomChart
      candles={candles}
      mode={mode}
      tool={tool}
      drawings={drawings}
      onDrawingComplete={(d) => setDrawings((p) => [...p, d])}                        // add
      onDrawingChange={(d) => setDrawings((p) => p.map((x) => (x.id === d.id ? d : x)))} // move
      onDrawingDelete={(id) => setDrawings((p) => p.filter((x) => x.id !== id))}      // remove
      onModeChange={setMode}
    />
  );
}
```

## Persisting drawings

Owning the array yourself is often more than you want. Instead, hand the chart a
**`drawingStore`** adapter and it manages and persists the drawings for you — no
`drawings` prop, no `onDrawing*` callbacks. The chart keys storage by
**`seriesKey`** (the market identity), passed to your adapter as `marketId`.

The adapter is a **plain string key-value store**: the chart serializes drawings
into a versioned envelope and hands you the string; you just persist bytes.

```tsx
import { useMemo } from 'react';
import { VroomChart, type DrawingStore } from '@vroomchart/react';

function Chart({ candles, asset }) {
  const drawingStore = useMemo<DrawingStore>(() => ({
    load: (marketId) => localStorage.getItem(`drawings:${marketId}`),          // string | null
    save: (marketId, data) => localStorage.setItem(`drawings:${marketId}`, data), // opaque string
  }), []);

  return (
    <VroomChart
      candles={candles}
      seriesKey={asset}          // the market — also the storage key
      drawingStore={drawingStore}
      // no `drawings` / onDrawing* — the chart owns them
    />
  );
}
```

Don't parse or reshape the string — treat it as opaque bytes. Store it verbatim
and hand back exactly what you were given.

### Why keyed by market

Drawings are anchored in **data space** (`{ timeMs, price }`), so a saved line
renders correctly on **any timeframe** of the same market. Keying the store by
market (`seriesKey`) gives exactly the behavior you want:

- **Switch timeframe** (`seriesKey` unchanged) → the lines stay.
- **Switch market** (`seriesKey` changes) → the chart saves the outgoing market's
  lines and loads the incoming market's.
- **Reload the page** → lines come back from your store.

The `marketId` argument means your adapter is a stateless "read/write this
market" object you define once — the chart tells it which market to act on
(important during a switch, where it must save the *old* market and load the
*new* one in the same moment).

### Async & other backends

`load` and `save` may return promises, so the same adapter shape works for
React Native (`AsyncStorage` / MMKV) or a remote backend:

```tsx
const drawingStore: DrawingStore = {
  load: (marketId) => AsyncStorage.getItem(`drawings:${marketId}`),   // Promise<string | null>
  save: (marketId, data) => AsyncStorage.setItem(`drawings:${marketId}`, data),
};
```

Saves are **debounced** by the chart (so dragging a handle doesn't thrash
storage) and flushed on market switch and unmount. `seriesKey` is required for
persistence — without it, drawings work in-session but aren't saved.

### Versioning & forward compatibility

The string is a **versioned envelope** (`{ v, drawings }`) the library owns.
Because your adapter never looks inside it, the persistence contract stays stable
as vroom evolves:

- **New drawing tools** — a payload can gain new drawing types. An older build
  that doesn't recognize a type simply drops it on load instead of crashing, so a
  store shared across versions degrades gracefully.
- **New persisted fields** — the library bumps the envelope version and migrates
  older payloads to the current shape when it loads them. Your stored strings
  upgrade transparently; your adapter code never changes.

If you need to pre-seed a store, or build an import/export feature, use the
`serializeDrawings` / `deserializeDrawings` helpers exported from
`@vroomchart/react` rather than hand-rolling the envelope.

## Activating a drawing tool

To start drawing, put the chart in draw mode and pick a tool — `'line'`, `'box'`
or `'pencil'`:

```tsx
setMode('draw');
setTool('line'); // or 'box' | 'pencil'
```

**How you trigger that is up to you** — a toolbar button, a menu, or a keyboard
shortcut. vroom deliberately does **not** bind a global hotkey, so it never
collides with your app's shortcuts. A common choice (Figma/Excalidraw style) is
the `L` key:

```tsx
useEffect(() => {
  const onKey = (e: KeyboardEvent) => {
    if (e.key.toLowerCase() !== 'l' || e.metaKey || e.ctrlKey || e.altKey) return;
    const el = document.activeElement as HTMLElement | null;
    if (el?.tagName === 'INPUT' || el?.tagName === 'TEXTAREA' || el?.isContentEditable) return;
    setMode((m) => {
      const next = m === 'draw' ? 'pan' : 'draw';
      setTool(next === 'draw' ? 'line' : null);
      return next;
    });
  };
  window.addEventListener('keydown', onKey);
  return () => window.removeEventListener('keydown', onKey);
}, []);
```

## Interactions

Once the tool is active, all of the interaction and keyboard handling below is
**built into the chart** — your app only reacts to the callbacks.

### Drawing

| Action | Effect |
| --- | --- |
| Click, then click again (line, box) | Places the two anchors — a line's endpoints, or a box's two opposite corners; the shape commits (`onDrawingComplete`) |
| Press and drag (pencil) | Draws freehand for as long as the button is held; releasing commits the stroke (`onDrawingComplete`). The tool stays active, so you can draw several strokes in a row |
| Hold **Shift** while placing the 2nd point | **Line**: snaps to the nearest 45° (0° / 45° / 90° …). **Box**: constrains to a perfect square (equal side lengths) |
| **Esc** / **Delete** / **Backspace** after the first point | Cancels the in-progress shape (stays in draw mode) |

### Editing (in `pan` mode)

| Action | Effect |
| --- | --- |
| Click a shape | Selects it; a line shows its 2 endpoint handles, a box its 4 corner handles, a pencil stroke anchors on its first and last point |
| Drag a handle | **Line**: moves that endpoint. **Box**: resizes from that corner, keeping the diagonally opposite corner fixed and all corners at 90° (`onDrawingChange`). Hold **Shift** to snap to 45° / a square. **Pencil**: its anchors are a visual cue only — see below |
| Drag the body | Moves the whole shape (`onDrawingChange`). A box's faint interior fill is grabbable |
| Drag a pencil stroke (anywhere, anchors included) | Translates the whole path. A committed stroke is never reshaped or resized — dragging an anchor moves it exactly like dragging the middle of the stroke does |
| **Delete** / **Backspace** (with a shape selected) | Deletes it (`onDrawingDelete`) |
| **Cmd/Ctrl + C**, then **Cmd/Ctrl + V** | Copies the selected shape and pastes a copy (`onDrawingComplete`) — under the crosshair, or above/below the original when the pointer hasn't moved |
| Click empty space | Deselects |

## About pencil strokes

A pencil `Drawing` differs from the other two in shape: `points` is a
**variable-length array** of the path's points in draw order, rather than the
exact two-anchor tuple a `'line'` or `'box'` carries. `Drawing` is a
discriminated union, so narrow on `type` before indexing:

```tsx
if (d.type === 'pencil') {
  console.log(d.points.length); // the whole path
} else {
  const [a, b] = d.points;      // always exactly two anchors
}
```

The chart thins each stroke before committing it (dropping points that don't
change the rendered shape), so a stroke typically persists as a few dozen points
rather than the hundreds a drag generates. They still serialize to noticeably
more than a line or box — worth keeping in mind if your `drawingStore` writes to
a size-limited backend such as `localStorage`.

## Platform support

Drawing tools are **web only** for now — the `drawings` prop and its callbacks
type-check on React Native but don't render there yet. See
[Platform differences](../reference-platform-differences.mdx).
