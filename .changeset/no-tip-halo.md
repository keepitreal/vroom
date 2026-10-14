---
'react-native-vroom-chart': patch
'@vroomchart/react': patch
'@vroomchart/core-wasm': patch
---

Drop the background-colored ring around the line chart's tip dot. On a dark background it read as a dark outline cutting into the line. The pulse now grows straight out of the dot's edge, and line mode's right gutter is 2px narrower since there is no ring to make room for.
