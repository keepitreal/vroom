import { Picker } from '@react-native-picker/picker';
import * as Haptics from 'expo-haptics';
import { StatusBar } from 'expo-status-bar';
import { useCallback, useMemo, useState } from 'react';
import { Modal, Pressable, StyleSheet, Text, View } from 'react-native';
import { GestureHandlerRootView } from 'react-native-gesture-handler';
import {
  SafeAreaProvider,
  SafeAreaView,
} from 'react-native-safe-area-context';
import {
  VroomChart,
  type Candle,
  type CrosshairEvent,
  type MovingAverageOverlay,
  type PriceLine,
} from 'react-native-vroom-chart';

import {
  DEFAULT_BOLLINGER_PARAMS,
  DEFAULT_EMA_LINE,
  DEFAULT_INDICATOR_STATE,
  DEFAULT_MA_LINE,
  DEFAULT_MACD_PARAMS,
  DEFAULT_RSI_PARAMS,
  DEFAULT_VWAP_PARAMS,
  enabledCount,
  IndicatorsMenu,
  type BollingerParams,
  type IndicatorId,
  type IndicatorState,
  type MACDParams,
  type MALineParams,
  type RSIParams,
  type VWAPParams,
} from './IndicatorsMenu';

const MINUTE = 60_000;
const INTERVALS = [
  { label: '1m', ms: MINUTE },
  { label: '5m', ms: 5 * MINUTE },
  { label: '15m', ms: 15 * MINUTE },
  { label: '1h', ms: 60 * MINUTE },
  { label: '4h', ms: 240 * MINUTE },
  { label: '1d', ms: 1440 * MINUTE },
  { label: '1w', ms: 7 * 1440 * MINUTE },
] as const;

type Interval = (typeof INTERVALS)[number];

function mockCandles(n: number, stepMs: number): Candle[] {
  const out: Candle[] = [];
  let price = 100;
  const now = Date.now();
  for (let i = 0; i < n; i++) {
    const open = price;
    const close = open + (Math.random() - 0.5) * 4;
    const high = Math.max(open, close) + Math.random() * 2;
    const low = Math.min(open, close) - Math.random() * 2;
    out.push({
      timeMs: now - (n - i) * stepMs,
      open,
      high,
      low,
      close,
      volume: Math.random() * 1000,
    });
    price = close;
  }
  return out;
}

// A price line plus the label prefix its `text` is built from, so a drag can
// rebuild the caption around the new price.
type DemoPriceLine = PriceLine & { label: string };

const priceLineText = (label: string, price: number) => `${label} @ ${price.toFixed(2)}`;

// Sample order/position lines around the latest close: a draggable resting limit
// buy below spot (with a size pill and a cancel button), the take-profit it pairs
// with above, and a fixed liquidation level that can't be moved or dismissed.
// Each takes a different lineStyle (the take-profit leaves it unset for the
// dotted default) so all three render at once.
function mockPriceLines(candles: Candle[]): DemoPriceLine[] {
  if (candles.length === 0) return [];
  // Spread the samples across the range of the candles that are roughly on
  // screen. Off-range lines are culled, so anchoring to the visible range keeps
  // all three in view no matter where the random walk wandered.
  const recent = candles.slice(-120);
  let lo = recent[0].low;
  let hi = recent[0].high;
  for (const c of recent) {
    if (c.low < lo) lo = c.low;
    if (c.high > hi) hi = c.high;
  }
  const at = (frac: number) => lo + (hi - lo) * frac;
  const line = (
    id: string,
    label: string,
    price: number,
    rest: Partial<DemoPriceLine>,
  ): DemoPriceLine => ({ id, label, price, text: priceLineText(label, price), ...rest });
  return [
    line('limit-buy', 'Limit Buy', at(0.3), {
      quantity: '0.75',
      color: '#26a69a',
      draggable: true,
      lineStyle: 'dashed',
    }),
    line('take-profit', 'Take Profit', at(0.78), {
      quantity: 'Full',
      color: '#ef5350',
      draggable: true,
    }),
    line('liquidation', 'Liquidation', at(0.08), {
      color: '#f0a020',
      closable: false,
      lineStyle: 'solid',
    }),
  ];
}

function fmtVol(v: number): string {
  if (v >= 1_000_000) return `${(v / 1_000_000).toFixed(1)}m`;
  if (v >= 1_000) return `${(v / 1_000).toFixed(1)}k`;
  return v.toFixed(0);
}

function Field({
  label,
  value,
  color,
}: {
  label: string;
  value: string;
  color?: string;
}) {
  return (
    <View style={styles.ohlcvField}>
      <Text style={styles.ohlcvLabel}>{label}</Text>
      <Text style={[styles.ohlcvValue, color ? { color } : null]}>{value}</Text>
    </View>
  );
}

