import { useEffect, useState } from 'react';
import type { VroomTheme } from '@vroomchart/react';

// A full, always-populated theme. Mirrors the core defaults in
// packages/core/src/theme.cpp. The four border/wick fields default to the
// transparent sentinel "#00000000", which the core resolves to the body fill.
export type ThemeState = Record<keyof VroomTheme, string>;

const INHERIT = '#00000000';

export const DEFAULT_THEME: ThemeState = {
  background: '#0d1117',
  bull: '#26a69a',
  bear: '#ef5350',
  accentBull: '#26a69a',
  accentBear: '#ef5350',
  borderBull: INHERIT,
  borderBear: INHERIT,
  wickBull: INHERIT,
  wickBear: INHERIT,
  grid: '#1a1e24',
  axisText: '#c9d1d9',
  crosshair: '#303741',
  crosshairTarget: '#3e4855',
};

// Fields whose default is "inherit the candle body fill". Each gets an
// "Inherit fill" checkbox alongside its color picker.
const INHERIT_FIELDS = new Set<keyof VroomTheme>([
  'borderBull',
  'borderBear',
  'wickBull',
  'wickBear',
]);

// The opaque color a freshly-unchecked inherit field falls back to, so the
// picker shows something sensible rather than black.
const INHERIT_FALLBACK: Partial<Record<keyof VroomTheme, string>> = {
  borderBull: '#1b7a70',
  borderBear: '#c0392b',
  wickBull: '#26a69a',
  wickBear: '#ef5350',
};

const SECTIONS: { title: string; fields: (keyof VroomTheme)[] }[] = [
  {
    title: 'Candles',
    fields: ['bull', 'bear', 'borderBull', 'borderBear', 'wickBull', 'wickBear'],
  },
  { title: 'Price & volume', fields: ['accentBull', 'accentBear'] },
  { title: 'Chart', fields: ['background', 'grid', 'axisText'] },
  { title: 'Crosshair', fields: ['crosshair', 'crosshairTarget'] },
];

const LABELS: Record<keyof VroomTheme, string> = {
  background: 'Background',
  bull: 'Bull body',
  bear: 'Bear body',
  accentBull: 'Accent up',
  accentBear: 'Accent down',
  borderBull: 'Bull border',
  borderBear: 'Bear border',
  wickBull: 'Bull wick',
  wickBear: 'Bear wick',
  grid: 'Gridlines',
  axisText: 'Axis text',
  crosshair: 'Crosshair',
  crosshairTarget: 'Crosshair target',
};

const overlay: React.CSSProperties = {
  position: 'fixed',
  inset: 0,
  background: 'rgba(1, 4, 9, 0.6)',
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'center',
  zIndex: 100,
};

const panel: React.CSSProperties = {
  background: '#161b22',
  color: '#c9d1d9',
  border: '1px solid #30363d',
  borderRadius: 10,
  width: 420,
  maxWidth: 'calc(100vw - 32px)',
  maxHeight: 'calc(100vh - 64px)',
  overflowY: 'auto',
  boxShadow: '0 12px 32px rgba(1, 4, 9, 0.7)',
  fontFamily: 'system-ui, sans-serif',
};

const btn: React.CSSProperties = {
  background: '#21262d',
  color: '#c9d1d9',
  border: '1px solid #30363d',
  borderRadius: 6,
  padding: '6px 12px',
  fontSize: 13,
  cursor: 'pointer',
};

