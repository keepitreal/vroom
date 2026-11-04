import type { ChartFrame, ChartHandle } from './jsi.d';

/** States of a chart's UI-thread frame loop, kept in a SharedValue. */
export const LOOP_IDLE = 0;
export const LOOP_RUNNING = 1;
/** Its chart unmounted or was hidden: the loop draws nothing more until the chart shows again. */
export const LOOP_STOPPED = 2;

/**
 * Draws a frame on the UI thread, then one per display frame for as long as
 * the core is animating (the loading wave, the hand-off, the tip pulse, label
 * fades), so those keep moving however busy the JS thread is. A request while
 * the loop is already running is dropped: its next frame renders the core as
 * it is by then.
 */
export function drawUiFrames(
  handle: Pick<ChartHandle, 'render' | 'isAnimating'>,
  state: { value: number },
  present: (frame: ChartFrame) => void,
): void {
  'worklet';
  if (state.value !== LOOP_IDLE) return;
  state.value = LOOP_RUNNING;
  const tick = () => {
    if (state.value === LOOP_STOPPED) return;
    const frame = handle.render();
    if (frame) present(frame);
    if (handle.isAnimating()) requestAnimationFrame(tick);
    else state.value = LOOP_IDLE;
  };
  tick();
}
