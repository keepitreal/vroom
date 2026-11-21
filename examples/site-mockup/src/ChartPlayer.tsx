import {
  useCallback,
  useEffect,
  useImperativeHandle,
  useLayoutEffect,
  useRef,
  useState,
  type ReactNode,
  type Ref,
  type RefObject,
} from 'react';
import { VroomChart, type Candle, type VisibleRange, type VroomChartProps } from '@vroomchart/react';
import { toRange } from './ChartLayer';
import { dayChange, formatChange, formatPrice } from './market';

export type PlayerHandle = { open: () => void };

type Layout = 'closed' | 'full' | 'docked';

type Props = {
  ref: Ref<PlayerHandle>;
  candles: Candle[];
  /** The element the player grows out of and collapses back into. */
  origin: RefObject<HTMLElement | null>;
  reduceMotion: boolean;
  chartProps: Partial<VroomChartProps>;
  controls: ReactNode;
};

const EASE = 'cubic-bezier(0.2, 0.8, 0.2, 1)';
const STRIP_H = 40;

/** The docked card's size and where a full-viewport player lands when scaled into it. */
function dockGeometry() {
  const vw = innerWidth;
  const vh = innerHeight;
  const w = Math.min(340, vw - 32);
  const h = Math.round(w * 0.66);
  const x = vw - 16 - w;
  const y = vh - STRIP_H - 16 - h;
  const s = w / vw;
  const clipBottom = Math.max(0, vh - h / s);
  return {
    w,
    h,
    transform: `translate(${x}px, ${y}px) scale(${s})`,
    clipPath: `inset(0px 0px ${clipBottom}px 0px round ${12 / s}px)`,
  };
}

function insetOf(r: DOMRect) {
  return `inset(${r.top}px ${innerWidth - r.right}px ${innerHeight - r.bottom}px ${r.left}px round 2px)`;
}

const FULL_CLIP = 'inset(0px 0px 0px 0px round 0px)';

function originClip(origin: HTMLElement | null) {
  const r = origin?.getBoundingClientRect();
  if (r && r.bottom > 0 && r.top < innerHeight && r.width > 0) return insetOf(r);
  return `inset(${innerHeight * 0.3}px ${innerWidth * 0.3}px ${innerHeight * 0.3}px ${innerWidth * 0.3}px round 2px)`;
}

function Icon({ name }: { name: 'min' | 'max' | 'close' }) {
  const d = {
    min: 'M3 11h10',
    max: 'M9 3h4v4M7 13H3V9M13 3L9 7M3 13l4-4',
    close: 'M4 4l8 8M12 4l-8 8',
  }[name];
  return (
    <svg width="14" height="14" viewBox="0 0 16 16" aria-hidden>
      <path d={d} fill="none" stroke="currentColor" strokeWidth="1.5" strokeLinecap="square" />
    </svg>
  );
}

/**
 * The finale's playable chart. It grows out of `origin` to fill the viewport and
 * locks page scroll so gestures only move the chart. It can shrink into a
 * floating card while the visitor keeps scrolling, or collapse back into the page.
 */