// Normalize a typed/pasted hex string into "#rrggbb". Accepts optional leading
// '#', 3-digit shorthand (expanded), and any case. Returns null when invalid so
// the caller can keep the user's draft without committing a bad value.
function normalizeHex(input: string): string | null {
  let s = input.trim().replace(/^#/, '');
  if (/^[0-9a-fA-F]{3}$/.test(s)) {
    s = s
      .split('')
      .map((ch) => ch + ch)
      .join('');
  }
  if (!/^[0-9a-fA-F]{6}$/.test(s)) return null;
  return `#${s.toLowerCase()}`;
}

function ColorRow({
  label,
  value,
  canInherit,
  inheriting,
  fallback,
  onChange,
  onToggleInherit,
}: {
  label: string;
  value: string;
  canInherit: boolean;
  inheriting: boolean;
  fallback: string;
  onChange: (hex: string) => void;
  onToggleInherit: (inherit: boolean) => void;
}) {
  // The swatch always needs a valid #rrggbb; fall back when inheriting.
  const swatch = inheriting ? fallback : value;
  // Local draft so partial typing/pasting isn't clobbered until it's valid.
  const [draft, setDraft] = useState(swatch);
  useEffect(() => setDraft(swatch), [swatch]);

  const commit = (raw: string) => {
    const hex = normalizeHex(raw);
    if (hex) onChange(hex);
    else setDraft(swatch); // revert invalid input on blur
  };

  return (
    <div
      style={{
        display: 'flex',
        alignItems: 'center',
        gap: 10,
        padding: '5px 0',
        fontSize: 13,
      }}
    >
      <span style={{ flex: 1 }}>{label}</span>
      {canInherit && (
        <label
          style={{
            display: 'flex',
            alignItems: 'center',
            gap: 5,
            fontSize: 12,
            opacity: 0.85,
            cursor: 'pointer',
          }}
        >
          <input
            type="checkbox"
            checked={inheriting}
            onChange={(e) => onToggleInherit(e.target.checked)}
          />
          Inherit fill
        </label>
      )}
      <input
        type="text"
        value={inheriting ? '' : draft}
        placeholder={inheriting ? 'inherit' : '#rrggbb'}
        disabled={inheriting}
        spellCheck={false}
        onChange={(e) => {
          setDraft(e.target.value);
          const hex = normalizeHex(e.target.value);
          if (hex) onChange(hex);
        }}
        onBlur={(e) => commit(e.target.value)}
        onKeyDown={(e) => {
          if (e.key === 'Enter') commit((e.target as HTMLInputElement).value);
        }}
        style={{
          width: 88,
          fontFamily: 'ui-monospace, monospace',
          fontSize: 12,
          background: '#0d1117',
          color: '#c9d1d9',
          border: '1px solid #30363d',
          borderRadius: 6,
          padding: '4px 6px',
          opacity: inheriting ? 0.4 : 1,
        }}
      />
      <input
        type="color"
        value={swatch}
        disabled={inheriting}
        onChange={(e) => onChange(e.target.value)}
        style={{
          width: 36,
          height: 24,
          padding: 0,
          background: 'transparent',
          border: '1px solid #30363d',
          borderRadius: 4,
          cursor: inheriting ? 'not-allowed' : 'pointer',
          opacity: inheriting ? 0.4 : 1,
        }}
      />
    </div>
  );
}

export function SettingsModal({
  theme,
  onChange,
  onClose,
}: {
  theme: ThemeState;
  onChange: (next: ThemeState) => void;
  onClose: () => void;
}) {
  const setField = (field: keyof VroomTheme, value: string) => {
    onChange({ ...theme, [field]: value });
  };

  return (
    <div style={overlay} onClick={onClose}>
      <div style={panel} onClick={(e) => e.stopPropagation()}>
        <div
          style={{
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'space-between',
            padding: '12px 16px',
            borderBottom: '1px solid #30363d',
          }}
        >
          <strong style={{ fontSize: 15 }}>Chart colors</strong>
          <button style={btn} onClick={onClose} aria-label="Close settings">
            Close
          </button>
        </div>

        <div style={{ padding: '8px 16px 16px' }}>
          {SECTIONS.map((section) => (
            <div key={section.title} style={{ marginTop: 12 }}>
              <div
                style={{
                  fontSize: 11,
                  textTransform: 'uppercase',
                  letterSpacing: 0.6,
                  opacity: 0.6,
                  margin: '4px 0 8px',
                }}
              >
                {section.title}
              </div>
              {section.fields.map((field) => {
                const canInherit = INHERIT_FIELDS.has(field);
                const inheriting = canInherit && theme[field] === INHERIT;
                const fallback = INHERIT_FALLBACK[field] ?? '#000000';
                return (
                  <ColorRow
                    key={field}
                    label={LABELS[field]}
                    value={theme[field]}
                    canInherit={canInherit}
                    inheriting={inheriting}
                    fallback={fallback}
                    onChange={(hex) => setField(field, hex)}
                    onToggleInherit={(inherit) =>
                      setField(field, inherit ? INHERIT : fallback)
                    }
                  />
                );
              })}
            </div>
          ))}
        </div>

        <div
          style={{
            display: 'flex',
            justifyContent: 'flex-end',
            gap: 8,
            padding: '12px 16px',
            borderTop: '1px solid #30363d',
          }}
        >
          <button style={btn} onClick={() => onChange(DEFAULT_THEME)}>
            Reset to defaults
          </button>
          <button style={btn} onClick={onClose}>
            Close
          </button>
        </div>
      </div>
    </div>
  );
}
