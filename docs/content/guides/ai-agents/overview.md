# Driving the chart with an AI agent

`@vroomchart/agent` lets a language model operate a chart the way a user would:
turn on indicators ("add RSI with 30 and 70 bands"), mark up the chart ("draw
support and resistance"), frame the view, and read back what's on screen.

It doesn't call a model or hold an API key. It gives your app the contract
between the model and the chart:

- **Tool definitions** as JSON Schema, ready for OpenAI, Anthropic, or an MCP
  server.
- **A validating reducer**, `applyAction`. It turns each tool call into a new
  JSON chart state. A bad call returns a readable error instead of throwing, so
  the model can correct itself.
- **Market context**: `describeChart` and `findSwingLevels`. They hand the model
  real prices and levels so it doesn't invent them.
- **A React hook**, `useAgentChart`, that holds the state and returns props for
  `<VroomChart>`.

It works with the web package, `@vroomchart/react`.

## Install

```sh
npm install @vroomchart/agent @vroomchart/react
```

## Wire up the chart

`useAgentChart` takes your candles and gives back props to spread into the
chart, the tool definitions to send to the model, and `run` to apply each tool
call:

```tsx
import { VroomChart } from "@vroomchart/react";
import { useAgentChart } from "@vroomchart/agent/react";

function Chart({ candles }) {
  const agent = useAgentChart({ candles });

  return <VroomChart candles={candles} {...agent.chartProps} />;
}
```

Pass `candles` yourself, and anything else you own, such as `theme` and
callbacks. `chartProps` covers the indicators, moving averages, display mode,
visible window, price levels, and drawings.

## Run the model loop

Each turn, send the user's message and `agent.tools` to the model. When it
calls a tool, pass the call to `agent.run` and return `result` as the tool
result. Repeat until the model replies with text.

This example uses the Anthropic Messages API. Call the model from your backend
so the API key never reaches the browser; `askModel` stands in for that request.

```tsx
import { toAnthropicTools } from "@vroomchart/agent";

async function send(agent, messages) {
  const tools = toAnthropicTools(agent.tools);
  for (;;) {
    // Your backend forwards { system, messages, tools } to the Messages API.
    const reply = await askModel({
      system: `You control a trading chart.\n\n${agent.describe()}`,
      messages,
      tools,
    });
    messages.push({ role: "assistant", content: reply.content });

    const calls = reply.content.filter((b) => b.type === "tool_use");
    if (calls.length === 0) return reply;

    const results = [];
    for (const call of calls) {
      const { result } = await agent.run(call.name, call.input);
      results.push({ type: "tool_result", tool_use_id: call.id, content: result });
    }
    messages.push({ role: "user", content: results });
  }
}
```

For OpenAI, use `toOpenAITools(agent.tools)` and read the calls from
`message.tool_calls`. The arguments arrive as a JSON string there, so pass
`JSON.parse(call.function.arguments)` to `run`.

Run calls one at a time, in order. Each call builds on the state the previous
one left, and `run` always reads the latest state, even before React has
re-rendered.

`agent.describe()` returns the same summary `get_market_context` starts with:
the interval, the window in view, price action, indicator readings, and what's
drawn. Putting it in the system prompt saves the model a tool call on simple
requests.

## Drawings and undo

By default the hook owns the drawings array. `chartProps` includes `drawings`
and the `onDrawing*` callbacks, so drawings the user makes and drawings the
agent makes live in the same list.

If the chart uses a [`drawingStore`](../drawing-tools.md#persisting-drawings),
pass its `historyRef` as `drawingControls`. The agent's drawing edits then go
through the chart's managed drawings: each one is an undo step and persists like
the user's own, so ⌘Z takes back what the agent drew.

```tsx
const historyRef = useRef<DrawingControls | null>(null);
const agent = useAgentChart({ candles, drawingControls: historyRef });

<VroomChart
  candles={candles}
  seriesKey={symbol}
  drawingStore={store}
  historyRef={historyRef}
  {...agent.chartProps}
/>;
```

## Your own tools

The chart doesn't own the data, so "switch to the 15 minute chart" is your
app's job. Register tools like that with `defineHostTool`. The schema is a
[zod](https://zod.dev) schema, and the return value is the result string the
model sees:

```tsx
import { z } from "zod";
import { defineHostTool } from "@vroomchart/agent";

const setTimeframe = defineHostTool({
  name: "set_timeframe",
  description: "Switch the chart to another candle interval.",
  schema: z.object({ interval: z.enum(["1m", "5m", "15m", "1h", "4h", "1d"]) }),
  execute: async ({ interval }) => {
    await loadCandles(symbol, interval);
    return `Switched to ${interval}.`;
  },
});

const agent = useAgentChart({ candles, hostTools: [setTimeframe] });
```

Host tools appear in `agent.tools` alongside the built-in ones. A host tool with
a built-in's name replaces it. An error thrown by `execute` comes back to the
model as a failed result.

## What the agent can't touch

- **Your price lines.** Pass orders, alerts, and other lines you own as the
  hook's `priceLines` option. They render with the agent's levels, but the agent
  never sees them and can't move or remove them.
- **Locked drawings.** The agent can't update or remove a drawing the user
  locked. `remove_drawings` skips locked drawings and says so in its result.
- **Anything outside the state.** The agent can only change indicators, moving
  averages, its own price levels, drawings, the visible window, and the display
  mode. Candles, theme, and callbacks stay with you.

Every drawing the agent creates carries `meta: { source: "agent" }`. That's how
"remove the lines you drew" leaves the user's own drawings alone, and how your
UI can style or list agent drawings separately.

## Without React

The tools and reducer are plain functions, so they also work on a server or in
an MCP tool handler:

```ts
import { applyAction, toolDefinitions } from "@vroomchart/agent";

let state = {};
const out = applyAction(
  state,
  { name: "set_indicators", args: { rsi: { lowerBand: 30, upperBand: 70 } } },
  { candles },
);
if (out.ok) state = out.state;
// out.result goes back to the model either way
```

`toolDefinitions()` returns `{ name, description, inputSchema }` objects, the
shape MCP's `tools/list` expects. `runTool` is the async version of
`applyAction` that also runs host tools.

See [Agent tools](./tools.md) for every tool and its arguments.
