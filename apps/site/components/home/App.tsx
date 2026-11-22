"use client";

import Link from "next/link";
import { useEffect, useMemo, useRef, useState, type ReactNode } from "react";
import type {
  Candle,
  ChartMode,
  ChartType,
  Drawing,
  VroomChartProps,
} from "@vroomchart/react";
import { GITHUB_URL, NAV } from "@/lib/site";
import { ChartLayer } from "./ChartLayer";
import { ChartPlayer, type PlayerHandle } from "./ChartPlayer";
import { PhoneChart } from "./PhoneChart";
import { SplitFlap } from "./SplitFlap";
import {
  dayChange,
  formatChange,
  formatPrice,
  useBtcCandles,
  type FeedStatus,
} from "./market";
import {
  AGENT_STEPS,
  BRAND_STEPS,
  GREEN,
  INDICATOR_ROWS,
  INK,
  sceneFor,
  type SceneId,
} from "./scenes";

const ORDER: SceneId[] = [
  "hero",
  "speed",
  "core",
  "indicators",
  "draw",
  "trades",
  "brand",
  "agents",
  "mobile",
  "finale",
];

type ScrollPos = { id: SceneId; progress: number };

function useReducedMotion() {
  const [reduced, setReduced] = useState(false);
  useEffect(() => {
    const mq = matchMedia("(prefers-reduced-motion: reduce)");
    const on = () => setReduced(mq.matches);
    on();
    mq.addEventListener("change", on);
    return () => mq.removeEventListener("change", on);
  }, []);
  return reduced;
}

/** Which section owns the middle of the viewport, and how far through it we are. */
function useScrollPos(
  refs: React.RefObject<Partial<Record<SceneId, HTMLElement | null>>>,
) {
  const [pos, setPos] = useState<ScrollPos>({ id: "hero", progress: 0 });
  useEffect(() => {
    let raf = 0;
    const measure = () => {
      raf = 0;
      const mid = innerHeight * 0.5;
      let id: SceneId = "hero";
      let progress = 0;
      for (const sid of ORDER) {
        const el = refs.current?.[sid];
        if (!el) continue;
        const r = el.getBoundingClientRect();
        if (r.top <= mid) {
          id = sid;
          progress = Math.min(1, Math.max(0, (mid - r.top) / r.height));
        }
      }
      setPos((p) =>
        p.id === id && Math.abs(p.progress - progress) < 0.005
          ? p
          : { id, progress },
      );
    };
    const onScroll = () => {
      if (!raf) raf = requestAnimationFrame(measure);
    };
    measure();
    addEventListener("scroll", onScroll, { passive: true });
    addEventListener("resize", onScroll);
    return () => {
      removeEventListener("scroll", onScroll);
      removeEventListener("resize", onScroll);
      cancelAnimationFrame(raf);
    };
  }, [refs]);
  return pos;
}

const stepOf = (progress: number, count: number) =>
  Math.min(count - 1, Math.floor(progress * count));
const pad2 = (n: number) => String(n).padStart(2, "0");

function CopyInstall() {
  const [copied, setCopied] = useState(false);
  const cmd = "npm i @vroomchart/react";
  return (
    <button
      className="btn btn--ghost btn--sm copy"
      onClick={() => {
        void navigator.clipboard.writeText(cmd);
        setCopied(true);
        setTimeout(() => setCopied(false), 1500);
      }}
    >
      {copied ? "Copied" : cmd}
    </button>
  );
}

function LiveDot({ status }: { status: FeedStatus }) {
  const label =
    status === "live"
      ? "Live"
      : status === "connecting"
        ? "Connecting"
        : "Delayed";
  return (
    <span className={`live live--${status}`}>
      <i aria-hidden /> {label} · Hyperliquid BTC-PERP
    </span>
  );
}

