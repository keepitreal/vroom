import { StatusBar } from 'expo-status-bar';
import { useMemo, useState } from 'react';
import {
  Pressable,
  SafeAreaView,
  StyleSheet,
  Text,
  View,
  useWindowDimensions,
} from 'react-native';
import { VroomChart, type Candle } from 'react-native-vroom-chart';

const TOTAL = 300;
const VISIBLE = 60;
const STEP = 10;

function mockCandles(n: number): Candle[] {
  const out: Candle[] = [];
  let price = 100;
  const now = Date.now();
  const minute = 60_000;
  for (let i = 0; i < n; i++) {
    const open = price;
    const close = open + (Math.random() - 0.5) * 4;
    const high = Math.max(open, close) + Math.random() * 2;
    const low = Math.min(open, close) - Math.random() * 2;
    out.push({
      timeMs: now - (n - i) * minute,
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
  const { width } = useWindowDimensions();
  const candles = useMemo(() => mockCandles(TOTAL), []);

  // Window endIdx is exclusive; window is candles[endIdx-VISIBLE .. endIdx).
  const [endIdx, setEndIdx] = useState(TOTAL);
  const startIdx = Math.max(0, endIdx - VISIBLE);
  const visibleRange = {
    startMs: candles[startIdx]!.timeMs,
    endMs: candles[endIdx - 1]!.timeMs,
  };

  const onLeft = () => setEndIdx((i) => Math.max(VISIBLE, i - STEP));
  const onRight = () => setEndIdx((i) => Math.min(TOTAL, i + STEP));

  return (
    <SafeAreaView style={styles.container}>
      <View style={styles.header}>
        <Text style={styles.title}>vroom test bench</Text>
        <Text style={styles.subtle}>
          Phase 2 · window {startIdx}–{endIdx} of {TOTAL}
        </Text>
      </View>

      <VroomChart
        candles={candles}
        width={width}
        height={300}
        visibleRange={visibleRange}
      />

      <View style={styles.controls}>
        <Pressable onPress={onLeft} style={styles.btn}>
          <Text style={styles.btnText}>← step</Text>
        </Pressable>
        <Pressable onPress={onRight} style={styles.btn}>
          <Text style={styles.btnText}>step →</Text>
        </Pressable>
      </View>

      <StatusBar style="light" />
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1, backgroundColor: '#0d1117' },
  header: { padding: 16 },
  title: { color: '#c9d1d9', fontSize: 18, fontWeight: '600' },
  subtle: { color: '#8b949e', fontSize: 12, marginTop: 4 },
  controls: {
    flexDirection: 'row',
    justifyContent: 'space-around',
    paddingHorizontal: 16,
    paddingTop: 16,
  },
  btn: {
    paddingVertical: 12,
    paddingHorizontal: 24,
    backgroundColor: '#21262d',
    borderRadius: 8,
  },
  btnText: { color: '#c9d1d9', fontSize: 16, fontWeight: '500' },
});
