# Drawing tools

vroom ships interactive drawing tools for annotating the chart. Drawings are
anchored in **data space** (time + price), so they stay glued to the candles as
the user pans and zooms.

Drawing is **web only** today (React Native support is planned). Three tools are
available: the **line** (a two-point trendline), the **box** (an axis-aligned
rectangle), and the **pencil** (a freehand
stroke tool).

There are two ways to manage the drawings themselves:

- **[Managed persistence](#persisting-drawings)** (recommended) — inject a storage adapter into `VroomChart` that gives vroom access to a store (e.g. local storage, MMKV).
- **[Controlled](#the-controlled-drawings-model)** — you own the `Drawings` array and persist them however you'd like.

## The controlled `drawings` model

Drawings are a **controlled** prop: the chart renders
what you pass and hands you every change through callbacks so you can manage/store. Unless you really need
full control of drawings [managed persistence](#persisting-drawings) is probably what you want.

```tsx
import { useState } from "react";
import {
  VroomChart,
  type ChartMode,
  type DrawTool,
  type Drawing,
} from "@vroomchart/react";

function Chart({ candles }) {
  const [mode, setMode] = useState<ChartMode>("pan");
  const [tool, setTool] = useState<DrawTool>(null);
  const [drawings, setDrawings] = useState<Drawing[]>([]);

  return (
    <VroomChart
      candles={candles}
      mode={mode}
      tool={tool}
      drawings={drawings}
      onDrawingComplete={(d) => setDrawings((p) => [...p, d])} // add
      onDrawingChange={(d) =>
        setDrawings((p) => p.map((x) => (x.id === d.id ? d : x)))
      } // move
      onDrawingDelete={(id) => setDrawings((p) => p.filter((x) => x.id !== id))} // remove
      onModeChange={setMode}
    />
  );
}
```

## Persisting drawings

Owning the array yourself is often more than you want. Instead, pass a
**`drawingStore`** adapter and it manages and persists the drawings for you. The chart keys storage by
**`seriesKey`** (the market identity), passed to your adapter as `marketId`.

```tsx
import { useMemo } from "react";
import { VroomChart, type DrawingStore } from "@vroomchart/react";

function Chart({ candles, asset }) {
  const drawingStore = useMemo<DrawingStore>(
    () => ({
      load: (marketId) => localStorage.getItem(`drawings:${marketId}`), // string | null
      save: (marketId, data) =>
        localStorage.setItem(`drawings:${marketId}`, data), // opaque string
    }),
    [],
  );

  return (
    <VroomChart
      candles={candles}
      seriesKey={asset} // the market — also the storage key
      drawingStore={drawingStore}
      // no `drawings` / onDrawing* — the chart owns them
    />
  );
}
```

### Async & other backends

`load` and `save` may return promises, so the same adapter shape works for
React Native (`AsyncStorage` / MMKV) or a remote backend:

```tsx
const drawingStore: DrawingStore = {
  load: (marketId) => AsyncStorage.getItem(`drawings:${marketId}`), // Promise<string | null>
  save: (marketId, data) => AsyncStorage.setItem(`drawings:${marketId}`, data),
};
```

Saves are **debounced** by the chart (so dragging a handle doesn't thrash
storage) and flushed on market switch and unmount. `seriesKey` is required for
persistence — without it, drawings work in-session but aren't saved.

## Activating a drawing tool

To start drawing, put the chart in draw mode and pick a tool — `'line'`, `'box'`
or `'pencil'`:

```tsx
setMode("draw");
setTool("line"); // or 'box' | 'pencil'
```

**How you trigger that is up to you**: a toolbar button, a menu, or a keyboard
shortcut. vroom deliberately does **not** bind a global hotkey, so it won't
collide with your app's shortcuts.

```tsx
useEffect(() => {
  const onKey = (e: KeyboardEvent) => {
    // "l" for "line"...get it?
    if (e.key.toLowerCase() !== "l" || e.metaKey || e.ctrlKey) return;
    const drawing = mode === "draw";
    setMode(drawing ? "pan" : "draw");
    setTool(drawing ? null : "line");
  };
  window.addEventListener("keydown", onKey);
  return () => window.removeEventListener("keydown", onKey);
}, [mode]);
```

## Platform support

Drawing tools are **web only** for now — the `drawings` prop and its callbacks
type-check on React Native but don't render there yet. See
[Platform differences](../reference-platform-differences.mdx).
