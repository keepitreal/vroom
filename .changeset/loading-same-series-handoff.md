---
'@vroomchart/react': patch
'react-native-vroom-chart': patch
---

Hand the loading line off to the candles when the same `candles` array comes back after loading. Previously the push was skipped and the line kept waving.
