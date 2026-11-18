import React from 'react';
import { StyleSheet, Text, View, type StyleProp, type ViewStyle } from 'react-native';
import { orbStatsRows } from '@vroomchart/strategies';
import type { OrbResult } from '@vroomchart/types';

export type OrbStatsPanelProps = {
  result: OrbResult;
  /** Corner of the parent to sit in. Default 'topLeft'. */
  position?: 'topLeft' | 'topRight' | 'bottomLeft' | 'bottomRight';
  /** Merged over the panel's own style. */
  style?: StyleProp<ViewStyle>;
};

const CORNER: Record<NonNullable<OrbStatsPanelProps['position']>, ViewStyle> = {
  topLeft: { top: 8, left: 8 },
  topRight: { top: 8, right: 64 },
  bottomLeft: { bottom: 32, left: 8 },
  bottomRight: { bottom: 32, right: 64 },
};

/**
 * The Opening Range Breakout stats table: trades, win rate, R statistics,
 * longs vs shorts and days skipped per filter, plus any warning that turned
 * signals off. `<VroomChart>` renders one when `orb.display.showStatsPanel`
 * is on; render it yourself from `onOrbResult` to place it elsewhere.
 */
export function OrbStatsPanel({ result, position = 'topLeft', style }: OrbStatsPanelProps) {
  return (
    <View pointerEvents="none" style={[styles.panel, CORNER[position], style]} accessibilityLabel="ORB statistics">
      <Text style={[styles.text, styles.title]}>ORB</Text>
      {result.warnings.map((w) => (
        <Text key={w.code} style={[styles.text, styles.warning]}>
          {w.message}
        </Text>
      ))}
      {orbStatsRows(result).map(([label, value]) => (
        <View key={label} style={styles.row}>
          <Text style={[styles.text, styles.label]}>{label}</Text>
          <Text style={styles.text}>{value}</Text>
        </View>
      ))}
    </View>
  );
}

const styles = StyleSheet.create({
  panel: {
    position: 'absolute',
    paddingHorizontal: 8,
    paddingVertical: 6,
    borderRadius: 6,
    backgroundColor: 'rgba(0, 0, 0, 0.6)',
    maxWidth: 240,
  },
  row: { flexDirection: 'row', justifyContent: 'space-between' },
  text: { color: '#fff', fontSize: 11, lineHeight: 15, fontVariant: ['tabular-nums'] },
  title: { fontWeight: '600' },
  label: { opacity: 0.75, marginRight: 10 },
  warning: { color: '#ffb74d' },
});
