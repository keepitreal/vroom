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

// RSI-specific parameters (the enable toggle lives in IndicatorState above).
export type RSIParams = {
  period: number;
  upperBand: number;
  lowerBand: number;
  maEnabled: boolean;
  maPeriod: number;
};

export const DEFAULT_RSI_PARAMS: RSIParams = {
  period: 14,
  upperBand: 70,
  lowerBand: 30,
  maEnabled: true,
  maPeriod: 14,
};

export type MACDParams = {
  fast: number;
  slow: number;
  signal: number;
};

export const DEFAULT_MACD_PARAMS: MACDParams = {
  fast: 12,
  slow: 26,
  signal: 9,
};

type Props = {
  visible: boolean;
  onClose: () => void;
  state: IndicatorState;
  onToggle: (id: IndicatorId, enabled: boolean) => void;
  rsiParams: RSIParams;
  onRsiParamsChange: (patch: Partial<RSIParams>) => void;
  macdParams: MACDParams;
  onMacdParamsChange: (patch: Partial<MACDParams>) => void;
};

export function IndicatorsMenu({
  visible,
  onClose,
  state,
  onToggle,
  rsiParams,
  onRsiParamsChange,
  macdParams,
  onMacdParamsChange,
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
            rsiParams={detail.id === 'rsi' ? rsiParams : undefined}
            onRsiParamsChange={
              detail.id === 'rsi' ? onRsiParamsChange : undefined
            }
            macdParams={detail.id === 'macd' ? macdParams : undefined}
            onMacdParamsChange={
              detail.id === 'macd' ? onMacdParamsChange : undefined
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

function Stepper({
  label,
  value,
  min,
  max,
  onChange,
}: {
  label: string;
  value: number;
  min: number;
  max: number;
  onChange: (value: number) => void;
}) {
  return (
    <View style={styles.paramRow}>
      <Text style={styles.paramLabel}>{label}</Text>
      <View style={styles.stepper}>
        <Pressable
          style={styles.stepBtn}
          onPress={() => onChange(Math.max(min, value - 1))}
        >
          <Text style={styles.stepText}>−</Text>
        </Pressable>
        <Text style={styles.stepValue}>{value}</Text>
        <Pressable
          style={styles.stepBtn}
          onPress={() => onChange(Math.min(max, value + 1))}
        >
          <Text style={styles.stepText}>+</Text>
        </Pressable>
      </View>
    </View>
  );
}

function DetailScreen({
  meta,
  enabled,
  onToggle,
  onBack,
  rsiParams,
  onRsiParamsChange,
  macdParams,
  onMacdParamsChange,
}: {
  meta: IndicatorMeta;
  enabled: boolean;
  onToggle: (value: boolean) => void;
  onBack: () => void;
  rsiParams?: RSIParams;
  onRsiParamsChange?: (patch: Partial<RSIParams>) => void;
  macdParams?: MACDParams;
  onMacdParamsChange?: (patch: Partial<MACDParams>) => void;
}) {
  const rsi = rsiParams && onRsiParamsChange ? rsiParams : null;
  const macd = macdParams && onMacdParamsChange ? macdParams : null;
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
          {rsi ? (
            <>
              <Stepper
                label="Period"
                value={rsi.period}
                min={2}
                max={50}
                onChange={(n) => onRsiParamsChange!({ period: n })}
              />
              <Stepper
                label="Overbought"
                value={rsi.upperBand}
                min={rsi.lowerBand + 1}
                max={100}
                onChange={(n) => onRsiParamsChange!({ upperBand: n })}
              />
              <Stepper
                label="Oversold"
                value={rsi.lowerBand}
                min={0}
                max={rsi.upperBand - 1}
                onChange={(n) => onRsiParamsChange!({ lowerBand: n })}
              />
              <View style={styles.paramRow}>
                <Text style={styles.paramLabel}>Trendline (RSI MA)</Text>
                <Switch
                  value={rsi.maEnabled}
                  onValueChange={(v) => onRsiParamsChange!({ maEnabled: v })}
                  trackColor={{ true: '#238636', false: '#30363d' }}
                  thumbColor="#f0f6fc"
                  ios_backgroundColor="#30363d"
                />
              </View>
              {rsi.maEnabled ? (
                <Stepper
                  label="Trendline length"
                  value={rsi.maPeriod}
                  min={1}
                  max={50}
                  onChange={(n) => onRsiParamsChange!({ maPeriod: n })}
                />
              ) : null}
            </>
          ) : macd ? (
            <>
              <Stepper
                label="Fast"
                value={macd.fast}
                min={1}
                max={macd.slow - 1}
                onChange={(n) => onMacdParamsChange!({ fast: n })}
              />
              <Stepper
                label="Slow"
                value={macd.slow}
                min={macd.fast + 1}
                max={100}
                onChange={(n) => onMacdParamsChange!({ slow: n })}
              />
              <Stepper
                label="Signal"
                value={macd.signal}
                min={1}
                max={50}
                onChange={(n) => onMacdParamsChange!({ signal: n })}
              />
            </>
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
