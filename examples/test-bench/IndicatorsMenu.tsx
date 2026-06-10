// Full-screen Indicators menu: an alphabetical list that drills into a per-
// indicator detail screen. The detail screen has an enable/disable toggle plus
// the name and description; parameter controls (period, source, color…) will
// be added here once the indicators themselves are implemented.
//
// State is controlled by the host (App owns which indicators are enabled so it
// can later feed the chart); this component only owns the list↔detail
// navigation. No bottom sheet — browsing a catalog wants a full screen.

import { useEffect, useState } from 'react';
import {
  FlatList,
  Modal,
  Pressable,
  SafeAreaView,
  ScrollView,
  StyleSheet,
  Switch,
  Text,
  View,
} from 'react-native';

export type IndicatorId = 'ema' | 'macd' | 'ma' | 'rsi' | 'vwap';

export type IndicatorConfig = {
  enabled: boolean;
  // Parameters (period, source, color, …) will live here per indicator.
};

export type IndicatorState = Record<IndicatorId, IndicatorConfig>;

type IndicatorMeta = {
  id: IndicatorId;
  name: string;
  description: string;
};

// Listed alphabetically by name. Add search once the list grows.
export const INDICATORS: IndicatorMeta[] = [
  {
    id: 'ema',
    name: 'Exponential Moving Average',
    description:
      'A moving average that weights recent prices more heavily, so it reacts faster to new moves than a simple average.',
  },
  {
    id: 'macd',
    name: 'MACD',
    description:
      'Moving Average Convergence Divergence — momentum from the gap between two EMAs, drawn with a signal line and histogram.',
  },
  {
    id: 'ma',
    name: 'Moving Average',
    description:
      'The average price over a rolling window (SMA). Smooths price action to reveal the underlying trend.',
  },
  {
    id: 'rsi',
    name: 'RSI',
    description:
      'Relative Strength Index — a 0–100 momentum oscillator that flags overbought and oversold conditions.',
  },
  {
    id: 'vwap',
    name: 'VWAP',
    description:
      'Volume Weighted Average Price — the average price over the session weighted by traded volume.',
  },
];

export const DEFAULT_INDICATOR_STATE: IndicatorState = {
  ema: { enabled: false },
  macd: { enabled: false },
  ma: { enabled: false },
  rsi: { enabled: false },
  vwap: { enabled: false },
};

export function enabledCount(state: IndicatorState): number {
  return Object.values(state).filter((c) => c.enabled).length;
}

type Props = {
  visible: boolean;
  onClose: () => void;
  state: IndicatorState;
  onToggle: (id: IndicatorId, enabled: boolean) => void;
  rsiPeriod: number;
  onRsiPeriodChange: (period: number) => void;
};

export function IndicatorsMenu({
  visible,
  onClose,
  state,
  onToggle,
  rsiPeriod,
  onRsiPeriodChange,
}: Props) {
  const [detailId, setDetailId] = useState<IndicatorId | null>(null);

  // Always reopen on the list, never the last-viewed detail.
  useEffect(() => {
    if (!visible) setDetailId(null);
  }, [visible]);

  const detail = detailId
    ? (INDICATORS.find((i) => i.id === detailId) ?? null)
    : null;

  return (
    <Modal
      visible={visible}
      animationType="slide"
      presentationStyle="fullScreen"
      onRequestClose={detail ? () => setDetailId(null) : onClose}
    >
      <SafeAreaView style={styles.container}>
        {detail ? (
          <DetailScreen
            meta={detail}
            enabled={state[detail.id].enabled}
            onToggle={(v) => onToggle(detail.id, v)}
            onBack={() => setDetailId(null)}
            period={detail.id === 'rsi' ? rsiPeriod : undefined}
            onPeriodChange={
              detail.id === 'rsi' ? onRsiPeriodChange : undefined
            }
          />
        ) : (
          <ListScreen state={state} onClose={onClose} onSelect={setDetailId} />
        )}
      </SafeAreaView>
    </Modal>
  );
}

function ListScreen({
  state,
  onClose,
  onSelect,
}: {
  state: IndicatorState;
  onClose: () => void;
  onSelect: (id: IndicatorId) => void;
}) {
  return (
    <View style={styles.flex}>
      <View style={styles.navBar}>
        <View style={styles.navSide} />
        <Text style={styles.navTitle}>Indicators</Text>
        <Pressable style={styles.navSideRight} onPress={onClose} hitSlop={8}>
          <Text style={styles.navAction}>Done</Text>
        </Pressable>
      </View>

      <FlatList
        data={INDICATORS}
        keyExtractor={(item) => item.id}
        ItemSeparatorComponent={() => <View style={styles.separator} />}
        renderItem={({ item }) => (
          <Pressable
            style={({ pressed }) => [styles.row, pressed && styles.rowPressed]}
            onPress={() => onSelect(item.id)}
          >
            <View style={styles.rowText}>
              <Text style={styles.rowName}>{item.name}</Text>
              <Text style={styles.rowDesc} numberOfLines={1}>
                {item.description}
              </Text>
            </View>
            {state[item.id].enabled ? (
              <Text style={styles.onBadge}>On</Text>
            ) : null}
            <Text style={styles.chevron}>›</Text>
          </Pressable>
        )}
      />
    </View>
  );
}

