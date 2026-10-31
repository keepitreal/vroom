// Zod schemas for every tool's arguments. They cover the settings an agent
// plausibly needs (periods, levels, sources, colors) rather than every styling
// knob, and reject out-of-range values instead of clamping, so a model gets a
// readable error it can correct.

import { z } from 'zod';
import type {
  ATRConfig,
  BollingerBandsConfig,
  FairValueGapsConfig,
  IchimokuConfig,
  MACDConfig,
  MovingAverageOverlay,
  PriceLine,
  RSIConfig,
  VolumeConfig,
  VWAPConfig,
} from '@vroomchart/types';

const color = z
  .string()
  .regex(/^#(?:[0-9a-fA-F]{6}|[0-9a-fA-F]{8})$/)
  .describe('Hex color. Use 6 digits (#rrggbb); 8 digits are read as #aarrggbb, alpha first.');
const period = (min: number) => z.number().int().min(min).max(1000);
const width = z.number().min(0.5).max(10).describe('Stroke width in px.');
const unit = z.number().min(0).max(1);
const maSource = z.enum(['close', 'open', 'high', 'low', 'hl2', 'hlc3', 'ohlc4']);
const maKind = z.enum(['sma', 'ema']);
const enabled = z
  .boolean()
  .describe('Show or hide the indicator. Defaults to true whenever you pass the indicator.');

export const rsiSchema = z.strictObject({
  enabled: enabled.optional(),
  period: period(2).optional().describe('Lookback in candles. Default 14.'),
  upperBand: z.number().min(0).max(100).optional().describe('Overbought level. Default 70.'),
  lowerBand: z.number().min(0).max(100).optional().describe('Oversold level. Default 30.'),
  maPeriod: period(1).optional().describe('Length of the moving-average trendline over RSI. Default 14.'),
  maType: maKind.optional(),
  maVisible: z.boolean().optional().describe('Draw the trendline. Default true.'),
  bandsVisible: z.boolean().optional().describe('Draw the overbought/oversold rules. Default true.'),
  extremeFill: z.boolean().optional().describe('Shade where RSI is past a band. Default true.'),
  lineColor: color.optional(),
  maColor: color.optional(),
  bandColor: color.optional(),
});

export const macdSchema = z.strictObject({
  enabled: enabled.optional(),
  fast: period(1).optional().describe('Fast MA length. Default 12.'),
  slow: period(2).optional().describe('Slow MA length, greater than fast. Default 26.'),
  signal: period(1).optional().describe('Signal line length. Default 9.'),
  source: maSource.optional(),
  maType: maKind.optional().describe("Averaging for the fast/slow legs. Default 'ema'."),
  signalMaType: maKind.optional(),
  histogramVisible: z.boolean().optional(),
  lineColor: color.optional(),
  signalColor: color.optional(),
});

export const atrSchema = z.strictObject({
  enabled: enabled.optional(),
  period: period(1).optional().describe('Lookback in candles. Default 14.'),
  smoothing: z.enum(['rma', 'sma', 'ema']).optional().describe("Default 'rma' (Wilder)."),
  lineColor: color.optional(),
});

export const vwapSchema = z.strictObject({
  enabled: enabled.optional(),
  resetMinutes: z
    .number()
    .int()
    .min(0)
    .max(1439)
    .optional()
    .describe('Session reset, minutes after UTC midnight. Default 0.'),
  color: color.optional(),
  width: width.optional(),
});

export const bollingerSchema = z.strictObject({
  enabled: enabled.optional(),
  period: period(1).optional().describe('Lookback in candles. Default 20.'),
  stdDev: z.number().min(0.1).max(10).optional().describe('Band width in standard deviations. Default 2.'),
  source: maSource.optional(),
  maType: maKind.optional(),
  fillVisible: z.boolean().optional(),
  fillOpacity: unit.optional(),
});

export const ichimokuSchema = z.strictObject({
  enabled: enabled.optional(),
  tenkanPeriod: period(1).optional().describe('Default 9.'),
  kijunPeriod: period(1).optional().describe('Default 26.'),
  senkouBPeriod: period(1).optional().describe('Default 52.'),
  displacement: z.number().int().min(0).max(500).optional().describe('Default 26.'),
  cloudVisible: z.boolean().optional(),
});

export const fairValueGapsSchema = z.strictObject({
  enabled: enabled.optional(),
  maxBarsBack: z.number().int().min(0).max(5000).optional().describe('Default 300.'),
  fillType: z.enum(['close', 'wick']).optional(),
  deleteAfterFill: z.boolean().optional(),
  extendBoxes: z.boolean().optional(),
  boxLength: period(1).optional(),
  showLabels: z.boolean().optional(),
  showInverse: z.boolean().optional().describe('Keep filled gaps as inverse zones.'),
});

export const volumeSchema = z.strictObject({
  enabled: z.boolean().optional().describe('Volume bars are on by default; pass false to hide them.'),
  opacity: unit.optional(),
  height: unit.optional().describe('Tallest bar as a fraction of the price pane. Default 0.2.'),
});

export const setIndicatorsSchema = z.strictObject({
  rsi: rsiSchema.optional().describe('Relative Strength Index, in its own pane.'),
  macd: macdSchema.optional().describe('MACD, in its own pane.'),
  atr: atrSchema.optional().describe('Average True Range, in its own pane.'),
  vwap: vwapSchema.optional().describe('Session VWAP line on the price pane.'),
  bollingerBands: bollingerSchema.optional(),
  ichimoku: ichimokuSchema.optional(),
  fairValueGaps: fairValueGapsSchema.optional(),
  volume: volumeSchema.optional(),
});

export const movingAverageSchema = z.strictObject({
  id: z
    .string()
    .min(1)
    .max(64)
    .optional()
    .describe('Stable id. Reusing an id replaces that line. Default `<maType>-<period>`.'),
  maType: maKind,
  period: period(1),
  source: maSource.optional(),
  color: color.optional(),
  width: width.optional(),
});

export const removeByIdSchema = z.strictObject({ id: z.string().min(1) });

export const anchorSchema = z
  .strictObject({
    price: z.number(),
    timeMs: z.number().int().optional().describe('Anchor time, Unix epoch ms.'),
    barsAgo: z
      .number()
      .int()
      .optional()
      .describe('Anchor on the candle this many bars before the latest (0 = latest). Negative values reach into the future.'),
  })
  .refine((a) => (a.timeMs == null) !== (a.barsAgo == null), {
    message: 'Give exactly one of timeMs or barsAgo.',
  })
  .describe('A point in data space: a price plus a time (timeMs or barsAgo).');

const drawingStyle = {
  color: color.optional(),
  width: width.optional(),
  fill: color.optional().describe('Box interior fill. Use an alpha so candles show through, e.g. #332962ff.'),
  label: z.string().max(80).optional().describe('Name for the drawing, e.g. "Resistance". Not drawn on the chart.'),
};

export const addDrawingSchema = z.strictObject({
  type: z
    .enum(['line', 'box', 'path'])
    .describe('line: two-point trendline. box: rectangle between two opposite corners (good for zones). path: polyline ending in an arrow.'),
  points: z.array(anchorSchema).min(2).max(64).describe('line and box take exactly 2 points; path takes 2 to 64.'),
  ...drawingStyle,
});

export const updateDrawingSchema = z.strictObject({
  id: z.string().min(1),
  points: z.array(anchorSchema).min(2).max(64).optional(),
  ...drawingStyle,
});

export const removeDrawingsSchema = z.strictObject({
  ids: z.array(z.string().min(1)).optional(),
  allAgentDrawings: z.boolean().optional().describe('Remove every drawing you created.'),
});

export const addPriceLevelSchema = z.strictObject({
  id: z.string().min(1).max(64).optional().describe('Stable id. Reusing an id moves that level.'),
  price: z.number(),
  text: z.string().max(60).optional().describe('Label drawn on the line, e.g. "Resistance".'),
  color: color.optional(),
  lineStyle: z.enum(['solid', 'dotted', 'dashed']).optional(),
});

export const removePriceLevelsSchema = z.strictObject({
  ids: z.array(z.string().min(1)).optional(),
  all: z.boolean().optional().describe('Remove every level you placed.'),
});

export const setViewportSchema = z
  .strictObject({
    startMs: z.number().int().optional(),
    endMs: z.number().int().optional(),
    lastBars: z.number().int().min(2).max(5000).optional().describe('Frame the most recent N candles.'),
  })
  .refine((v) => (v.lastBars != null) !== (v.startMs != null || v.endMs != null), {
    message: 'Give either lastBars, or both startMs and endMs.',
  });

export const setChartDisplaySchema = z.strictObject({
  chartType: z.enum(['candles', 'line']).optional(),
  priceScaleMode: z.enum(['linear', 'log']).optional(),
});

export const getMarketContextSchema = z.strictObject({
  strength: z
    .number()
    .int()
    .min(1)
    .max(20)
    .optional()
    .describe('Bars on each side a swing must dominate. Higher finds fewer, more major levels. Default 3.'),
  maxLevels: z.number().int().min(1).max(20).optional().describe('Default 6.'),
});

export const emptySchema = z.strictObject({});

// Compile-time drift guard: each config schema must produce a value the chart's
// own type accepts, using only keys that type declares. A renamed or retyped
// field in @vroomchart/types fails here.
type Extends<T extends U & Record<Exclude<keyof T, keyof U>, never>, U> = T;
export type SchemaTypeChecks = [
  Extends<z.infer<typeof rsiSchema>, RSIConfig>,
  Extends<z.infer<typeof macdSchema>, MACDConfig>,
  Extends<z.infer<typeof atrSchema>, ATRConfig>,
  Extends<z.infer<typeof vwapSchema>, VWAPConfig>,
  Extends<z.infer<typeof bollingerSchema>, BollingerBandsConfig>,
  Extends<z.infer<typeof ichimokuSchema>, IchimokuConfig>,
  Extends<z.infer<typeof fairValueGapsSchema>, FairValueGapsConfig>,
  Extends<z.infer<typeof volumeSchema>, VolumeConfig>,
  Extends<z.infer<typeof movingAverageSchema>, MovingAverageOverlay>,
  Extends<Omit<z.infer<typeof addPriceLevelSchema>, 'id'> & { id: string }, PriceLine>,
];
