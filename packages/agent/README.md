# @vroomchart/agent

Let an AI agent drive a [vroom](https://github.com/keepitreal/vroom) chart: add indicators, draw levels and zones, frame the view, and read back what's on screen.

The package doesn't call a model. It gives you the contract:

- **Tool definitions** (JSON Schema) for OpenAI, Anthropic, or any MCP-style client.
- **`applyAction`**, a pure reducer that validates a tool call and applies it to a JSON `ChartState`. Bad arguments come back as readable errors the model can correct.
- **`describeChart` and `findSwingLevels`**, so the model works from real prices instead of guessing.
- **`useAgentChart`** (`@vroomchart/agent/react`), which holds the state and returns props for `<VroomChart>`.

```sh
npm install @vroomchart/agent @vroomchart/react
```

## React

```tsx
import { VroomChart } from '@vroomchart/react';
import { toAnthropicTools } from '@vroomchart/agent';
import { useAgentChart } from '@vroomchart/agent/react';

function Chart({ candles }) {
  const agent = useAgentChart({ candles });

  async function onToolCall(name: string, input: unknown) {
    const { result } = await agent.run(name, input);
    return result; // send back to the model as the tool result
  }

  // Send toAnthropicTools(agent.tools) with your model request.
  return <VroomChart candles={candles} {...agent.chartProps} />;
}
```

With a `drawingStore`, pass the chart's `historyRef` as `drawingControls`. Agent drawings then go through the chart's undo history, so the user can undo them like their own.

## Without React

```ts
import { applyAction, toolDefinitions, toOpenAITools } from '@vroomchart/agent';

const tools = toOpenAITools(toolDefinitions());
let state = {};
const out = applyAction(state, { name: 'set_indicators', args: { rsi: { lowerBand: 30, upperBand: 70 } } }, { candles });
state = out.state; // out.result goes back to the model
```

## Tools

| Tool | What it does |
| --- | --- |
| `set_indicators` | RSI, MACD, ATR, VWAP, Bollinger Bands, Ichimoku, fair value gaps, volume |
| `add_moving_average` / `remove_moving_average` | SMA and EMA lines |
| `add_drawing` / `update_drawing` / `remove_drawings` | Trendlines, boxes (zones), arrowed paths |
| `add_price_level` / `remove_price_levels` | Labelled horizontal levels |
| `set_viewport` | Frame the last N bars or a time window |
| `set_chart_display` | Candles or line, linear or log scale |
| `get_chart_state` / `get_market_context` | Read the configuration, price action, and swing levels |

Drawing points take a price plus `timeMs` or `barsAgo` (0 is the latest candle; negative reaches into the future). Agent drawings are tagged `meta.source: 'agent'`. The agent can't change drawings the user locked, and it never sees or edits the price lines you pass in yourself.

Add your own tools, such as switching symbol or timeframe, with `defineHostTool` and pass them as `hostTools`.