// Native interval picker: a compact trigger that opens the OS picker wheel in a
// bottom modal.
function IntervalSelect({
  value,
  onChange,
}: {
  value: Interval;
  onChange: (interval: Interval) => void;
}) {
  const [open, setOpen] = useState(false);
  return (
    <>
      <Pressable style={styles.selectTrigger} onPress={() => setOpen(true)}>
        <Text style={styles.selectValue}>{value.label}</Text>
        <Text style={styles.selectCaret}>▾</Text>
      </Pressable>

      <Modal
        visible={open}
        transparent
        animationType="slide"
        onRequestClose={() => setOpen(false)}
      >
        <Pressable
          style={styles.pickerBackdrop}
          onPress={() => setOpen(false)}
        />
        <View style={styles.pickerSheet}>
          <View style={styles.pickerBar}>
            <Pressable onPress={() => setOpen(false)} hitSlop={8}>
              <Text style={styles.pickerDone}>Done</Text>
            </Pressable>
          </View>
          <Picker
            selectedValue={value.label}
            onValueChange={(label) => {
              const next = INTERVALS.find((it) => it.label === label);
              if (next) onChange(next);
            }}
            itemStyle={styles.pickerItem}
          >
            {INTERVALS.map((it) => (
              <Picker.Item
                key={it.label}
                label={it.label}
                value={it.label}
                color="#c9d1d9"
              />
            ))}
          </Picker>
        </View>
      </Modal>
    </>
  );
}

