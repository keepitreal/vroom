import { usePanGesture, usePinchGesture, useLongPressGesture, useTapGesture } from 'react-native-gesture-handler';
import { runOnJS, useSharedValue } from 'react-native-reanimated';
import type { ChartHandle } from './jsi.d';
import { beginPinch, updatePinch, panRegion, startUiPanDecay, type PanRegion } from './uiGestureState';

type CrosshairInfo = ReturnType<ChartHandle['getCrosshairInfo']>;
type Options = {
  enabled: boolean;
  handle: ChartHandle | null;
  draw: (() => void) | null;
  width: number;
  height: number;
  autoscaleY: boolean;
  crosshairEnabled: boolean;
  crosshairOffset: number;
  crosshairActive: { value: boolean };
  epoch: { value: number };
  notifyCrosshair: (info: CrosshairInfo, reason: 'show' | 'move' | 'hide') => void;
  notifyViewport: () => void;
};

/** Basic chart interactions. Interactive orders/footprints retain their JS path. */
export function useUiChartGestures({
  enabled, handle, draw, width, height, autoscaleY, crosshairEnabled,
  crosshairOffset, crosshairActive, epoch, notifyCrosshair, notifyViewport,
}: Options) {
  const mode = useSharedValue<PanRegion>('chart');
  const pinchState = useSharedValue(beginPinch({ x: 0, y: 0 }, { x: 0, y: 0 }));
  const crosshairKey = useSharedValue<string | null>(null);

  const pan = usePanGesture({
    enabled, runOnJS: false, maxPointers: 1,
    onActivate: (e) => {
      'worklet';
      epoch.value++;
      if (!handle) return;
      mode.value = panRegion(handle.getAxisMetrics(), width, height, e.x, e.y, autoscaleY);
    },
    onUpdate: (e) => {
      'worklet';
      if (!handle || !draw) return;
      if (mode.value === 'price-axis') handle.scalePriceAxis(e.changeY);
      else if (mode.value === 'time-axis') handle.scaleTimeAxis(e.changeX);
      else if (crosshairActive.value) {
        handle.setCrosshair(e.x, e.y - crosshairOffset);
        const info = handle.getCrosshairInfo();
        const key = `${info?.timeMs}|${info?.price}|${info?.indicatorValue}`;
        if (key !== crosshairKey.value) {
          crosshairKey.value = key;
          runOnJS(notifyCrosshair)(info, 'move');
        }
      } else if (mode.value === 'indicator') handle.pan(e.changeX, 0);
      else handle.translate(e.changeX, e.changeY);
      draw();
    },
    onDeactivate: (e) => {
      'worklet';
      if (!handle || !draw) return;
      if (crosshairActive.value && (mode.value === 'chart' || mode.value === 'indicator')) return;
      runOnJS(notifyViewport)();
      if (mode.value === 'chart' || mode.value === 'indicator') {
        startUiPanDecay(handle, epoch, draw, e.velocityX);
      }
    },
  });
  const pinch = usePinchGesture({
    enabled, runOnJS: false,
    onTouchesDown: (e) => {
      'worklet';
      if (e.numberOfTouches < 2) return;
      epoch.value++;
      const [a, b] = e.allTouches;
      pinchState.value = beginPinch(a, b);
    },
    onTouchesMove: (e) => {
      'worklet';
      if (!handle || !draw || crosshairActive.value || e.numberOfTouches < 2) return;
      const [a, b] = e.allTouches;
      const step = updatePinch(pinchState.value, a, b, autoscaleY);
      pinchState.value = step.next;
      if (step.scaleX === 1 && step.scaleY === 1) return;
      handle.zoom(step.scaleX, step.scaleY, step.x, step.y);
      draw();
    },
  });
  const longPress = useLongPressGesture({
    enabled: enabled && crosshairEnabled, runOnJS: false,
    onActivate: (e) => {
      'worklet';
      if (!handle || !draw) return;
      const region = panRegion(handle.getAxisMetrics(), width, height, e.x, e.y, false);
      if (region !== 'chart' && region !== 'indicator') return;
      epoch.value++;
      crosshairActive.value = true;
      handle.setCrosshair(e.x, e.y - crosshairOffset);
      draw();
      const info = handle.getCrosshairInfo();
      crosshairKey.value = `${info?.timeMs}|${info?.price}|${info?.indicatorValue}`;
      runOnJS(notifyCrosshair)(info, 'show');
    },
  });
  const tap = useTapGesture({
    enabled, runOnJS: false,
    // Android has no default distance limit: a quick drag can also be a tap.
    maxDistance: 10,
    onActivate: (e) => {
      'worklet';
      if (!handle || !draw || !crosshairActive.value) return;
      const region = panRegion(handle.getAxisMetrics(), width, height, e.x, e.y, false);
      if (region !== 'chart' && region !== 'indicator') return;
      crosshairActive.value = false;
      crosshairKey.value = null;
      handle.clearCrosshair();
      draw();
      runOnJS(notifyCrosshair)(null, 'hide');
    },
  });
  return { pan, pinch, longPress, tap };
}
