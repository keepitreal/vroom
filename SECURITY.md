# Security policy

## Supported versions

Security fixes are applied to the latest published minor of each package:

| Package                   | Supported          |
| ------------------------- | ------------------ |
| `@vroomchart/react`       | latest minor (0.x) |
| `@vroomchart/core-wasm`   | latest minor (0.x) |
| `react-native-vroom-chart`| latest minor (0.x) |

## Reporting a vulnerability

Please **do not** open a public issue for security reports.

- Preferred: use GitHub's private reporting — the **"Report a vulnerability"**
  button under the repo's *Security* tab
  (https://github.com/keepitreal/vroom/security/advisories/new).
- Fallback: email darion.welch@gmail.com with the details.

You can expect an acknowledgement within a few days. Please allow up to 90
days of coordinated disclosure before publishing details; a fix and advisory
will normally ship much sooner.

## Scope

In scope: the runtime surface of the published packages — the React/React
Native wrappers, the WASM core and its JS boundary, and the drawing
serialization/storage paths that handle consumer-supplied data.

Out of scope: the `examples/` apps and `docs/` site (development-only
toolchains), and vulnerabilities that require a compromised development
machine or CI environment.