export function ChartPlayer({ ref, candles, origin, reduceMotion, chartProps, controls }: Props) {
  const [layout, setLayout] = useState<Layout>('closed');
  /** Frames the opening view, then goes back to null so gestures and live ticks own the window. */
  const [range, setRange] = useState<VisibleRange | null>(null);
  const el = useRef<HTMLDivElement>(null);
  const closeBtn = useRef<HTMLButtonElement>(null);
  const opener = useRef<HTMLElement | null>(null);
  const enter = useRef<'grow' | 'expand' | null>(null);
  const busy = useRef(false);
  const settle = useRef<Animation | null>(null);
  const dur = reduceMotion ? 0 : 560;

  const last = candles[candles.length - 1];
  const change = dayChange(candles);

  const open = useCallback(() => {
    if (busy.current) return;
    if (layout === 'full') return;
    opener.current = document.activeElement as HTMLElement | null;
    enter.current = layout === 'docked' ? 'expand' : 'grow';
    if (layout === 'closed') {
      const bars = Math.max(40, Math.min(130, Math.round(innerWidth / 10)));
      setRange(toRange(last.timeMs, bars, Math.round(bars / 13)));
    }
    setLayout('full');
  }, [layout, last.timeMs]);

  useImperativeHandle(ref, () => ({ open }), [open]);

  useLayoutEffect(() => {
    settle.current?.cancel();
    settle.current = null;
    const node = el.current;
    const how = enter.current;
    enter.current = null;
    if (!node || layout !== 'full' || !how) return;
    busy.current = true;
    const dock = dockGeometry();
    const frames =
      how === 'grow'
        ? [{ clipPath: originClip(origin.current) }, { clipPath: FULL_CLIP }]
        : [
            { transform: dock.transform, clipPath: dock.clipPath },
            { transform: 'none', clipPath: FULL_CLIP },
          ];
    const anim = node.animate(frames, { duration: dur, easing: EASE });
    anim.onfinish = () => {
      busy.current = false;
      setRange(null);
      closeBtn.current?.focus({ preventScroll: true });
    };
  }, [layout, dur, origin]);

  const close = useCallback(() => {
    const node = el.current;
    if (!node || busy.current) return;
    busy.current = true;
    const done = () => {
      busy.current = false;
      setLayout('closed');
      opener.current?.focus({ preventScroll: true });
    };
    if (layout === 'docked') {
      const anim = node.animate(
        [{ opacity: 1, transform: 'none' }, { opacity: 0, transform: 'translateY(16px)' }],
        { duration: dur * 0.5, easing: EASE, fill: 'forwards' },
      );
      anim.onfinish = done;
      return;
    }
    const anim = node.animate(
      [
        { clipPath: FULL_CLIP, opacity: 1 },
        { clipPath: originClip(origin.current), opacity: 1, offset: 0.85 },
        { clipPath: originClip(origin.current), opacity: 0 },
      ],
      { duration: dur, easing: EASE, fill: 'forwards' },
    );
    anim.onfinish = done;
  }, [layout, dur, origin]);

  const minimize = useCallback(() => {
    const node = el.current;
    if (!node || busy.current) return;
    busy.current = true;
    const dock = dockGeometry();
    const anim = node.animate(
      [
        { transform: 'none', clipPath: FULL_CLIP },
        { transform: dock.transform, clipPath: dock.clipPath },
      ],
      { duration: dur, easing: EASE, fill: 'forwards' },
    );
    anim.onfinish = () => {
      busy.current = false;
      settle.current = anim;
      setLayout('docked');
      opener.current?.focus({ preventScroll: true });
    };
  }, [dur]);

  useEffect(() => {
    document.documentElement.classList.toggle('is-locked', layout === 'full');
    if (layout !== 'full') return;
    const onKey = (e: KeyboardEvent) => {
      if (e.key === 'Escape') close();
    };
    addEventListener('keydown', onKey);
    return () => removeEventListener('keydown', onKey);
  }, [layout, close]);

  useEffect(() => () => document.documentElement.classList.remove('is-locked'), []);

  if (layout === 'closed') return null;

  const docked = layout === 'docked';
  const dock = docked ? dockGeometry() : null;
  const quote = (
    <>
      <span className="player__sym">BTC-PERP · 1M</span>
      <span className="ink">{formatPrice(last.close)}</span>
      <span className={change >= 0 ? 'up' : 'down'}>{formatChange(change)}</span>
    </>
  );

  return (
    <div
      ref={el}
      className={`player${docked ? ' is-docked' : ''}`}
      style={dock ? { width: dock.w, height: dock.h } : undefined}
      role={docked ? 'region' : 'dialog'}
      aria-modal={docked ? undefined : true}
      aria-label="Live BTC chart"
      onClick={docked ? open : undefined}
    >
      {docked ? (
        <header className="player__dockbar mono">
          <span className="player__quote">{quote}</span>
          <span className="player__actions">
            <button
              className="pbtn pbtn--icon"
              aria-label="Expand chart"
              onClick={(e) => {
                e.stopPropagation();
                open();
              }}
            >
              <Icon name="max" />
            </button>
            <button
              className="pbtn pbtn--icon"
              aria-label="Close chart"
              onClick={(e) => {
                e.stopPropagation();
                close();
              }}
            >
              <Icon name="close" />
            </button>
          </span>
        </header>
      ) : (
        <header className="player__bar mono">
          <span className="player__quote">
            <i className="player__dot" aria-hidden />
            {quote}
          </span>
          <div className="player__controls">{controls}</div>
          <span className="player__actions">
            <button className="pbtn" onClick={minimize} aria-label="Minimize chart">
              <Icon name="min" />
              <span>Minimize</span>
            </button>
            <button ref={closeBtn} className="pbtn pbtn--close" onClick={close} aria-label="Close chart">
              <Icon name="close" />
              <span>Close</span>
              <kbd>Esc</kbd>
            </button>
          </span>
        </header>
      )}

      <div className="player__chart">
        <VroomChart
          candles={candles}
          seriesKey="BTC-PERP-player"
          reduceMotion={reduceMotion || undefined}
          autoscaleY
          {...chartProps}
          visibleRange={range ?? undefined}
        />
      </div>

      {!docked && (
        <p className="player__hint mono">
          {matchMedia('(pointer: coarse)').matches
            ? 'Drag to pan · pinch to zoom'
            : 'Drag to pan · scroll or pinch to zoom · Esc to close'}
        </p>
      )}
    </div>
  );
}