const PERIOD_MIN = 2;
const PERIOD_MAX = 50;

function DetailScreen({
  meta,
  enabled,
  onToggle,
  onBack,
  period,
  onPeriodChange,
}: {
  meta: IndicatorMeta;
  enabled: boolean;
  onToggle: (value: boolean) => void;
  onBack: () => void;
  period?: number;
  onPeriodChange?: (period: number) => void;
}) {
  const hasPeriod = period != null && onPeriodChange != null;
  return (
    <View style={styles.flex}>
      <View style={styles.navBar}>
        <Pressable style={styles.navSideLeft} onPress={onBack} hitSlop={8}>
          <Text style={styles.navAction}>‹ Indicators</Text>
        </Pressable>
        <View style={styles.navSide} />
      </View>

      <ScrollView contentContainerStyle={styles.detailContent}>
        <View style={styles.detailHeader}>
          <Text style={styles.detailName}>{meta.name}</Text>
          <Switch
            value={enabled}
            onValueChange={onToggle}
            trackColor={{ true: '#238636', false: '#30363d' }}
            thumbColor="#f0f6fc"
            ios_backgroundColor="#30363d"
          />
        </View>

        <Text style={styles.detailDescription}>{meta.description}</Text>

        <View style={styles.settingsSection}>
          <Text style={styles.sectionHeader}>SETTINGS</Text>
          {hasPeriod ? (
            <View style={styles.paramRow}>
              <Text style={styles.paramLabel}>Period</Text>
              <View style={styles.stepper}>
                <Pressable
                  style={styles.stepBtn}
                  onPress={() =>
                    onPeriodChange!(Math.max(PERIOD_MIN, period! - 1))
                  }
                >
                  <Text style={styles.stepText}>−</Text>
                </Pressable>
                <Text style={styles.stepValue}>{period}</Text>
                <Pressable
                  style={styles.stepBtn}
                  onPress={() =>
                    onPeriodChange!(Math.min(PERIOD_MAX, period! + 1))
                  }
                >
                  <Text style={styles.stepText}>+</Text>
                </Pressable>
              </View>
            </View>
          ) : (
            <Text style={styles.placeholder}>
              Parameters (period, source, color…) will appear here once this
              indicator is implemented.
            </Text>
          )}
        </View>
      </ScrollView>
    </View>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1, backgroundColor: '#0d1117' },
  flex: { flex: 1 },

  navBar: {
    height: 48,
    flexDirection: 'row',
    alignItems: 'center',
    paddingHorizontal: 12,
    borderBottomWidth: StyleSheet.hairlineWidth,
    borderBottomColor: '#21262d',
  },
  navSide: { flex: 1 },
  navSideLeft: { flex: 1, alignItems: 'flex-start' },
  navSideRight: { flex: 1, alignItems: 'flex-end' },
  navTitle: {
    flex: 2,
    textAlign: 'center',
    color: '#c9d1d9',
    fontSize: 17,
    fontWeight: '600',
  },
  navAction: { color: '#58a6ff', fontSize: 16 },

  separator: {
    height: StyleSheet.hairlineWidth,
    backgroundColor: '#21262d',
    marginLeft: 16,
  },
  row: {
    flexDirection: 'row',
    alignItems: 'center',
    paddingHorizontal: 16,
    paddingVertical: 14,
  },
  rowPressed: { backgroundColor: '#161b22' },
  rowText: { flex: 1, marginRight: 8 },
  rowName: { color: '#c9d1d9', fontSize: 16, fontWeight: '600' },
  rowDesc: { color: '#8b949e', fontSize: 13, marginTop: 2 },
  onBadge: {
    color: '#3fb950',
    fontSize: 13,
    fontWeight: '600',
    marginRight: 8,
  },
  chevron: { color: '#6e7681', fontSize: 22, fontWeight: '300' },

  detailContent: { padding: 16 },
  detailHeader: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
  },
  detailName: {
    flex: 1,
    marginRight: 12,
    color: '#f0f6fc',
    fontSize: 20,
    fontWeight: '700',
  },
  detailDescription: {
    color: '#8b949e',
    fontSize: 15,
    lineHeight: 21,
    marginTop: 12,
  },
  settingsSection: { marginTop: 28 },
  sectionHeader: {
    color: '#6e7681',
    fontSize: 12,
    fontWeight: '600',
    letterSpacing: 0.5,
    marginBottom: 8,
  },
  placeholder: { color: '#8b949e', fontSize: 14, lineHeight: 20 },
  paramRow: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
  },
  paramLabel: { color: '#c9d1d9', fontSize: 15 },
  stepper: { flexDirection: 'row', alignItems: 'center' },
  stepBtn: {
    width: 36,
    height: 36,
    alignItems: 'center',
    justifyContent: 'center',
    borderRadius: 8,
    backgroundColor: '#161b22',
    borderWidth: StyleSheet.hairlineWidth,
    borderColor: '#30363d',
  },
  stepText: { color: '#c9d1d9', fontSize: 20, fontWeight: '600' },
  stepValue: {
    color: '#f0f6fc',
    fontSize: 16,
    fontWeight: '700',
    minWidth: 44,
    textAlign: 'center',
    fontVariant: ['tabular-nums'],
  },
});
