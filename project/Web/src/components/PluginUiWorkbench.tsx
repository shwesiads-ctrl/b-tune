import { useEffect, useRef, useState } from 'react';
import definitions from '../parameters.json';
import Knob from './Knob';
import { builtIns, parsePreset, clone, effectiveParams, stageNames } from '../state';
import type { Extras, Snapshot, Preset, Stage } from '../state';

type Meter = { pitch: number; input: number; output: number; progress: number; confidence?: number; rms?: number; left?: number; right?: number; peak?: number; lufs?: number | null; range?: number[] | null; register?: string; deessReduction?: number; smoothReduction?: number };
type AutoScope = Stage | 'all';
const noteName = (midi: number) => ['C','C♯','D','D♯','E','F','F♯','G','G♯','A','A♯','B'][((midi % 12) + 12) % 12] + (Math.floor(midi / 12) - 1);
type Session = { context: AudioContext; node: AudioWorkletNode; analyser: AnalyserNode; gain: GainNode; source?: AudioNode; stream?: MediaStream };
const tabs = ['Pitch Auto-Tune', 'Compressor', '5-Band EQ', 'Delay / Reverb'];
const choices = [
  ['C', 'C♯', 'D', 'D♯', 'E', 'F', 'F♯', 'G', 'G♯', 'A', 'A♯', 'B'],
  ['Chromatic', 'Major', 'Minor', 'Major Pentatonic', 'Minor Pentatonic'],
  ['Natural', 'Hard Tune']
];
const level = (v: number) => Math.max(-90, 20 * Math.log10(Math.max(1e-9, v))).toFixed(1);

