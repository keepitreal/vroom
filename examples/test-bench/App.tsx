import { StatusBar } from 'expo-status-bar';
import { useMemo } from 'react';
import { SafeAreaView, StyleSheet, Text, View, useWindowDimensions } from 'react-native';
import { VroomChart, type Candle } from 'react-native-vroom-chart';

function mockCandles(n = 150): Candle[] {
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
  const candles = useMemo(() => mockCandles(150), []);

  return (
    <SafeAreaView style={styles.container}>
      <View style={styles.header}>
        <Text style={styles.title}>vroom test bench</Text>
        <Text style={styles.subtle}>Phase 1 · {candles.length} candles from C++</Text>
      </View>

      <VroomChart candles={candles} width={width} height={300} />

      <StatusBar style="light" />
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1, backgroundColor: '#0d1117' },
  header: { padding: 16 },
  title: { color: '#c9d1d9', fontSize: 18, fontWeight: '600' },
  subtle: { color: '#8b949e', fontSize: 12, marginTop: 4 },
});
