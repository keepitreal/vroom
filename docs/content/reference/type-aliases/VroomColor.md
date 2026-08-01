# Type Alias: VroomColor

```ts
type VroomColor = string | number;
```

Defined in: [types/src/index.ts:63](https://github.com/keepitreal/vroom/blob/3f6a67d6951af43fd8eed66e70bf2793c353bbc0/packages/types/src/index.ts#L63)

A color value: a hex string (`'#0d1117'`, or 8-digit `'#aarrggbb'`) or a
packed ARGB number. In `VroomTheme` every field is optional — omitted colors
keep the library default.