export default function PluginUiWorkbench() {
  const [params, setParams] = useState([...builtIns[0].params]);
  const [extras, setExtras] = useState<Extras>({ ...builtIns[0].extras });
  const [stages, setStages] = useState({ ...builtIns[0].stages });
  const [savedPresets, setSavedPresets] = useState<Preset[]>([]);
  const [presetId, setPresetId] = useState('builtin:0');
  const [showSave, setShowSave] = useState(false);
  const [saveName, setSaveName] = useState('My Vocal');
  const [slot, setSlot] = useState<'A' | 'B'>('A');
  const slots = useRef({ A: clone(builtIns[0]), B: clone(builtIns[0]) });
  const [autoIncludes, setAutoIncludes] = useState({ pitch: true, dynamics: true, eq: true, smooth: true, deesser: true, reverb: true, delay: true });
  const autoScope = useRef<AutoScope>('all');
  const autoIncludesRef = useRef(autoIncludes);
  useEffect(()=>{autoIncludesRef.current=autoIncludes;},[autoIncludes]);
  const [sampleRateChoice, setSampleRateChoice] = useState('auto');
  const [latencyHint, setLatencyHint] = useState<AudioContextLatencyCategory>('interactive');
  const [monitorVolume, setMonitorVolume] = useState(70);
  const monitorValue = useRef(70);
  const [displayMode, setDisplayMode] = useState<'waveform' | 'spectrum'>('spectrum');
  const displayModeRef = useRef(displayMode);
  const presetFile = useRef<HTMLInputElement>(null);
  const saveDialog = useRef<HTMLDialogElement>(null);
  useEffect(()=>{if(showSave)saveDialog.current?.showModal();else saveDialog.current?.close();},[showSave]);
  const [tab, setTab] = useState(-1);
  const [sourceCode, setSourceCode] = useState('');
  const [meter, setMeter] = useState<Meter>({ pitch: 0, input: 0, output: 0, progress: 0 });
  const [status, setStatus] = useState('Choose a vocal file or microphone to begin.');
  const [active, setActive] = useState(false);
  const [busy, setBusy] = useState(false);
  const [analysing, setAnalysing] = useState(false);
  const [monitor, setMonitor] = useState(false);
  const [pitchHistory, setPitchHistory] = useState<number[]>([]);
  const session = useRef<Session | null>(null);
  const values = useRef(params);
  const snapshotRef = useRef<Snapshot>({params,extras,stages});
  const scope = useRef<HTMLCanvasElement>(null);
  const eq = useRef<HTMLCanvasElement>(null);
  const file = useRef<HTMLInputElement>(null);
  const generation = useRef(0);
  const creating = useRef<AudioContext | null>(null);
  useEffect(() => {
    values.current = params; snapshotRef.current={params,extras,stages};
    session.current?.node.port.postMessage({ params: effectiveParams({params,extras,stages}), extras: {...extras, deess: stages.deesser ? extras.deess : 0} });
  }, [params, extras, stages]);
  useEffect(() => { monitorValue.current = monitorVolume; const s=session.current; if(s) s.gain.gain.setTargetAtTime(monitor ? monitorVolume/100 : 0,s.context.currentTime,.02); }, [monitorVolume, monitor]);
  useEffect(() => { displayModeRef.current = displayMode; }, [displayMode]);
  useEffect(() => {
    try { if (typeof localStorage !== 'undefined') { const raw=JSON.parse(localStorage.getItem('matale-presets-v1') || '[]'); if(Array.isArray(raw)) setSavedPresets(raw.slice(0,64).map(parsePreset)); } }
    catch { setStatus('Saved presets could not be read. You can still import a preset file.'); }
  }, []);
  function applySnapshot(next: Snapshot) { setParams([...next.params]);setExtras({...next.extras});setStages({...next.stages}); }
  function switchSlot(next: 'A' | 'B') { if(next===slot)return; slots.current[slot]=clone({params,extras,stages});applySnapshot(slots.current[next]);setSlot(next);setPresetId('current');setStatus('Comparing snapshot '+next+'.'); }
  function changeExtra(key: keyof Extras, value: number | boolean) { setExtras(old=>({...old,[key]:value}));setPresetId('current'); }
  function toggleStage(stage: Stage) { setStages(old=>({...old,[stage]:!old[stage]}));setPresetId('current'); }
  function runAuto(scope: AutoScope = 'all') { autoScope.current=scope;setAnalysing(true);setStatus('Listening for three seconds…');session.current?.node.port.postMessage({auto:true}); }
  function savePreset() {
    const name=saveName.trim();if(!name || name.length>80){setStatus('Use a preset name between 1 and 80 characters.');return;}
    const preset: Preset={format:'matale-tune',version:1,name,...clone({params,extras,stages})};
    const next=[...savedPresets.filter(p=>p.name!==name),preset].slice(-64);
    let persisted=true;try{localStorage.setItem('matale-presets-v1',JSON.stringify(next));}catch{persisted=false;}
    setSavedPresets(next);setPresetId('saved:'+name);setShowSave(false);
    const url=URL.createObjectURL(new Blob([JSON.stringify(preset,null,2)],{type:'application/json'}));
    const a=document.createElement('a');a.href=url;a.download=name.replace(/[^a-zA-Z0-9_-]/g,'_')+'.matale.json';a.click();setTimeout(()=>URL.revokeObjectURL(url),1000);
    setStatus(persisted?'Preset saved in this browser and exported as JSON.':'Preset exported. Browser storage was unavailable.');
  }
  async function importPreset(file: File) {
    try {
      if(file.size>1000000)throw new Error('Preset file is too large.');
      const preset=parsePreset(JSON.parse(await file.text()));applySnapshot(preset);
      const next=[...savedPresets.filter(p=>p.name!==preset.name),preset].slice(-64);setSavedPresets(next);setPresetId('saved:'+preset.name);
      try{localStorage.setItem('matale-presets-v1',JSON.stringify(next));}catch{}
      setStatus('Imported preset: '+preset.name);
    } catch(error) { setStatus(error instanceof Error?error.message:'Invalid preset file.'); }
  }
  function change(i: number, value: number) {
    setParams(p => p.map((old, n) => n === i ? value : old)); setPresetId('current');
  }
  async function stop() {
    generation.current++;
    const s = session.current; session.current = null;
    const pending = creating.current; creating.current = null;
    s?.stream?.getTracks().forEach(t => t.stop());
    s?.source?.disconnect(); s?.node.disconnect();
    s?.analyser.disconnect(); s?.gain.disconnect();
    if (s) s.node.port.onmessage = null;
    setActive(false); setBusy(false); setAnalysing(false); setMonitor(false);
    setMeter({ pitch: 0, input: 0, output: 0, progress: 0 });
    setPitchHistory([]); setStatus('Stopped.');
    // Capture and detach owned contexts before awaiting: a newer start may
    // create another context while these close operations are pending.
    await Promise.all([...new Set([s?.context, pending])].map(ctx =>
      ctx && ctx.state !== 'closed' ? ctx.close() : Promise.resolve()));
  }
  useEffect(() => () => {
    generation.current++;
    session.current?.stream?.getTracks().forEach(t => t.stop());
    void session.current?.context.close();
    void creating.current?.close();
  }, []);
  async function start(mode: 'mic' | 'file', audioFile?: File) {
    const stopped = stop(); const token = generation.current;
    setBusy(true); setStatus('Starting audio engine…');
    await stopped;
    if (token !== generation.current) return;
    let ctx: AudioContext | undefined; let stream: MediaStream | undefined;
    try {
      ctx = new AudioContext({latencyHint,...(sampleRateChoice==='auto'?{}:{sampleRate:Number(sampleRateChoice)})}); creating.current = ctx;
      await ctx.resume();
      if (token !== generation.current) return;
      const response = await fetch('/matale-dsp.wasm');
      if (!response.ok) throw new Error('DSP missing. Run scripts/build-web-dsp.sh first.');
      const module = await WebAssembly.compile(await response.arrayBuffer());
      if (token !== generation.current) return;
      await ctx.audioWorklet.addModule('/matale-worklet.js');
      if (token !== generation.current) return;
      const node = new AudioWorkletNode(ctx, 'matale', { numberOfInputs: 1, numberOfOutputs: 1, outputChannelCount: [2], processorOptions: { module } });
      const analyser = ctx.createAnalyser(); analyser.fftSize = 2048;
      const gain = ctx.createGain(); gain.gain.value = mode === 'file' ? monitorValue.current/100 : 0;
      node.connect(analyser); analyser.connect(gain); gain.connect(ctx.destination);
      let source: AudioNode;
      if (mode === 'mic') {
        stream = await navigator.mediaDevices.getUserMedia({ audio: { echoCancellation: false, noiseSuppression: false, autoGainControl: false } });
        if (token !== generation.current) return;
        source = ctx.createMediaStreamSource(stream);
      } else {
        if (!audioFile) throw new Error('Choose a vocal audio file.');
        const buffer = await ctx.decodeAudioData(await audioFile.arrayBuffer());
        if (token !== generation.current) return;
        const player = ctx.createBufferSource(); player.buffer = buffer; player.loop = true;
        source = player; player.start();
      }
      if (token !== generation.current) return;
      source.connect(node);
      session.current = { context: ctx, node, analyser, gain, source, stream }; creating.current = null;
      node.onprocessorerror = () => {
        if (session.current?.node !== node) return;
        void stop();
        setStatus('Audio processing failed. Start the input again to retry.');
      };
      node.port.onmessage = ({ data }) => {
        if (session.current?.node !== node) return;
        if ('autoResult' in data) {
          setAnalysing(false);
          if (data.autoResult) {
            const scope=autoScope.current, autoIncludes=autoIncludesRef.current;
            const group=(i:number): Stage => [4,5].includes(i)?'pitch':[8,12].includes(i)?'dynamics':[17,23].includes(i)?'eq':i===28?'delay':'reverb';
            setParams(old => { const next = [...old]; for (const [i,value] of data.autoResult) if((scope==='all'&&autoIncludes[group(i)])||scope===group(i)) {const d=definitions[i];next[i]=Number((d.lo+Math.round((Math.max(d.lo,Math.min(d.hi,value))-d.lo)/d.step)*d.step).toFixed(6));} return next; });
            if(data.extras) setExtras(old=>({...old,
              ...((scope==='all'&&autoIncludes.smooth)?{smooth:data.extras.smooth,harsh:data.extras.harsh,warmth:data.extras.warmth,presence:data.extras.presence}:{}),
              ...(((scope==='all'&&autoIncludes.deesser)||scope==='deesser')?{deess:data.extras.deess,deessHz:data.extras.deessHz,deessThreshold:data.extras.deessThreshold}:{})}));
            setPresetId('current');
            setStatus('Auto applied. Key / scale preserved. Adjust to taste.');
          } else setStatus('Not enough voiced audio. Sing for three seconds and retry.');
        } else {
          setMeter(data);
          setPitchHistory(p => [...p.slice(-119), data.pitch]);
        }
      };
      const latest=snapshotRef.current;
      node.port.postMessage({ params: effectiveParams(latest), extras: {...latest.extras,deess:latest.stages.deesser?latest.extras.deess:0} });
      setActive(true); setMonitor(mode === 'file');
      setStatus(mode === 'mic' ? 'Microphone live. Monitoring is muted; wear headphones before enabling.' : 'Vocal file looping. Audio stays in this browser.');
    } catch (err) {
      if (token === generation.current) setStatus(err instanceof Error ? err.message : 'Audio startup failed.');
    } finally {
      if (creating.current === ctx) creating.current = null;
      if (session.current?.context !== ctx) {
        stream?.getTracks().forEach(t => t.stop());
        if (ctx && ctx.state !== 'closed') await ctx.close();
      }
      if (token === generation.current) setBusy(false);
    }
  }

  useEffect(() => {
    let frame = 0; const data = new Float32Array(2048), spectrum = new Float32Array(1024);
    const draw = () => {
      const canvas = scope.current, c = canvas?.getContext('2d');
      if (canvas && c) {
        c.clearRect(0, 0, canvas.width, canvas.height);
        c.strokeStyle = '#e3edf6'; c.beginPath(); c.moveTo(0, 80); c.lineTo(1000, 80); c.stroke();
        session.current?.analyser.getFloatTimeDomainData(data);
        if (!session.current) data.fill(0);
        c.strokeStyle = '#1685ff'; c.lineWidth = 1.5; c.beginPath();
        if(displayModeRef.current==='spectrum' && session.current) {
          session.current.analyser.getFloatFrequencyData(spectrum);
          const sr=session.current.context.sampleRate;
          for(let x=0;x<1000;x++){const hz=20*Math.pow(Math.min(20000,sr/2)/20,x/999);const bin=Math.min(1023,Math.round(hz/sr*2048));const y=155-Math.max(0,Math.min(1,(spectrum[bin]+100)/100))*150;x?c.lineTo(x,y):c.moveTo(x,y);}
        } else for (let i = 0; i < 1000; i++) { const y = 80 - data[i * 2] * 65; i ? c.lineTo(i, y) : c.moveTo(i, y); }
        c.stroke();
      }
      frame = requestAnimationFrame(draw);
    };
    frame = requestAnimationFrame(draw); return () => cancelAnimationFrame(frame);
  }, []);
  useEffect(() => {
    const canvas = eq.current, c = canvas?.getContext('2d'); if (!canvas || !c) return;
    c.clearRect(0, 0, 1000, 150); c.strokeStyle = '#e3edf6'; c.beginPath(); c.moveTo(0, 75); c.lineTo(1000, 75); c.stroke();
    c.strokeStyle = '#1685ff'; c.lineWidth = 2; c.beginPath();
    const sr = session.current?.context.sampleRate ?? 48000;
    for (let x = 0; x < 1000; x++) {
      const f = 20 * Math.pow(1000, x / 999), w = 2 * Math.PI * f / sr;
      let dB = 0;
      for (let band = 0; band < 5; band++) {
        const base = 13 + band * 3, A = Math.pow(10, params[base + 1] / 40), w0 = 2 * Math.PI * Math.min(sr * .45, params[base]) / sr;
        const alpha = Math.sin(w0) / (2 * params[base + 2]), cos = Math.cos(w0);
        const mag = (a: number, b: number, d: number) => Math.hypot(a + b * Math.cos(w) + d * Math.cos(2 * w), -b * Math.sin(w) - d * Math.sin(2 * w));
        dB += 20 * Math.log10(mag(1 + alpha * A, -2 * cos, 1 - alpha * A) / mag(1 + alpha / A, -2 * cos, 1 - alpha / A));
      }
      const y = 75 - dB * 2; x ? c.lineTo(x, y) : c.moveTo(x, y);
    }
    c.stroke();
  }, [params, tab, active]);
  function valueText(i: number) {
    if (i === 28 || i === 31) return `${Math.round(params[i] * 100)}%`;
    const suffix = i === 7 || i === 5 ? '%' : i === 4 || i === 10 || i === 11 ? ' ms' : [8, 12, 14, 17, 20, 23, 26, 33].includes(i) ? ' dB' : '';
    return params[i].toFixed(definitions[i].step < 1 ? 1 : 0) + suffix;
  }
  function control(i: number, _knob = true, title?: string) {
    const p=definitions[i];
    return <Knob key={p.id} label={title ?? p.name} min={p.lo} max={p.hi} step={p.step} initial={p.initial} value={params[i]} text={valueText(i)} onChange={v=>change(i,v)} />;
  }
  function extraControl(key: Exclude<keyof Extras,'harsh'>, label: string, min=0,max=100,step=1,suffix='%') {
    return <Knob label={label} min={min} max={max} step={step} value={extras[key]} initial={builtIns[0].extras[key]} text={extras[key]+suffix} onChange={v=>changeExtra(key,v)} />;
  }
  function selectors() {
    return <div className="selectors">{[0, 1, 2].map(i => <label key={i}>{definitions[i].name}
      <select value={params[i]} onChange={e => change(i, +e.target.value)}>
        {choices[i].map((s, n) => <option key={s} value={n}>{s}</option>)}
      </select></label>)}</div>;
  }
  const midi = meter.pitch > 0 ? Math.round(69 + 12 * Math.log2(meter.pitch / 440)) : null;
  const note = midi === null ? '—' : noteName(midi);
  const points = pitchHistory.map((hz, i) => `${i * 8.3},${hz > 0 ? 100 - (69 + 12 * Math.log2(hz / 440) - 36) * 2 : 110}`).join(' ');
  const meterPercent = (v: number) => Math.max(0, Math.min(100, (Number(level(v)) + 60) / 60 * 100));
  return <main className="app-shell">
    <header className="app-header">
      <div className="brand"><b>M</b><div><h1>Matale Tune</h1><small>Pro Vocal Processor</small></div></div>
      <div className="header-right">
        <label className="preset-select">Preset<select aria-label="Preset" value={presetId} onChange={e=>{
          const id=e.target.value;const preset=id.startsWith('builtin:')?builtIns[Number(id.slice(8))]:savedPresets.find(p=>p.name===id.slice(6));
          if(preset){applySnapshot(preset);setPresetId(id);setStatus('Loaded preset: '+preset.name);}
        }}><option value="current" disabled>Current settings</option>{builtIns.map((p,i)=><option key={p.name} value={'builtin:'+i}>{p.name}</option>)}{savedPresets.map(p=><option key={p.name} value={'saved:'+p.name}>{p.name} · saved</option>)}</select></label>
        <button onClick={()=>setShowSave(true)}>Save</button><button onClick={()=>presetFile.current?.click()}>Import</button>
        <input ref={presetFile} type="file" accept="application/json,.json" hidden onChange={e=>{const f=e.target.files?.[0];if(f)void importPreset(f);e.target.value='';}} />
        <div className="ab-switch" aria-label="A/B comparison">{(['A','B'] as const).map(s=><button key={s} aria-pressed={slot===s} onClick={()=>switchSlot(s)}>{s}</button>)}</div>
        <button aria-label="Audio settings" onClick={()=>setTab(4)}>⚙</button>
        <button className={'power ' + (params[3] ? 'on' : '')} aria-label="Toggle vocal processing" aria-pressed={!!params[3]} onClick={() => change(3, params[3] ? 0 : 1)}>
          <span className="dot" />{params[3] ? 'Processing on' : 'Bypassed'}</button></div>
    </header>
    <aside className="sidebar"><small>VOCAL TOOLS</small><nav aria-label="Vocal processors">
      {['Home', 'Pitch', 'Dynamics', 'EQ', 'Effects', 'Settings'].map((name, i) => <button key={name} aria-pressed={tab === i - 1} onClick={() => setTab(i - 1)}>
        <span aria-hidden="true">{['⌂', '〰', '≋', '☷', '✧', '⚙'][i]}</span>{name}</button>)}
      </nav><div className="sidebar-foot"><span className={'dot ' + (active ? 'live' : '')} />{active ? 'Engine running' : 'Engine stopped'}<small>2048-frame DSP latency</small></div>
    </aside>
    <div className="workspace">
      <section className="toolbar" aria-label="Audio input">
        <div><b>Your voice. Your sound.</b><small>Choose a vocal file or microphone to begin.</small></div>
        <div className="input-actions"><button disabled={busy} onClick={() => file.current?.click()}>Vocal file</button>
          <input ref={file} type="file" accept="audio/*" hidden onChange={e => { const f = e.target.files?.[0]; if (f) void start('file', f); e.target.value = ''; }} />
          <button disabled={busy} onClick={() => void start('mic')}>Mic input</button>
          <button className="stop" disabled={!active && !busy} onClick={() => void stop()}>Stop</button>
        </div>
        <label className="monitor"><input type="checkbox" checked={monitor} disabled={!active} onChange={e => {
          setMonitor(e.target.checked); const s = session.current; if (s) s.gain.gain.setTargetAtTime(e.target.checked ? monitorValue.current/100 : 0, s.context.currentTime, .02);
        }} /> Monitor · headphones</label>
      </section>
      <section className="musical-target" aria-label="Musical target">{selectors()}</section>
      <div className="analysis-grid">
        <section className="panel analysis"><div className="panel-heading"><h2><span>≋</span> Vocal Analysis</h2><span className={'live-pill ' + (active ? 'live' : '')}><i />{active ? 'Live' : 'Idle'}</span></div>
          <div className="analysis-body"><div className={'note-ring ' + (meter.pitch ? 'voiced' : '')}><strong>{note}</strong><small>{meter.pitch ? meter.pitch.toFixed(1) + ' Hz' : 'No voiced pitch'}</small></div>
            <dl className="analysis-stats"><div><dt>Detected note</dt><dd>{note}</dd></div>
              <div title="Normalized periodic correlation at the detected pitch; not AI certainty."><dt>Pitch confidence</dt><dd>{meter.pitch?Math.round((meter.confidence??0)*100)+'%':'—'}</dd></div>
              <div title="Voiced notes observed during this input session."><dt>Vocal range</dt><dd>{meter.range?noteName(meter.range[0])+' – '+noteName(meter.range[1]):'—'}</dd></div>
              <div title="Heuristic register estimate after enough different voiced notes; not a voice classification."><dt>Register (est.)</dt><dd>{meter.register??'More notes needed'}</dd></div>
              <div title="Input RMS amplitude relative to digital full scale."><dt>Energy · RMS</dt><dd>{((meter.rms??0)*100).toFixed(1)}% FS</dd></div>
            </dl><div className="waveform"><div className="wave-label"><small>LIVE OUTPUT {displayMode.toUpperCase()}</small><small>{session.current ? `${session.current.context.sampleRate / 1000} kHz` : 'Awaiting input'}</small></div>
              <canvas ref={scope} width="1000" height="160" aria-label={displayMode==='spectrum'?'Live output spectrum':'Live output waveform'} />
              <div className="input-meter"><span>Input level</span><div className="meter-track"><i style={{ width: `${meterPercent(meter.input)}%` }} /></div><strong>{level(meter.input)} dBFS</strong></div>
            </div></div>
        </section>
        <section className="panel output"><h2><span>◖</span> Output</h2><div className="output-value">{level(meter.output)} <small>dBFS</small></div>
          <div className="meter-track"><i style={{ width: `${meterPercent(meter.output)}%` }} /></div>
          <div className="output-info"><span title="400ms BS.1770 K-weighted momentary loudness; not integrated LUFS.">LUFS · M<b>{meter.lufs==null?'—':meter.lufs.toFixed(1)}</b></span>
            <button className="peak-reset" title="Reset held sample peak" onClick={()=>session.current?.node.port.postMessage({resetPeak:true})}>Peak<b>{level(meter.peak??0)} dBFS</b></button>
            <div className="stereo-meter">{(['left','right'] as const).map((ch,i)=><div key={ch}><small>{i?'R':'L'}</small><div><i style={{height:meterPercent(meter[ch]??0)+'%'}} /></div></div>)}</div></div>
        </section>
      </div>
      {tab === -1 ? <>
        <div className="home-grid"><section className="panel auto-panel"><div className="auto-description"><h2><span>✦</span> Auto Mode</h2><p>A starting point for your vocal. Fine-tune by ear.</p>
          <ul className="auto-options" aria-label="Include in Auto">{([['smooth','Vocal Smooth'],['dynamics','Dynamic range control'],['deesser','Harsh voice / De-Esser'],['eq','Smart EQ'],['pitch','Pitch retune'],['reverb','Reverb'],['delay','Delay']] as const).map(([key,label])=><li key={key}><label><input type="checkbox" checked={autoIncludes[key]} onChange={e=>setAutoIncludes(old=>({...old,[key]:e.target.checked}))} />{label}</label></li>)}</ul>
          <div className="auto-tip"><span>✧</span><div><b>Pitch confidence</b><small>{meter.pitch?Math.round((meter.confidence??0)*100)+'% periodicity':'Waiting for voiced input'}</small></div></div></div>
          <div className="auto-action"><button className="auto-orb" aria-label="Set Audio On Auto" disabled={!active || analysing} onClick={() => runAuto()}>
            <span>✦</span><b>{analysing ? 'LISTENING' : 'AUTO'}</b><small>{analysing ? `${Math.round(meter.progress * 100)}%` : '3 sec vocal analysis'}</small></button>
            <span className="auto-caption"><i className={'dot ' + (active ? 'live' : '')} />{analysing ? 'Sing for three seconds' : active ? 'Ready to analyse' : 'Start an input to enable Auto'}</span></div>
        </section>
        <section className="panel vocal-smooth"><h2><span>✧</span> Vocal Smooth</h2><p>Dynamic harshness reduction and tonal shaping.</p>
          <div className="smooth-controls">{extraControl('smooth','Smooth amount')}
            <label className="switch-control">Harsh control<input role="switch" aria-label="Harsh control" type="checkbox" checked={extras.harsh} onChange={e=>changeExtra('harsh',e.target.checked)} /><span>{extras.harsh?'Focused':'Gentle'}</span></label>
            {extraControl('warmth','Vocal warmth',-100,100)}{extraControl('presence','Presence',-100,100)}</div>
          <div className="result-card"><b>Measured processing</b><small>Smooth reduction {(meter.smoothReduction??0).toFixed(1)} dB · De-Esser {(meter.deessReduction??0).toFixed(1)} dB</small></div>
          <button className="manual-button" onClick={()=>setTab(0)}>Manual controls →</button>
        </section></div>
        <section className="quick-rack" aria-label="Vocal processing chain">{stageNames.map((stage,i)=><div className="stage-card" key={stage}>
          <button className="stage-toggle" role="switch" aria-label={['Pitch stage','Dynamics stage','EQ stage','De-Esser stage','Reverb stage','Delay stage'][i]} aria-checked={stages[stage]} onClick={()=>toggleStage(stage)}>{stages[stage]?'● On':'○ Off'}</button>
          {i===3?extraControl('deess','De-Esser'):control([7,8,23,0,31,28][i],true,['Pitch','Dynamics','EQ','De-Esser','Reverb','Delay'][i])}
          <button className="stage-auto" disabled={!active||analysing} aria-label={'Auto '+stage} onClick={()=>runAuto(stage)}>Auto</button>
        </div>)}</section>
      </> : <section className="panel rack"><div className="panel-heading"><h2>{tab===4?'Audio settings':tabs[tab]}</h2><span className="tag">MANUAL CONTROLS</span></div>
        {tab === 0 && <><svg viewBox="0 0 1000 120" role="img" aria-label="Detected pitch history"><polyline points={points} fill="none" stroke="#1685ff" strokeWidth="2" /></svg><small>Live pitch history · Formant is a dual-band colour filter.</small></>}
        {tab === 2 && <><canvas ref={eq} width="1000" height="150" aria-label="EQ response curve" /><small>20 Hz → 20 kHz · centre line 0 dB</small></>}
        {tab===4?<div className="settings-grid">
          <label>Sample rate<select aria-label="Sample rate" value={sampleRateChoice} onChange={e=>setSampleRateChoice(e.target.value)}><option value="auto">Device default</option>{['44100','48000','96000'].map(v=><option key={v} value={v}>{Number(v)/1000} kHz</option>)}</select></label>
          <label>Latency preference<select aria-label="Latency preference" value={latencyHint} onChange={e=>setLatencyHint(e.target.value as AudioContextLatencyCategory)}><option value="interactive">Interactive</option><option value="balanced">Balanced</option><option value="playback">Playback</option></select></label>
          <label>Analysis display<select aria-label="Analysis display" value={displayMode} onChange={e=>setDisplayMode(e.target.value as 'waveform'|'spectrum')}><option value="spectrum">Spectrum</option><option value="waveform">Waveform</option></select></label>
          <Knob label="Monitor volume" min={0} max={100} step={1} value={monitorVolume} initial={70} text={monitorVolume+'%'} onChange={setMonitorVolume} />
          <div className="settings-note"><b>Current audio session</b><p>{session.current?session.current.context.sampleRate+' Hz · '+(2048000/session.current.context.sampleRate).toFixed(1)+' ms DSP latency':'No input running.'}</p>
          <p>Sample rate / latency preferences apply when you start the next input. Device and browser latency are additional.</p><p>Knobs: drag up/down, use arrow keys, or double-click to reset. Shift-drag gives finer control.</p></div>
        </div>:<div className="rack-controls">{definitions.map((p, i) => p.page === tab && i > 3 ? control(i) : null)}{tab===1&&<>{extraControl('deess','De-Esser amount')}{extraControl('deessHz','Sibilance frequency',3000,10000,100,' Hz')}{extraControl('deessThreshold','De-Esser threshold',-48,-6,1,' dB')}</>}</div>}
      </section>}
      <dialog ref={saveDialog} onCancel={()=>setShowSave(false)} onClose={()=>setShowSave(false)} className="save-dialog"><form onSubmit={e=>{e.preventDefault();savePreset();}}><h2>Save vocal preset</h2><p>Save in this browser and download a portable JSON file.</p><label>Preset name<input autoFocus aria-label="Preset name" value={saveName} maxLength={80} onChange={e=>setSaveName(e.target.value)} /></label><div><button type="button" onClick={()=>setShowSave(false)}>Cancel</button><button type="submit">Save preset</button></div></form></dialog>
      <footer role="status"><span className={active ? 'dot live' : 'dot'} />{status}{analysing && <progress value={meter.progress} max="1" />}</footer>
      <details><summary>Source, builds and tests</summary><p>Audio stays in your browser. Shared C++ DSP runs through WebAssembly, followed by the browser vocal processing stage.</p>
        <p><a href="/MataleTune_Source.zip" download>Download source/build scripts</a> · <button onClick={async () => {
          try { const r = await fetch('/VocalDSP.h'); if (!r.ok) throw new Error('Source unavailable'); setSourceCode(await r.text()); }
          catch { setSourceCode('Source is in Source/VocalDSP.h in the package.'); }
        }}>Inspect C++ DSP</button></p>{sourceCode && <pre>{sourceCode}</pre>}
        <p>Browser validation comes first. Platform plugin builds require their target operating systems.</p></details>
    </div>
  </main>;
}
