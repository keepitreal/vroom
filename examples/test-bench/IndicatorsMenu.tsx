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
  ScrollView,
  StyleSheet,
  Switch,
  Text,
  View,
} from 'react-native';
import { SafeAreaView } from 'react-native-safe-area-context';
import type { MASource } from 'react-native-vroom-chart';

export type IndicatorId = 'bb' | 'ema' | 'macd' | 'ma' | 'rsi' | 'vwap';

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
    id: 'bb',
    name: 'Bollinger Bands',
    description:
      'A moving-average basis with bands ±N standard deviations away — the bands widen with volatility and squeeze when it fades.',
  },
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
  bb: { enabled: false },
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

// One moving-average overlay line (kind is implied by which list it's in).
export type MALineParams = {
  length: number;
  source: MASource;
  color: string;
  width: number;
};

export const DEFAULT_MA_LINE: MALineParams = {
  length: 9,
  source: 'close',
  color: '#2962ff',
  width: 1.5,
};

export const DEFAULT_EMA_LINE: MALineParams = {
  length: 9,
  source: 'close',
  color: '#ff9800',
  width: 1.5,
};

// VWAP params (session anchor). resetHour is the UTC hour the session resets.
export type VWAPParams = {
  resetHour: number;
  color: string;
  width: number;
};

export const DEFAULT_VWAP_PARAMS: VWAPParams = {
  resetHour: 0,
  color: '#00bcd4',
  width: 1.5,
};

// Bollinger Bands params (single instance; the enable toggle lives in
// IndicatorState above). One width is shared by all three lines.
export type BollingerParams = {
  period: number;
  stdDev: number;
  source: MASource;
  basis: 'sma' | 'ema';
  upperColor: string;
  middleColor: string;
  lowerColor: string;
  width: number;
  fill: boolean;
  fillOpacity: number;
};

export const DEFAULT_BOLLINGER_PARAMS: BollingerParams = {
  period: 20,
  stdDev: 2,
  source: 'close',
  basis: 'sma',
  upperColor: '#2962ff',
  middleColor: '#ff6d00',
  lowerColor: '#2962ff',
  width: 1,
  fill: true,
  fillOpacity: 0.1,
};

// Drives one overlay list editor (MA or EMA) in the detail screen.
export type OverlayEditor = {
  lines: MALineParams[];
  onChange: (index: number, patch: Partial<MALineParams>) => void;
  onAdd: () => void;
  onRemove: (index: number) => void;
};

const MA_SOURCES: MASource[] = [
  'close',
  'open',
  'high',
  'low',
  'hl2',
  'hlc3',
  'ohlc4',
];
const MA_SWATCHES = [
  '#2962ff',
  '#ff9800',
  '#26a69a',
  '#f85149',
  '#8957e5',
  '#00bcd4',
];
const MA_WIDTHS = [
  { label: 'Thin', value: 1 },
  { label: 'Med', value: 1.5 },
  { label: 'Thick', value: 2.5 },
];

