import { StatusBar } from 'expo-status-bar';
import { SafeAreaView, StyleSheet, Text, View, useWindowDimensions } from 'react-native';
import { VroomChart } from 'react-native-vroom-chart';

export default function App() {
  const { width } = useWindowDimensions();

  return (
    <SafeAreaView style={styles.container}>
      <View style={styles.header}>
        <Text style={styles.title}>vroom test bench</Text>
        <Text style={styles.subtle}>
          Phase 0 · expecting a red 100×100 rect from C++
        </Text>
      </View>

      <VroomChart candles={[]} width={width} height={300} />

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