export default function App() {
  const [selected, setSelected] = useState<Interval>(INTERVALS[0]);
  const candles = useMemo(() => mockCandles(1000, selected.ms), [selected]);

  // OHLCV readout: the candle under the crosshair while it's active, otherwise
  // the latest candle. Updated live via VroomChart's onCrosshair callback.
  const [hover, setHover] = useState<Candle | null>(null);
  const shown = hover ?? candles[candles.length - 1] ?? null;
  const bull = shown ? shown.close >= shown.open : true;

  // The chart only *reports* crosshair events — the app decides on haptics.
  // Impact when it appears, a light selection tick each time it crosses into a
  // new candle ('move' fires once per candle, not per drag frame).
  const handleCrosshair = useCallback((e: CrosshairEvent) => {
    setHover(e.active ? e.candle : null);
    if (e.reason === 'show') {
      Haptics.impactAsync(Haptics.ImpactFeedbackStyle.Medium).catch(() => {});
    } else if (e.reason === 'move') {
      Haptics.selectionAsync().catch(() => {});
    }
  }, []);

  // Price lines are a controlled prop, so the app owns the array: a drag reports
  // a candidate price and the × reports a cancellation, and this state is what
  // decides whether either actually happens. Re-seeded per interval, since the
  // mock series (and therefore spot) changes with it.
  const [showPriceLines, setShowPriceLines] = useState(false);
  const [priceLines, setPriceLines] = useState<DemoPriceLine[]>([]);
  const togglePriceLines = useCallback(() => {
    setShowPriceLines((on) => {
      if (!on) setPriceLines(mockPriceLines(candles));
      return !on;
    });
  }, [candles]);
  const onPriceLineDragEnd = useCallback((id: string, price: number) => {
    Haptics.impactAsync(Haptics.ImpactFeedbackStyle.Light).catch(() => {});
    setPriceLines((prev) =>
      prev.map((l) =>
        l.id === id ? { ...l, price, text: priceLineText(l.label, price) } : l,
      ),
    );
  }, []);
  const onPriceLineClose = useCallback((id: string) => {
    Haptics.impactAsync(Haptics.ImpactFeedbackStyle.Medium).catch(() => {});
    setPriceLines((prev) => prev.filter((l) => l.id !== id));
  }, []);

  // Indicator enable/config state lives here so it can later drive the chart;
  // the menu only owns its own list↔detail navigation.
  const [menuOpen, setMenuOpen] = useState(false);
  const [indicators, setIndicators] = useState<IndicatorState>(
    DEFAULT_INDICATOR_STATE,
  );
  const [rsiParams, setRsiParams] = useState<RSIParams>(DEFAULT_RSI_PARAMS);
  const patchRsi = useCallback(
    (patch: Partial<RSIParams>) =>
      setRsiParams((prev) => ({ ...prev, ...patch })),
    [],
  );
  const [macdParams, setMacdParams] = useState<MACDParams>(DEFAULT_MACD_PARAMS);
  const patchMacd = useCallback(
    (patch: Partial<MACDParams>) =>
      setMacdParams((prev) => ({ ...prev, ...patch })),
    [],
  );

  // Moving-average overlay lines (ribbons). Each list is edited in its detail
  // screen and combined into the `movingAverages` prop below.
  const [maLines, setMaLines] = useState<MALineParams[]>([DEFAULT_MA_LINE]);
  const [emaLines, setEmaLines] = useState<MALineParams[]>([DEFAULT_EMA_LINE]);
  const maEditor = {
    lines: maLines,
    onChange: (i: number, patch: Partial<MALineParams>) =>
      setMaLines((p) => p.map((l, idx) => (idx === i ? { ...l, ...patch } : l))),
    onAdd: () => setMaLines((p) => [...p, DEFAULT_MA_LINE]),
    onRemove: (i: number) => setMaLines((p) => p.filter((_, idx) => idx !== i)),
  };
  const emaEditor = {
    lines: emaLines,
    onChange: (i: number, patch: Partial<MALineParams>) =>
      setEmaLines((p) => p.map((l, idx) => (idx === i ? { ...l, ...patch } : l))),
    onAdd: () => setEmaLines((p) => [...p, DEFAULT_EMA_LINE]),
    onRemove: (i: number) => setEmaLines((p) => p.filter((_, idx) => idx !== i)),
  };

  const [vwapParams, setVwapParams] = useState<VWAPParams>(DEFAULT_VWAP_PARAMS);
  const patchVwap = useCallback(
    (patch: Partial<VWAPParams>) =>
      setVwapParams((prev) => ({ ...prev, ...patch })),
    [],
  );

  const [bbParams, setBbParams] = useState<BollingerParams>(
    DEFAULT_BOLLINGER_PARAMS,
  );
  const patchBb = useCallback(
    (patch: Partial<BollingerParams>) =>
      setBbParams((prev) => ({ ...prev, ...patch })),
    [],
  );

  const toggleIndicator = useCallback((id: IndicatorId, enabled: boolean) => {
    setIndicators((prev) => ({ ...prev, [id]: { ...prev[id], enabled } }));
    // Seed one default line when enabling an empty MA/EMA group.
    if (enabled && id === 'ma') {
      setMaLines((p) => (p.length ? p : [DEFAULT_MA_LINE]));
    }
    if (enabled && id === 'ema') {
      setEmaLines((p) => (p.length ? p : [DEFAULT_EMA_LINE]));
    }
  }, []);
  const activeCount = enabledCount(indicators);

  const movingAverages: MovingAverageOverlay[] = [
    ...(indicators.ma.enabled
      ? maLines.map(
          (l): MovingAverageOverlay => ({
            maType: 'sma',
            period: l.period,
            source: l.source,
            color: l.color,
            width: l.width,
          }),
        )
      : []),
    ...(indicators.ema.enabled
      ? emaLines.map(
          (l): MovingAverageOverlay => ({
            maType: 'ema',
            period: l.period,
            source: l.source,
            color: l.color,
            width: l.width,
          }),
        )
      : []),
  ];

  return (
    <SafeAreaProvider>
      <GestureHandlerRootView style={styles.root}>
        <SafeAreaView style={styles.container}>
          <View style={styles.header}>
            <Text style={styles.title}>Vroom Test Bench</Text>
          </View>

          <View style={styles.ohlcv}>
            {shown ? (
              <>
                <Field label="O" value={shown.open.toFixed(2)} />
                <Field label="H" value={shown.high.toFixed(2)} />
                <Field label="L" value={shown.low.toFixed(2)} />
                <Field
                  label="C"
                  value={shown.close.toFixed(2)}
                  color={bull ? '#3fb950' : '#f85149'}
                />
                <Field label="V" value={fmtVol(shown.volume)} />
              </>
            ) : null}
          </View>

          {/* Remount on interval change so the visible window re-defaults to
              the new data's recent range. */}
          <VroomChart
            key={selected.label}
            candles={candles}
            style={styles.chart}
            onCrosshair={handleCrosshair}
            rsi={{ enabled: indicators.rsi.enabled, ...rsiParams }}
            macd={{ enabled: indicators.macd.enabled, ...macdParams }}
            movingAverages={movingAverages}
            vwap={{
              enabled: indicators.vwap.enabled,
              resetMinutes: vwapParams.resetHour * 60,
              color: vwapParams.color,
              width: vwapParams.width,
            }}
            bollingerBands={{
              enabled: indicators.bb.enabled,
              period: bbParams.period,
              stdDev: bbParams.stdDev,
              source: bbParams.source,
              maType: bbParams.maType,
              upperColor: bbParams.upperColor,
              upperWidth: bbParams.width,
              middleColor: bbParams.middleColor,
              middleWidth: bbParams.width,
              lowerColor: bbParams.lowerColor,
              lowerWidth: bbParams.width,
              fillVisible: bbParams.fillVisible,
              fillOpacity: bbParams.fillOpacity,
            }}
            priceLines={showPriceLines ? priceLines : undefined}
            onPriceLineDragEnd={onPriceLineDragEnd}
            onPriceLineClose={onPriceLineClose}
          />

          <View style={styles.footer}>
            <View style={styles.footerRow}>
              <IntervalSelect value={selected} onChange={setSelected} />

              <Pressable
                style={[styles.fnBtn, showPriceLines && styles.fnBtnActive]}
                onPress={togglePriceLines}
              >
                <Text
                  style={[
                    styles.fnSymbol,
                    showPriceLines && styles.fnSymbolActive,
                  ]}
                >
                  ⊞
                </Text>
              </Pressable>

              <Pressable
                style={styles.fnBtn}
                onPress={() => setMenuOpen(true)}
              >
                <Text
                  style={[
                    styles.fnSymbol,
                    activeCount > 0 && styles.fnSymbolActive,
                  ]}
                >
                  ƒ
                </Text>
                {activeCount > 0 ? (
                  <Text style={styles.fnCount}>{activeCount}</Text>
                ) : null}
              </Pressable>
            </View>
          </View>
        </SafeAreaView>

        <IndicatorsMenu
          visible={menuOpen}
          onClose={() => setMenuOpen(false)}
          state={indicators}
          onToggle={toggleIndicator}
          rsiParams={rsiParams}
          onRsiParamsChange={patchRsi}
          macdParams={macdParams}
          onMacdParamsChange={patchMacd}
          maEditor={maEditor}
          emaEditor={emaEditor}
          vwapParams={vwapParams}
          onVwapParamsChange={patchVwap}
          bbParams={bbParams}
          onBbParamsChange={patchBb}
        />

        <StatusBar style="light" />
      </GestureHandlerRootView>
    </SafeAreaProvider>
  );
}

