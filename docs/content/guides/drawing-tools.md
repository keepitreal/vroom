# Drawing tools

vroom ships interactive drawing tools for annotating the chart. Drawings are
anchored in **data space** (time + price), so they stay glued to the candles as
the user pans and zooms.

Drawing is **web only** today (React Native support is planned). Four tools are
available: the **line** (a two-point trendline), the **box** (an axis-aligned
rectangle), the **pencil** (a freehand stroke tool), and the **path** (a
multi-segment polyline ending in an arrowhead).

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

To start drawing, put the chart in draw mode and pick a tool — `'line'`, `'box'`,
`'pencil'` or `'path'`:

```tsx
setMode("draw");
setTool("line"); // or 'box' | 'pencil' | 'path'
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

## The path tool

The line and box are done after two clicks, and the pencil after you lift the
pointer. A path has no fixed length: the first click drops its starting vertex,
every click after that adds a segment, and a preview segment follows the cursor
in between. Because there's no point count to finish on, **you** say when it's
done:

| Gesture | Result |
| --- | --- |
| **Escape** | Finish the path where it stands |
| **Double-click** | Finish it at the vertex you just placed |
| **Right-click** | Finish it without placing another vertex |
| **⌘Z / Ctrl+Z**, **Backspace**, **Delete** | Take back the last vertex |

An arrowhead marks the path's direction of travel. While you're drawing it rides
the preview segment under the cursor; once you finish, it sits on the final
vertex. A path that never got a second vertex isn't a shape, so finishing one
discards it rather than committing a dot.

Hold **Shift** while placing a vertex to constrain that segment to 45° off the
previous one, the same as a trendline's second point. A path holds at most 64
vertices.

Once committed, a path behaves like the other tools in pan mode — click its body
to select it, drag the body to move the whole thing — plus **every vertex is its
own drag handle**, so you can reshape a leg without redrawing.

## Undo & redo

Every committed drawing action — a finished stroke or shape, an endpoint drag, a
delete — is one undo step. Draw two pencil strokes and press **⌘Z / Ctrl+Z**:
only the second disappears; again, and the first goes too. **⇧⌘Z /
Ctrl+Shift+Z / Ctrl+Y** redoes in order. In-progress gestures never enter
history (Escape or ⌘Z cancels a pending point instead).

History is **in-memory and session-only** — the drawings persist (via
`drawingStore` or your own state), the undo stack deliberately doesn't, matching
Excalidraw, Figma and Photoshop. In managed mode it's also scoped per
`seriesKey`: switching markets resets it, so undo can never edit a market you're
no longer looking at. Depth is capped at 100 steps (`historyLimit` to change).

### Managed mode

With a `drawingStore` the shortcuts just work. For toolbar buttons, bind
availability via `onHistoryChange` and trigger via `historyRef`:

```tsx
import { useRef, useState } from "react";
import {
  VroomChart,
  type UndoRedoControls,
  type UndoRedoState,
} from "@vroomchart/react";

function Chart({ candles, asset, drawingStore }) {
  const [history, setHistory] = useState<UndoRedoState>({
    canUndo: false,
    canRedo: false,
  });
  const historyRef = useRef<UndoRedoControls | null>(null);

  return (
    <>
      <button disabled={!history.canUndo} onClick={() => historyRef.current?.undo()}>
        Undo
      </button>
      <button disabled={!history.canRedo} onClick={() => historyRef.current?.redo()}>
        Redo
      </button>
      <VroomChart
        candles={candles}
        seriesKey={asset}
        drawingStore={drawingStore}
        onHistoryChange={setHistory}
        historyRef={historyRef}
      />
    </>
  );
}
```

### Controlled mode

When you own the array, own the history too — `useDrawingHistory` bundles both.
Wire its handlers to the chart and its `undo`/`redo` to the `onUndo`/`onRedo`
props (which fire on the keyboard shortcuts):

```tsx
import { VroomChart, useDrawingHistory } from "@vroomchart/react";

function Chart({ candles }) {
  const h = useDrawingHistory();

  return (
    <VroomChart
      candles={candles}
      drawings={h.drawings}
      onDrawingComplete={h.onDrawingComplete}
      onDrawingChange={h.onDrawingChange}
      onDrawingDelete={h.onDrawingDelete}
      onUndo={h.undo}
      onRedo={h.redo}
    />
  );
}
```

`h.canUndo` / `h.canRedo` drive buttons; `h.replaceAll(list)` swaps in a loaded
document without recording an undo step. For fully custom state management, the
bare `DrawingHistory` stack is exported too.

## Platform support

Drawing tools are **web only** for now — the `drawings` prop and its callbacks
type-check on React Native but don't render there yet. See
[Platform differences](../reference-platform-differences.mdx).
