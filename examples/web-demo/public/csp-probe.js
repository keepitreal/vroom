// CSP self-check for visual QA. Loaded by main.tsx as a same-origin <script>,
// which means it is subject to the page's CSP — unlike anything you type into
// the DevTools console, which runs with a CSP bypass and will happily evaluate
// `new Function` even when the page itself cannot.
//
// The chart core must never need eval: @vroomchart/core-wasm imports the
// emscripten module with a plain dynamic import(), and that module is built with
// -sDYNAMIC_EXECUTION=0 so embind/emval don't codegen invokers at runtime.
try {
  new Function('return 1')();
  window.__evalState = 'ALLOWED';
  console.warn(
    '[csp] eval is ALLOWED — this page is not enforcing a no-unsafe-eval policy, ' +
      'so it will not catch an eval regression in the chart core.',
  );
} catch (e) {
  window.__evalState = 'BLOCKED:' + e.constructor.name;
  console.info(
    `[csp] eval is BLOCKED (${e.constructor.name}) — the chart is running under a ` +
      'production-shaped CSP. If it still renders, it is eval-free.',
  );
}
