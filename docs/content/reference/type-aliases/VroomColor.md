# `VroomColor`

```ts
type VroomColor = string | number;
```

Source: [types/src/index.ts:76](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L76)

A color value: a hex string (`'#0d1117'`, or 8-digit `'#aarrggbb'`) or a
packed ARGB number. In `VroomTheme` every field is optional — omitted colors
keep the library default.