type Props = {
  visible: boolean;
  onClose: () => void;
  state: IndicatorState;
  onToggle: (id: IndicatorId, enabled: boolean) => void;
  rsiParams: RSIParams;
  onRsiParamsChange: (patch: Partial<RSIParams>) => void;
  macdParams: MACDParams;
  onMacdParamsChange: (patch: Partial<MACDParams>) => void;
  maEditor: OverlayEditor;
  emaEditor: OverlayEditor;
  vwapParams: VWAPParams;
  onVwapParamsChange: (patch: Partial<VWAPParams>) => void;
  bbParams: BollingerParams;
  onBbParamsChange: (patch: Partial<BollingerParams>) => void;
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
  maEditor,
  emaEditor,
  vwapParams,
  onVwapParamsChange,
  bbParams,
  onBbParamsChange,
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
            editor={
              detail.id === 'ma'
                ? maEditor
                : detail.id === 'ema'
                  ? emaEditor
                  : undefined
            }
            vwapParams={detail.id === 'vwap' ? vwapParams : undefined}
            onVwapParamsChange={
              detail.id === 'vwap' ? onVwapParamsChange : undefined
            }
            bbParams={detail.id === 'bb' ? bbParams : undefined}
            onBbParamsChange={
              detail.id === 'bb' ? onBbParamsChange : undefined
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

function Swatches({
  value,
  onChange,
}: {
  value: string;
  onChange: (color: string) => void;
}) {
  return (
    <View style={styles.swatchRow}>
      {MA_SWATCHES.map((c) => (
        <Pressable
          key={c}
          onPress={() => onChange(c)}
          style={[
            styles.swatch,
            { backgroundColor: c },
            value.toLowerCase() === c.toLowerCase() && styles.swatchSelected,
          ]}
        />
      ))}
    </View>
  );
}

function Segmented({
  options,
  value,
  onChange,
}: {
  options: { label: string; value: number }[];
  value: number;
  onChange: (value: number) => void;
}) {
  return (
    <View style={styles.segmented}>
      {options.map((o) => {
        const active = o.value === value;
        return (
          <Pressable
            key={o.label}
            onPress={() => onChange(o.value)}
            style={[styles.segment, active && styles.segmentActive]}
          >
            <Text
              style={[styles.segmentText, active && styles.segmentTextActive]}
            >
              {o.label}
            </Text>
          </Pressable>
        );
      })}
    </View>
  );
}

function OverlayLineEditor({
  line,
  index,
  onChange,
  onRemove,
}: {
  line: MALineParams;
  index: number;
  onChange: (patch: Partial<MALineParams>) => void;
  onRemove: () => void;
}) {
  const cycleSource = () => {
    const i = MA_SOURCES.indexOf(line.source);
    onChange({ source: MA_SOURCES[(i + 1) % MA_SOURCES.length] });
  };
  return (
    <View style={styles.lineCard}>
      <View style={styles.lineHeader}>
        <Text style={styles.lineTitle}>Line {index + 1}</Text>
        <Pressable onPress={onRemove} hitSlop={8}>
          <Text style={styles.removeBtn}>Remove</Text>
        </Pressable>
      </View>
      <Stepper
        label="Length"
        value={line.length}
        min={1}
        max={400}
        onChange={(n) => onChange({ length: n })}
      />
      <View style={styles.paramRow}>
        <Text style={styles.paramLabel}>Source</Text>
        <Pressable style={styles.cycleBtn} onPress={cycleSource}>
          <Text style={styles.cycleText}>{line.source}</Text>
          <Text style={styles.cycleCaret}>⟳</Text>
        </Pressable>
      </View>
      <View style={styles.paramRow}>
        <Text style={styles.paramLabel}>Color</Text>
        <Swatches value={line.color} onChange={(c) => onChange({ color: c })} />
      </View>
      <View style={styles.paramRow}>
        <Text style={styles.paramLabel}>Width</Text>
        <Segmented
          options={MA_WIDTHS}
          value={line.width}
          onChange={(w) => onChange({ width: w })}
        />
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
  editor,
  vwapParams,
  onVwapParamsChange,
  bbParams,
  onBbParamsChange,
}: {
  meta: IndicatorMeta;
  enabled: boolean;
  onToggle: (value: boolean) => void;
  onBack: () => void;
  rsiParams?: RSIParams;
  onRsiParamsChange?: (patch: Partial<RSIParams>) => void;
  macdParams?: MACDParams;
  onMacdParamsChange?: (patch: Partial<MACDParams>) => void;
  editor?: OverlayEditor;
  vwapParams?: VWAPParams;
  onVwapParamsChange?: (patch: Partial<VWAPParams>) => void;
  bbParams?: BollingerParams;
  onBbParamsChange?: (patch: Partial<BollingerParams>) => void;
}) {
  const rsi = rsiParams && onRsiParamsChange ? rsiParams : null;
  const macd = macdParams && onMacdParamsChange ? macdParams : null;
  const vwap = vwapParams && onVwapParamsChange ? vwapParams : null;
  const bb = bbParams && onBbParamsChange ? bbParams : null;
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
          ) : editor ? (
            <>
              {editor.lines.map((line, i) => (
                <OverlayLineEditor
                  key={i}
                  line={line}
                  index={i}
                  onChange={(patch) => editor.onChange(i, patch)}
                  onRemove={() => editor.onRemove(i)}
                />
              ))}
              <Pressable style={styles.addBtn} onPress={editor.onAdd}>
                <Text style={styles.addBtnText}>+ Add line</Text>
              </Pressable>
            </>
          ) : vwap ? (
            <>
              <Stepper
                label="Reset hour (UTC)"
                value={vwap.resetHour}
                min={0}
                max={23}
                onChange={(n) => onVwapParamsChange!({ resetHour: n })}
              />
              <View style={styles.paramRow}>
                <Text style={styles.paramLabel}>Color</Text>
                <Swatches
                  value={vwap.color}
                  onChange={(c) => onVwapParamsChange!({ color: c })}
                />
              </View>
              <View style={styles.paramRow}>
                <Text style={styles.paramLabel}>Width</Text>
                <Segmented
                  options={MA_WIDTHS}
                  value={vwap.width}
                  onChange={(w) => onVwapParamsChange!({ width: w })}
                />
              </View>
            </>
          ) : bb ? (
            <>
              <Stepper
                label="Period"
                value={bb.period}
                min={1}
                max={200}
                onChange={(n) => onBbParamsChange!({ period: n })}
              />
              <View style={styles.paramRow}>
                <Text style={styles.paramLabel}>Std dev</Text>
                <Segmented
                  options={[
                    { label: '1', value: 1 },
                    { label: '1.5', value: 1.5 },
                    { label: '2', value: 2 },
                    { label: '2.5', value: 2.5 },
                    { label: '3', value: 3 },
                  ]}
                  value={bb.stdDev}
                  onChange={(v) => onBbParamsChange!({ stdDev: v })}
                />
              </View>
              <View style={styles.paramRow}>
                <Text style={styles.paramLabel}>Basis</Text>
                <Segmented
                  options={[
                    { label: 'SMA', value: 0 },
                    { label: 'EMA', value: 1 },
                  ]}
                  value={bb.basis === 'ema' ? 1 : 0}
                  onChange={(v) =>
                    onBbParamsChange!({ basis: v === 1 ? 'ema' : 'sma' })
                  }
                />
              </View>
              <View style={styles.paramRow}>
                <Text style={styles.paramLabel}>Source</Text>
                <Pressable
                  style={styles.cycleBtn}
                  onPress={() => {
                    const i = MA_SOURCES.indexOf(bb.source);
                    onBbParamsChange!({
                      source: MA_SOURCES[(i + 1) % MA_SOURCES.length],
                    });
                  }}
                >
                  <Text style={styles.cycleText}>{bb.source}</Text>
                  <Text style={styles.cycleCaret}>⟳</Text>
                </Pressable>
              </View>
              <View style={styles.paramRow}>
                <Text style={styles.paramLabel}>Band color</Text>
                <Swatches
                  value={bb.upperColor}
                  onChange={(c) =>
                    onBbParamsChange!({ upperColor: c, lowerColor: c })
                  }
                />
              </View>
              <View style={styles.paramRow}>
                <Text style={styles.paramLabel}>Basis color</Text>
                <Swatches
                  value={bb.middleColor}
                  onChange={(c) => onBbParamsChange!({ middleColor: c })}
                />
              </View>
              <View style={styles.paramRow}>
                <Text style={styles.paramLabel}>Width</Text>
                <Segmented
                  options={MA_WIDTHS}
                  value={bb.width}
                  onChange={(w) => onBbParamsChange!({ width: w })}
                />
              </View>
              <View style={styles.paramRow}>
                <Text style={styles.paramLabel}>Fill</Text>
                <Switch
                  value={bb.fill}
                  onValueChange={(v) => onBbParamsChange!({ fill: v })}
                  trackColor={{ true: '#238636', false: '#30363d' }}
                  thumbColor="#f0f6fc"
                  ios_backgroundColor="#30363d"
                />
              </View>
              {bb.fill ? (
                <View style={styles.paramRow}>
                  <Text style={styles.paramLabel}>Fill opacity</Text>
                  <Segmented
                    options={[
                      { label: '5%', value: 0.05 },
                      { label: '10%', value: 0.1 },
                      { label: '20%', value: 0.2 },
                    ]}
                    value={bb.fillOpacity}
                    onChange={(v) => onBbParamsChange!({ fillOpacity: v })}
                  />
                </View>
              ) : null}
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
  lineCard: {
    marginBottom: 14,
    padding: 12,
    borderRadius: 10,
    backgroundColor: '#161b22',
    borderWidth: StyleSheet.hairlineWidth,
    borderColor: '#21262d',
  },
  lineHeader: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
    marginBottom: 4,
  },
  lineTitle: { color: '#8b949e', fontSize: 12, fontWeight: '600' },
  removeBtn: { color: '#f85149', fontSize: 13, fontWeight: '600' },
  cycleBtn: {
    flexDirection: 'row',
    alignItems: 'center',
    paddingVertical: 6,
    paddingHorizontal: 12,
    borderRadius: 8,
    backgroundColor: '#0d1117',
    borderWidth: StyleSheet.hairlineWidth,
    borderColor: '#30363d',
  },
  cycleText: { color: '#c9d1d9', fontSize: 14, fontWeight: '600' },
  cycleCaret: { color: '#6e7681', fontSize: 13, marginLeft: 6 },
  swatchRow: { flexDirection: 'row', alignItems: 'center' },
  swatch: {
    width: 22,
    height: 22,
    borderRadius: 11,
    marginLeft: 8,
    borderWidth: 2,
    borderColor: 'transparent',
  },
  swatchSelected: { borderColor: '#f0f6fc' },
  segmented: {
    flexDirection: 'row',
    borderRadius: 8,
    overflow: 'hidden',
    borderWidth: StyleSheet.hairlineWidth,
    borderColor: '#30363d',
  },
  segment: { paddingVertical: 6, paddingHorizontal: 12, backgroundColor: '#0d1117' },
  segmentActive: { backgroundColor: '#21262d' },
  segmentText: { color: '#8b949e', fontSize: 13, fontWeight: '500' },
  segmentTextActive: { color: '#c9d1d9' },
  addBtn: {
    marginTop: 4,
    alignItems: 'center',
    paddingVertical: 12,
    borderRadius: 8,
    borderWidth: StyleSheet.hairlineWidth,
    borderColor: '#30363d',
    borderStyle: 'dashed',
  },
  addBtnText: { color: '#58a6ff', fontSize: 15, fontWeight: '600' },
});
