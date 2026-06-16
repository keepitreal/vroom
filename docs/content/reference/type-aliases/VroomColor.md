# Type Alias: VroomColor

```ts
type VroomColor = string | number;
```

Defined in: [types.ts:41](https://github.com/keepitreal/vroom/blob/5754f44f04cecef0138d960a9d6cb9811c7231fa/packages/react-native/src/types.ts#L41)

A color value: a hex string (`'#0d1117'`, or 8-digit `'#aarrggbb'`) or a
packed ARGB number. In `VroomTheme` every field is optional — omitted colors
keep the library default.
