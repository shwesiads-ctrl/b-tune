import definitions from './parameters.json';
export type Extras = { smooth: number; harsh: boolean; warmth: number; presence: number; deess: number; deessHz: number; deessThreshold: number };
export const extraDefaults: Extras = { smooth: 0, harsh: true, warmth: 0, presence: 0, deess: 0, deessHz: 6000, deessThreshold: -28 };
export const stageNames = ['pitch', 'dynamics', 'eq', 'deesser', 'reverb', 'delay'] as const;
export type Stage = typeof stageNames[number];
export type Stages = Record<Stage, boolean>;
export const defaultStages: Stages = { pitch: true, dynamics: true, eq: true, deesser: true, reverb: true, delay: true };
export type Snapshot = { params: number[]; extras: Extras; stages: Stages };
export type Preset = Snapshot & { format: 'matale-tune'; version: 1; name: string };
export const defaults = (): Snapshot => ({ params: definitions.map(p => p.initial), extras: { ...extraDefaults }, stages: { ...defaultStages } });
export const clone = (s: Snapshot): Snapshot => ({ params: [...s.params], extras: { ...s.extras }, stages: { ...s.stages } });
export function effectiveParams(s: Snapshot) {
  const p = [...s.params];
  if (!s.stages.pitch) p[7] = 0;
  if (!s.stages.dynamics) { p[9] = 1; p[12] = 0; }
  if (!s.stages.eq) for (const i of [14, 17, 20, 23, 26]) p[i] = 0;
  if (!s.stages.reverb) p[31] = 0;
  if (!s.stages.delay) p[28] = 0;
  return p;
}
export function parsePreset(raw: unknown): Preset {
  if (!raw || typeof raw !== 'object') throw new Error('Invalid preset file.');
  const p = raw as Partial<Preset>;
  if (p.format !== 'matale-tune' || p.version !== 1 || typeof p.name !== 'string' || !p.name.trim() || p.name.length > 80 || !Array.isArray(p.params) || p.params.length !== definitions.length) throw new Error('Unsupported Matale Tune preset.');
  const params = p.params.map((v, i) => {
    const d = definitions[i];
    if (typeof v !== 'number' || !Number.isFinite(v) || v < d.lo || v > d.hi) throw new Error('Preset parameter out of range: ' + d.name);
    return Number((d.lo + Math.round((v - d.lo) / d.step) * d.step).toFixed(6));
  });
  const extras = { ...extraDefaults };
  const ranges = { smooth: [0, 100], warmth: [-100, 100], presence: [-100, 100], deess: [0, 100], deessHz: [3000, 10000], deessThreshold: [-48, -6] };
  if (!p.extras || typeof p.extras.harsh !== 'boolean') throw new Error('Missing vocal settings.');
  extras.harsh = p.extras.harsh;
  for (const k of Object.keys(ranges) as (keyof typeof ranges)[]) {
    const v = p.extras[k], [lo, hi] = ranges[k];
    if (typeof v !== 'number' || !Number.isFinite(v) || v < lo || v > hi) throw new Error('Invalid vocal setting: ' + k);
    extras[k] = v;
  }
  const stages = { ...defaultStages };
  for (const k of stageNames) {
    if (!p.stages || typeof p.stages[k] !== 'boolean') throw new Error('Missing processing stage: ' + k);
    stages[k] = p.stages[k];
  }
  return { format: 'matale-tune', version: 1, name: p.name.trim(), params, extras, stages };
}
function builtIn(name: string, changes: Record<number, number>, extra: Partial<Extras>): Preset {
  const s = defaults();for (const [i,v] of Object.entries(changes)) s.params[Number(i)] = v;
  return { ...s, extras: { ...s.extras, ...extra }, format: 'matale-tune', version: 1, name };
}
export const builtIns: Preset[] = [
  builtIn('Natural Vocal', {}, { smooth: 20, deess: 25 }),
  builtIn('Warm Vocal', { 4: 55, 5: 45, 31: .08 }, { smooth: 35, warmth: 25, deess: 35 }),
  builtIn('Bright Vocal', { 31: .1, 28: .07 }, { smooth: 15, presence: 20, deess: 45 }),
  builtIn('Hard Tune', { 2: 1, 4: 1, 5: 0, 7: 100, 9: 4 }, { smooth: 10, deess: 30 })
];