function SpecTable({
  rows,
  active,
  compact,
}: {
  rows: [string, ReactNode][];
  active?: number;
  compact?: boolean;
}) {
  return (
    <dl className={`spec${compact ? " spec--compact" : ""}`}>
      {rows.map(([k, v], i) => (
        <div key={k} className={`spec__row${i === active ? " is-active" : ""}`}>
          <dt className="mono">
            {pad2(i + 1)} <span>{k}</span>
          </dt>
          <dd>{v}</dd>
        </div>
      ))}
    </dl>
  );
}

function Counter({ n, label }: { n: number; label: string }) {
  return (
    <p className="counter mono">
      {pad2(n)} / 10 <span>{label}</span>
    </p>
  );
}

function NavLink({
  href,
  children,
  onClick,
}: {
  href: string;
  children: ReactNode;
  onClick?: () => void;
}) {
  return href.startsWith("/") ? (
    <Link href={href} onClick={onClick}>
      {children}
    </Link>
  ) : (
    <a href={href} onClick={onClick}>
      {children}
    </a>
  );
}

export function App({ initialCandles }: { initialCandles: Candle[] }) {
  const { candles, status } = useBtcCandles(initialCandles);
  const reduceMotion = useReducedMotion();
  const refs = useRef<Partial<Record<SceneId, HTMLElement | null>>>({});
  const pos = useScrollPos(refs);
  const [menuOpen, setMenuOpen] = useState(false);
  const player = useRef<PlayerHandle>(null);
  const stage = useRef<HTMLButtonElement>(null);

  const [showIndicators, setShowIndicators] = useState(false);
  const [mode, setMode] = useState<ChartMode>("pan");
  const [chartType, setChartType] = useState<ChartType>("candles");
  const [userDrawings, setUserDrawings] = useState<Drawing[]>([]);

  const step =
    pos.id === "indicators"
      ? stepOf(pos.progress, INDICATOR_ROWS.length)
      : pos.id === "brand"
        ? stepOf(pos.progress, BRAND_STEPS.length)
        : pos.id === "agents"
          ? stepOf(pos.progress, AGENT_STEPS.length)
          : 0;
  const scene = useMemo(
    () => sceneFor(pos.id, step, candles),
    [pos.id, step, candles],
  );
  const playerBase = useMemo(
    () => sceneFor("finale", 0, candles).props,
    [candles],
  );

  const playerProps: Partial<VroomChartProps> = useMemo(() => {
    return {
      ...playerBase,
      chartType,
      mode,
      tool: mode === "draw" ? "line" : null,
      drawingStyle: { color: GREEN, width: 1.5 },
      drawings: userDrawings,
      onDrawingComplete: (d) => setUserDrawings((ds) => [...ds, d]),
      onDrawingChange: (d) =>
        setUserDrawings((ds) => ds.map((x) => (x.id === d.id ? d : x))),
      onDrawingDelete: (id) =>
        setUserDrawings((ds) => ds.filter((x) => x.id !== id)),
      onModeChange: setMode,
      ...(showIndicators
        ? {
            movingAverages: [
              {
                id: "ema21",
                maType: "ema",
                period: 21,
                color: INK,
                width: 1.25,
              },
            ],
            bollingerBands: {
              enabled: true,
              upperColor: "#8A8884",
              lowerColor: "#8A8884",
              middleColor: GREEN,
              fillOpacity: 0.06,
            },
            rsi: {
              enabled: true,
              lineColor: INK,
              maColor: GREEN,
              bandColor: "#3A3A3E",
              extremeFill: false,
            },
          }
        : {}),
    };
  }, [playerBase, chartType, mode, userDrawings, showIndicators]);

  const last = candles[candles.length - 1];
  const change = dayChange(candles);
  const n = ORDER.indexOf(pos.id) + 1;
  const reg = (id: SceneId) => (el: HTMLElement | null) => {
    refs.current[id] = el;
  };

  return (
    <div
      className={`home page scene--${pos.id}${pos.id === "brand" && step === 1 ? " is-paper" : ""}`}
    >
      <a className="skip mono" href="#main">
        Skip to content
      </a>

      <ChartLayer
        candles={candles}
        frame={scene.frame}
        pinching={pos.id === "speed"}
        reduceMotion={reduceMotion}
        chartProps={scene.props}
      />
      <div
        className="mask"
        style={{ ["--mask" as string]: scene.mask }}
        aria-hidden
      />

      <nav className="nav" aria-label="Site">
        <a className="nav__mark" href="#top" aria-label="vroom, home">
          <SplitFlap word="VROOM" size="sm" still />
        </a>
        <div className="nav__links mono">
          {NAV.map((l) => (
            <NavLink key={l.label} href={l.href}>
              {l.label}
            </NavLink>
          ))}
          <CopyInstall />
        </div>
        <button
          className="nav__menu mono"
          aria-expanded={menuOpen}
          onClick={() => setMenuOpen((o) => !o)}
        >
          {menuOpen ? "Close" : "Menu"}
        </button>
      </nav>
      {menuOpen && (
        <div className="menu mono" role="dialog" aria-label="Menu">
          {NAV.map((l) => (
            <NavLink
              key={l.label}
              href={l.href}
              onClick={() => setMenuOpen(false)}
            >
              {l.label}
            </NavLink>
          ))}
          <CopyInstall />
        </div>
      )}

      <main id="main">
        <section id="top" ref={reg("hero")} className="sec sec--hero">
          <p className="hero__tag mono" aria-hidden>
            BTC-PERP · 1M ·{" "}
            <span className="ink">{formatPrice(last.close)}</span>
          </p>
          <div className="hero__body">
            <h1>
              <SplitFlap word="VROOM" still={reduceMotion} />
              <span className="display">Charts that keep up.</span>
            </h1>
            <p className="lede">
              Pro-grade trading charts without the license fee. 60 FPS, MIT
              {"\u2011"}licensed, and built for AI agents.
            </p>
            <div className="ctas">
              <Link className="btn btn--solid" href="/docs">
                Read the docs
              </Link>
              <a className="btn btn--ghost" href={GITHUB_URL}>
                GitHub
              </a>
            </div>
          </div>
        </section>

        <section ref={reg("speed")} className="sec">
          <div className="col">
            <Counter n={2} label="Speed" />
            <h2 className="display">Sixty frames, every gesture.</h2>
            <p className="lede">
              Every gesture is handled by the C++ core, redraws through Skia in
              the same frame. Nothing waits on JavaScript and Canvas is nowhere
              to be found.
            </p>
            <SpecTable
              rows={[
                ["Gestures", "Pan, pinch, zoom, fling"],
                ["Frame rate", "60+ FPS on phones and in the browser"],
                [
                  "Crosshair",
                  "Snaps to candles and reports OHLCV for your readout",
                ],
              ]}
            />
          </div>
        </section>

        <section ref={reg("core")} className="sec">
          <div className="col">
            <Counter n={3} label="One core" />
            <h2 className="display">Write it once. Ship it everywhere.</h2>
            <p className="lede">
              A single Skia renderer in C++ draws the chart on iOS, Android, and
              in the browser through WebAssembly. Same props, same types, same
              pixels.
            </p>
          </div>
          <svg
            className="diagram mono"
            viewBox="0 0 520 260"
            role="img"
            aria-label="One C++ core feeds iOS, Android, and the web"
          >
            <rect x="170" y="10" width="180" height="56" />
            <text x="260" y="44">
              C++ CORE · SKIA
            </text>
            {[
              { x: 10, label: "iOS · JSI" },
              { x: 180, label: "ANDROID · JSI" },
              { x: 350, label: "WEB · WASM" },
            ].map((t) => (
              <g key={t.label}>
                <path d={`M260 66 V120 H${t.x + 80} V190`} />
                <rect x={t.x} y="190" width="160" height="50" />
                <text x={t.x + 80} y="220">
                  {t.label}
                </text>
              </g>
            ))}
          </svg>
        </section>

        <section ref={reg("indicators")} className="sec sec--tall">
          <div className="col sticky">
            <Counter n={4} label="Indicators" />
            <h2 className="display">Nine indicators, built in.</h2>
            <p className="lede">
              Computed in the core and drawn in the same frame as the candles,
              so they never lag behind.
            </p>
            <SpecTable
              compact
              rows={INDICATOR_ROWS.map((r) => [r, ""])}
              active={pos.id === "indicators" ? step : undefined}
            />
          </div>
        </section>

        <section ref={reg("draw")} className="sec">
          <div className="col">
            <Counter n={5} label="Drawing tools" />
            <h2 className="display">Mark it up.</h2>
            <p className="lede">
              Trendlines, boxes, and paths, with undo and redo. Drawings save
              per market, so they're still there after a refresh.
            </p>
            <SpecTable
              rows={[
                ["Tools", "Trendline, box, path"],
                ["History", "⌘Z and ⇧⌘Z, one step per drawing"],
                [
                  "Price lines",
                  "Draggable levels for orders, take-profit, and stop-loss",
                ],
              ]}
            />
          </div>
        </section>

        <section ref={reg("trades")} className="sec">
          <div className="col">
            <Counter n={6} label="Your trades" />
            <h2 className="display">See every fill.</h2>
            <p className="lede">
              Footprints mark where a position was entered and exited. Liquidity
              bands show resting orders behind the candles.
            </p>
            <SpecTable
              rows={[
                ["Footprints", "Entry and exit badges, grouped per candle"],
                ["Liquidity", "Order-book bands behind the price (web)"],
              ]}
            />
          </div>
        </section>

        <section ref={reg("brand")} className="sec sec--tall">
          <div className="col sticky">
            <Counter n={7} label="Theming" />
            <h2 className="display">Make it your own</h2>
            <p className="lede">
              vroom ships the chart, not the chrome around it. Colors, candle
              shapes, and every control around the chart are yours.
            </p>
            <SpecTable
              rows={BRAND_STEPS.map((s, i) => [
                s,
                [
                  "Dark, grayscale, one accent",
                  "Paper background, rounded bodies",
                  "Close-only line, green stroke",
                ][i],
              ])}
              active={pos.id === "brand" ? step : undefined}
            />
          </div>
        </section>

        <section ref={reg("agents")} className="sec sec--tall">
          <div className="col sticky">
            <Counter n={8} label="Agents" />
            <h2 className="display">Let a model drive.</h2>
            <p className="lede">
              <code>@vroomchart/agent</code> gives an LLM tools to add
              indicators, draw levels, and frame the view. It validates every
              call before the chart sees it.
            </p>
            <div className="code mono" aria-label="Agent tool calls">
              <div className="code__head">
                <span>npm i @vroomchart/agent</span>
              </div>
              <pre>
                {`const agent = useAgentChart({ candles });

<VroomChart candles={candles} {...agent.chartProps} />`}
              </pre>
              <ol className="code__log">
                {AGENT_STEPS.map((call, i) => {
                  const done =
                    pos.id === "agents"
                      ? i <= step
                      : ORDER.indexOf(pos.id) > ORDER.indexOf("agents");
                  return (
                    <li key={call} className={done ? "is-done" : undefined}>
                      <span>&gt; {call}</span>
                      <b>{done ? "OK" : "··"}</b>
                    </li>
                  );
                })}
              </ol>
            </div>
          </div>
        </section>

        <section ref={reg("mobile")} className="sec sec--split">
          <div className="col">
            <Counter n={9} label="Mobile" />
            <h2 className="display">Native on a phone.</h2>
            <p className="lede">
              The phone runs the same C++ core your React Native app ships. Go
              on, pinch it.
            </p>
            <SpecTable
              rows={[
                ["iOS", "JSI + Skia"],
                ["Android", "JSI + Skia"],
                ["Web", "WebAssembly"],
              ]}
            />
          </div>
          <div className="phone-wrap">
            <p className="phone-wrap__hint mono">Live · pinch me</p>
            <PhoneChart candles={candles} reduceMotion={reduceMotion} />
          </div>
        </section>

        <section ref={reg("finale")} className="sec sec--finale">
          <div className="finale__head">
            <Counter n={10} label="Your turn" />
            <h2 className="display">Test drive?</h2>
            <p className="lede">
              Try out vroom against live BTC Hyperliquid market data.
            </p>
            <div className="ctas">
              <button
                className="btn btn--solid"
                onClick={() => player.current?.open()}
              >
                Open the chart <span className="btn__icon" aria-hidden>⤢</span>
              </button>
              <Link
                className="btn btn--ghost"
                href="/docs/getting-started/quickstart"
              >
                Start with the Quickstart →
              </Link>
            </div>
          </div>
          <button
            ref={stage}
            className="stage"
            onClick={() => player.current?.open()}
            aria-label="Open the live chart full screen"
          >
            <i className="stage__corner" />
            <i className="stage__corner" />
            <i className="stage__corner" />
            <i className="stage__corner" />
            <span className="stage__label mono">⤢ Click to play</span>
          </button>
        </section>
      </main>

      <ChartPlayer
        ref={player}
        candles={candles}
        origin={stage}
        reduceMotion={reduceMotion}
        chartProps={playerProps}
        controls={
          <div
            className="toggles mono"
            role="group"
            aria-label="Chart controls"
          >
            <button
              aria-pressed={showIndicators}
              onClick={() => setShowIndicators((v) => !v)}
            >
              Indicators
            </button>
            <button
              aria-pressed={mode === "draw"}
              onClick={() => setMode((m) => (m === "draw" ? "pan" : "draw"))}
            >
              Draw
            </button>
            <button
              aria-pressed={chartType === "line"}
              onClick={() =>
                setChartType((t) => (t === "candles" ? "line" : "candles"))
              }
            >
              Line
            </button>
          </div>
        }
      />

      <footer className="footer">
        <div className="footer__cols">
          <div>
            <SplitFlap word="VROOM" size="sm" still />
            <p className="footer__blurb">
              Candlestick charts that keep up. MIT licensed.
            </p>
          </div>
          <div>
            <h4 className="mono">Docs</h4>
            <Link href="/docs">Introduction</Link>
            <Link href="/docs/getting-started/quickstart">Quickstart</Link>
            <Link href="/docs/guides/indicators">Guides</Link>
            <Link href="/docs/reference">API reference</Link>
          </div>
          <div>
            <h4 className="mono">Packages</h4>
            <a href="https://www.npmjs.com/package/react-native-vroom-chart">
              react-native-vroom-chart
            </a>
            <a href="https://www.npmjs.com/package/@vroomchart/react">
              @vroomchart/react
            </a>
            <a href="https://www.npmjs.com/package/@vroomchart/core-wasm">
              @vroomchart/core-wasm
            </a>
            <a href="https://www.npmjs.com/package/@vroomchart/agent">
              @vroomchart/agent
            </a>
          </div>
          <div>
            <h4 className="mono">Project</h4>
            <a href={GITHUB_URL}>GitHub</a>
            <a href={`${GITHUB_URL}/releases`}>Changelog</a>
            <a href={`${GITHUB_URL}/blob/main/LICENSE`}>License: MIT</a>
            <a href="#top">Back to top ↑</a>
          </div>
        </div>
        <p className="footer__fine mono">Market data: Hyperliquid public API</p>
      </footer>

      <div className="strip mono" aria-live="off">
        <span>{pad2(n)} / 10</span>
        <span className="strip__quote">
          <span className="ink">{formatPrice(last.close)}</span>
          <span className={change >= 0 ? "up" : "down"}>
            {formatChange(change)}
          </span>
          <LiveDot status={status} />
        </span>
      </div>
    </div>
  );
}
