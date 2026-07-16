# Type Alias: VroomColor

```ts
type VroomColor = string | number;
```

Defined in: [types/src/index.ts:63](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L63)

A color value: a hex string (`'#0d1117'`, or 8-digit `'#aarrggbb'`) or a
packed ARGB number. In `VroomTheme` every field is optional — omitted colors
keep the library default.
