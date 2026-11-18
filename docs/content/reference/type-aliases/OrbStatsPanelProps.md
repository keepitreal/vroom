# `OrbStatsPanelProps`

```ts
type OrbStatsPanelProps = {
  position?: "topLeft" | "topRight" | "bottomLeft" | "bottomRight";
  result: OrbResult;
  style?: StyleProp<ViewStyle>;
};
```

Source: react-native/src/OrbStatsPanel.tsx:6

## Properties

### position?

```ts
optional position?: "topLeft" | "topRight" | "bottomLeft" | "bottomRight";
```

Source: react-native/src/OrbStatsPanel.tsx:9

Corner of the parent to sit in. Default 'topLeft'.

---

### result

```ts
result: OrbResult;
```

Source: react-native/src/OrbStatsPanel.tsx:7

---

### style?

```ts
optional style?: StyleProp<ViewStyle>;
```

Source: react-native/src/OrbStatsPanel.tsx:11

Merged over the panel's own style.
