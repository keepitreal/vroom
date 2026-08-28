// A floating formatting tray for the selected drawing — the consumer scenario
// `onSelectionChange` + `restyle` exist for.
//
// Everything here is ordinary absolutely-positioned DOM: the tray knows where to
// sit because the chart hands it the selection's rect in CSS px relative to the
// container, refreshed on every painted frame. Pan or zoom the chart and the
// tray rides along without this file subscribing to a single gesture.

import type { CSSProperties } from 'react';
import type { DrawingControls, DrawingSelection } from '@vroomchart/react';

// #aarrggbb — vroom reads the alpha as the *high* byte, not CSS's trailing one.
const FILL_SWATCHES = [
  { label: 'green', value: '#5426a69a' },
  { label: 'red', value: '#54ef5350' },
  { label: 'blue', value: '#542962ff' },
] as const;

// The tray's own footprint, so it can be flipped below the drawing when there
// isn't room above.
const TRAY_H = 34;
const GAP = 8;

const tray: CSSProperties = {
  position: 'absolute',
  display: 'flex',
  alignItems: 'center',
  gap: 6,
  height: TRAY_H,
  padding: '0 8px',
  borderRadius: 8,
  background: '#1e222d',
  border: '1px solid #2a2e39',
  boxShadow: '0 4px 12px rgba(0,0,0,0.4)',
  font: '12px system-ui, sans-serif',
  color: '#d1d4dc',
  pointerEvents: 'auto',
  zIndex: 5,
};

const btn: CSSProperties = {
  border: '1px solid #2a2e39',
  background: '#131722',
  color: '#d1d4dc',
  borderRadius: 4,
  padding: '2px 8px',
  cursor: 'pointer',
  font: 'inherit',
};

export function SelectionTray({
  selection,
  controls,
}: {
  selection: DrawingSelection | null;
  controls: { current: DrawingControls | null };
}) {
  if (!selection) return null;
  const { drawing, rect } = selection;
  const above = rect.y - TRAY_H - GAP;
  const style: CSSProperties = {
    ...tray,
    left: Math.max(GAP, rect.x),
    top: above >= GAP ? above : rect.y + rect.height + GAP,
  };

  const restyle = (patch: Parameters<DrawingControls['restyle']>[1]) =>
    controls.current?.restyle(drawing.id, patch);

  return (
    <div style={style} data-testid="selection-tray">
      <span style={{ opacity: 0.6 }}>{drawing.type}</span>
      {drawing.type === 'box' &&
        FILL_SWATCHES.map((s) => (
          <button
            key={s.value}
            title={`fill ${s.label}`}
            aria-label={`fill ${s.label}`}
            onClick={() => restyle({ fill: s.value })}
            style={{
              ...btn,
              width: 18,
              height: 18,
              padding: 0,
              // The swatch previews the fill, so it has to reorder vroom's
              // #aarrggbb into the #rrggbbaa that CSS expects.
              background: `#${s.value.slice(3)}${s.value.slice(1, 3)}`,
            }}
          />
        ))}
      <button
        onClick={() => restyle({ locked: !drawing.locked })}
        style={{ ...btn, ...(drawing.locked ? { borderColor: '#f7931a' } : null) }}
      >
        {drawing.locked ? 'Locked' : 'Lock'}
      </button>
      <span style={{ opacity: 0.4, fontVariantNumeric: 'tabular-nums' }} data-testid="selection-rect">
        {Math.round(rect.x)},{Math.round(rect.y)} {Math.round(rect.width)}×
        {Math.round(rect.height)}
      </span>
    </div>
  );
}
