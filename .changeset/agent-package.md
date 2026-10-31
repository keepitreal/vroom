---
"@vroomchart/agent": minor
"@vroomchart/react": patch
---

New package `@vroomchart/agent` for driving a chart from AI agents: JSON Schema tool definitions (OpenAI, Anthropic, MCP), a validating `applyAction` reducer over a JSON `ChartState`, `describeChart` and `findSwingLevels` for market context, and a `useAgentChart` React hook (`@vroomchart/agent/react`).

`@vroomchart/react`: the managed-drawing controls on `historyRef` (`add`, `update`, `remove`, `list`) now see their own edits immediately when called several times before a render.
