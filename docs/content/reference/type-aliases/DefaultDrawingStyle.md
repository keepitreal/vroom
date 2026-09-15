# `DefaultDrawingStyle`

```ts
type DefaultDrawingStyle = {
  color?: VroomColor;
  fill?: VroomColor;
  width?: number;
};
```

Source: [types/src/index.ts:364](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L364)

Default appearance for drawings the user creates — the live draft and the
object handed to `onDrawingComplete`. Paste copies the source drawing's
style instead of this default.

Omitted fields keep the library defaults: a 2px `#ff2962ff` stroke, and for
boxes a 10% tint of the stroke as fill.

Prefer 6-digit hex for `color` (`'#00FFFF'`). vroom treats that as opaque,
and CSS swatches preview it correctly. 8-digit hex is `#aarrggbb`, not CSS
`#rrggbbaa`.

## Properties

### color?

```ts
optional color?: VroomColor;
```

Source: [types/src/index.ts:366](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L366)

Stroke color, used for the live draft and stamped onto the committed drawing.

---

### fill?

```ts
optional fill?: VroomColor;
```

Source: [types/src/index.ts:373](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L373)

Box interior fill. Omitted, new boxes keep the default 10% stroke tint.
Ignored by line, pencil, and path.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:368](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L368)

Stroke width in px. Default 2.
