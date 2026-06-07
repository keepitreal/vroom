import { StatusBar } from 'expo-status-bar';
import { useMemo, useState } from 'react';
import { Pressable, SafeAreaView, StyleSheet, Text, View } from 'react-native';
import { VroomChart, type Candle } from 'react-native-vroom-chart';

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

export default function App() {
  const [selected, setSelected] = useState<Interval>(INTERVALS[0]);
  const candles = useMemo(() => mockCandles(1000, selected.ms), [selected]);

  return (
    <SafeAreaView style={styles.container}>
      <View style={styles.header}>
        <Text style={styles.title}>Vroom Test Bench</Text>
      </View>

      {/* Remount on interval change so the visible window re-defaults to the
          new data's recent range. */}
      <VroomChart key={selected.label} candles={candles} style={styles.chart} />

      <View style={styles.footer}>
        {INTERVALS.map((it) => {
          const active = it.label === selected.label;
          return (
            <Pressable
              key={it.label}
              onPress={() => setSelected(it)}
              style={[styles.intervalBtn, active && styles.intervalBtnActive]}
            >
              <Text
                style={[styles.intervalText, active && styles.intervalTextActive]}
              >
                {it.label}
              </Text>
            </Pressable>
          );
        })}
      </View>

      <StatusBar style="light" />
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1, backgroundColor: '#0d1117' },
  header: { paddingHorizontal: 16, paddingVertical: 12 },
  title: { color: '#c9d1d9', fontSize: 18, fontWeight: '600' },
  chart: { flex: 1 },
  footer: {
    flexDirection: 'row',
    paddingVertical: 8,
    borderTopWidth: StyleSheet.hairlineWidth,
    borderTopColor: '#21262d',
  },
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
});