const styles = StyleSheet.create({
  root: { flex: 1, backgroundColor: '#0d1117' },
  container: { flex: 1, backgroundColor: '#0d1117' },
  header: { paddingHorizontal: 16, paddingTop: 12, paddingBottom: 6 },
  title: { color: '#c9d1d9', fontSize: 18, fontWeight: '600' },
  ohlcv: {
    flexDirection: 'row',
    flexWrap: 'wrap',
    paddingHorizontal: 16,
    paddingBottom: 10,
    minHeight: 22,
  },
  ohlcvField: {
    flexDirection: 'row',
    alignItems: 'baseline',
    marginRight: 14,
  },
  ohlcvLabel: { color: '#8b949e', fontSize: 12, marginRight: 4 },
  ohlcvValue: {
    color: '#c9d1d9',
    fontSize: 13,
    fontWeight: '600',
    fontVariant: ['tabular-nums'],
  },
  chart: { flex: 1 },
  footer: {
    paddingVertical: 10,
    borderTopWidth: StyleSheet.hairlineWidth,
    borderTopColor: '#21262d',
  },
  footerRow: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'flex-end',
    gap: 8,
    paddingHorizontal: 12,
  },
  selectTrigger: {
    flexDirection: 'row',
    alignItems: 'center',
    height: 36,
    paddingHorizontal: 14,
    borderRadius: 8,
    backgroundColor: '#161b22',
    borderWidth: StyleSheet.hairlineWidth,
    borderColor: '#30363d',
  },
  selectValue: {
    color: '#c9d1d9',
    fontSize: 15,
    fontWeight: '600',
    marginRight: 6,
  },
  selectCaret: { color: '#8b949e', fontSize: 12 },
  fnBtn: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'center',
    minWidth: 48,
    height: 36,
    paddingHorizontal: 12,
    borderRadius: 8,
    backgroundColor: '#161b22',
  },
  fnSymbol: {
    color: '#8b949e',
    fontSize: 18,
    fontWeight: '600',
    fontStyle: 'italic',
    textAlign: 'center',
    includeFontPadding: false,
  },
  fnBtnActive: { backgroundColor: '#1f2a1f' },
  fnSymbolActive: { color: '#3fb950' },
  fnCount: {
    color: '#3fb950',
    fontSize: 11,
    fontWeight: '700',
    marginLeft: 3,
  },
  pickerBackdrop: { flex: 1, backgroundColor: 'rgba(0,0,0,0.4)' },
  pickerSheet: {
    backgroundColor: '#161b22',
    borderTopWidth: StyleSheet.hairlineWidth,
    borderTopColor: '#30363d',
  },
  pickerBar: {
    flexDirection: 'row',
    justifyContent: 'flex-end',
    paddingHorizontal: 16,
    paddingVertical: 10,
    borderBottomWidth: StyleSheet.hairlineWidth,
    borderBottomColor: '#21262d',
  },
  pickerDone: { color: '#58a6ff', fontSize: 16, fontWeight: '600' },
  pickerItem: { color: '#c9d1d9', fontSize: 18 },
});
