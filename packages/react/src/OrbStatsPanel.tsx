import React from 'react';
import type { CSSProperties } from 'react';
import { orbStatsRows } from '@vroomchart/strategies';
import type { OrbResult } from '@vroomchart/types';

export type OrbStatsPanelProps = {
  result: OrbResult;
  /** Corner of the positioned parent to sit in. Default 'topLeft'. */
  position?: 'topLeft' | 'topRight' | 'bottomLeft' | 'bottomRight';
  /** Merged over the panel's own style. */
  style?: CSSProperties;
};

const CORNER: Record<NonNullable<OrbStatsPanelProps['position']>, CSSProperties> = {
  topLeft: { top: 8, left: 8 },
  topRight: { top: 8, right: 72 },
  bottomLeft: { bottom: 32, left: 8 },
  bottomRight: { bottom: 32, right: 72 },
};

const PANEL: CSSProperties = {
  position: 'absolute',
  zIndex: 1,
  padding: '6px 8px',
  borderRadius: 6,
  background: 'rgba(0, 0, 0, 0.6)',
  color: '#fff',
  font: '11px/1.35 ui-monospace, SFMono-Regular, Menlo, monospace',
  pointerEvents: 'none',
  maxWidth: 300,
};

/**
 * The Opening Range Breakout stats table: trades, win rate, R statistics,
 * longs vs shorts and days skipped per filter, plus any warning that turned
 * signals off. `<VroomChart>` renders one when `orb.display.showStatsPanel`
 * is on; render it yourself from `onOrbResult` to place it elsewhere.
 */
export function OrbStatsPanel({ result, position = 'topLeft', style }: OrbStatsPanelProps) {
  return (
    <div style={{ ...PANEL, ...CORNER[position], ...style }} aria-label="ORB statistics">
      <div style={{ fontWeight: 600, marginBottom: 2 }}>ORB</div>
      {result.warnings.map((w) => (
        <div key={w.code} style={{ color: '#ffb74d', marginBottom: 2 }}>
          {w.message}
        </div>
      ))}
      <table style={{ borderCollapse: 'collapse' }}>
        <tbody>
          {orbStatsRows(result).map(([label, value]) => (
            <tr key={label}>
              <td style={{ paddingRight: 10, opacity: 0.75, whiteSpace: 'nowrap' }}>{label}</td>
              <td style={{ textAlign: 'right', whiteSpace: 'nowrap' }}>{value}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}
