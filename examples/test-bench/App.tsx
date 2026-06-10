import { Picker } from '@react-native-picker/picker';
import * as Haptics from 'expo-haptics';
import { StatusBar } from 'expo-status-bar';
import { useCallback, useMemo, useState } from 'react';
import {
  Modal,
  Pressable,
  SafeAreaView,
  StyleSheet,
  Text,
  View,
} from 'react-native';
import { GestureHandlerRootView } from 'react-native-gesture-handler';
import {
  VroomChart,
  type Candle,
  type CrosshairEvent,
} from 'react-native-vroom-chart';

import {
  DEFAULT_INDICATOR_STATE,
  enabledCount,
  IndicatorsMenu,
  type IndicatorId,
  type IndicatorState,
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

  // Indicator enable/config state lives here so it can later drive the chart;
  // the menu only owns its own list↔detail navigation.
  const [menuOpen, setMenuOpen] = useState(false);
  const [indicators, setIndicators] = useState<IndicatorState>(
    DEFAULT_INDICATOR_STATE,
  );
  const [rsiPeriod, setRsiPeriod] = useState(14);
  const toggleIndicator = useCallback((id: IndicatorId, enabled: boolean) => {
    setIndicators((prev) => ({ ...prev, [id]: { ...prev[id], enabled } }));
  }, []);
  const activeCount = enabledCount(indicators);

  return (
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
          rsi={{ enabled: indicators.rsi.enabled, period: rsiPeriod }}
        />

        <View style={styles.footer}>
          <View style={styles.footerRow}>
            <IntervalSelect value={selected} onChange={setSelected} />

            <Pressable style={styles.fnBtn} onPress={() => setMenuOpen(true)}>
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
        rsiPeriod={rsiPeriod}
        onRsiPeriodChange={setRsiPeriod}
      />

      <StatusBar style="light" />
    </GestureHandlerRootView>
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
