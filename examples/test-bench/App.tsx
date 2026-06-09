import {
  BottomSheetBackdrop,
  BottomSheetModal,
  BottomSheetModalProvider,
  BottomSheetView,
  type BottomSheetBackdropProps,
} from '@gorhom/bottom-sheet';
import * as Haptics from 'expo-haptics';
import { StatusBar } from 'expo-status-bar';
import { useCallback, useMemo, useRef, useState } from 'react';
import { Pressable, SafeAreaView, StyleSheet, Text, View } from 'react-native';
import { GestureHandlerRootView } from 'react-native-gesture-handler';
import {
  VroomChart,
  type Candle,
  type CrosshairEvent,
} from 'react-native-vroom-chart';

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

  const indicatorsSheetRef = useRef<BottomSheetModal>(null);
  const snapPoints = useMemo(() => ['50%'], []);
  const openIndicators = useCallback(() => {
    indicatorsSheetRef.current?.present();
  }, []);

  const renderBackdrop = useCallback(
    (props: BottomSheetBackdropProps) => (
      <BottomSheetBackdrop
        {...props}
        appearsOnIndex={0}
        disappearsOnIndex={-1}
        pressBehavior="close"
      />
    ),
    [],
  );

  return (
    <GestureHandlerRootView style={styles.root}>
      <BottomSheetModalProvider>
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
          />

          <View style={styles.footer}>
            <View style={styles.intervalRow}>
              {INTERVALS.map((it) => {
                const active = it.label === selected.label;
                return (
                  <Pressable
                    key={it.label}
                    onPress={() => setSelected(it)}
                    style={[
                      styles.intervalBtn,
                      active && styles.intervalBtnActive,
                    ]}
                  >
                    <Text
                      style={[
                        styles.intervalText,
                        active && styles.intervalTextActive,
                      ]}
                    >
                      {it.label}
                    </Text>
                  </Pressable>
                );
              })}
            </View>

            <Pressable style={styles.indicatorsBtn} onPress={openIndicators}>
              <Text style={styles.indicatorsText}>Indicators</Text>
            </Pressable>
          </View>
        </SafeAreaView>

        <BottomSheetModal
          ref={indicatorsSheetRef}
          snapPoints={snapPoints}
          backdropComponent={renderBackdrop}
          backgroundStyle={styles.sheetBackground}
          handleIndicatorStyle={styles.sheetHandle}
        >
          <BottomSheetView style={styles.sheetContent}>
            <Text style={styles.sheetTitle}>Add indicators</Text>
          </BottomSheetView>
        </BottomSheetModal>

        <StatusBar style="light" />
      </BottomSheetModalProvider>
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
    paddingVertical: 8,
    borderTopWidth: StyleSheet.hairlineWidth,
    borderTopColor: '#21262d',
  },
  intervalRow: { flexDirection: 'row' },
  intervalBtn: {
    flex: 1,
    alignItems: 'center',
    paddingVertical: 8,
    marginHorizontal: 4,
    borderRadius: 6,
  },
  intervalBtnActive: { backgroundColor: '#21262d' },
  intervalText: { color: '#8b949e', fontSize: 14, fontWeight: '500' },
  intervalTextActive: { color: '#c9d1d9' },
  indicatorsBtn: {
    marginTop: 8,
    marginHorizontal: 4,
    alignItems: 'center',
    paddingVertical: 12,
    borderRadius: 6,
    borderWidth: StyleSheet.hairlineWidth,
    borderColor: '#30363d',
    backgroundColor: '#161b22',
  },
  indicatorsText: { color: '#c9d1d9', fontSize: 15, fontWeight: '600' },
  sheetBackground: { backgroundColor: '#161b22' },
  sheetHandle: { backgroundColor: '#484f58' },
  sheetContent: { flex: 1, paddingHorizontal: 16, paddingTop: 8 },
  sheetTitle: { color: '#c9d1d9', fontSize: 18, fontWeight: '600' },
});
