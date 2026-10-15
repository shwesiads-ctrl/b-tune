import { useEffect, useRef, useState } from 'react';
import Knob from './Knob';
import { analyzeBuffer, renderBuffer } from '../audioAnalysis';
import type { AnalysisResult, KeyResult, VocalNote } from '../../public/pitch-editor.js';
const name=(m:number)=>['C','C♯','D','D♯','E','F','F♯','G','G♯','A','A♯','B'][((Math.round(m)%12)+12)%12]+(Math.floor(Math.round(m)/12)-1);
type Props={active:boolean;buffer:AudioBuffer|null;fileName:string;root:number;scale:number;onLoad:()=>void;onKey:(key:KeyResult)=>void;onPreview:(wav:ArrayBuffer,name:string)=>void;onOriginal:()=>void};
export default function NoteEditor({active,buffer,fileName,root,scale,onLoad,onKey,onPreview,onOriginal}:Props) {
  const [analysis,setAnalysis]=useState<AnalysisResult|null>(null),[notes,setNotes]=useState<VocalNote[]>([]),[selected,setSelected]=useState<number|null>(null);
  const [busy,setBusy]=useState(false),[message,setMessage]=useState('Load a solo vocal file, then analyse its notes.');
  const [past,setPast]=useState<VocalNote[][]>([]),[future,setFuture]=useState<VocalNote[][]>([]),[wav,setWav]=useState<ArrayBuffer|null>(null);
  const job=useRef<AbortController|null>(null),generation=useRef(0),drag=useRef<{y:number;target:number;id:number}|null>(null);
  useEffect(()=>{generation.current++;job.current?.abort();setAnalysis(null);setNotes([]);setSelected(null);setPast([]);setFuture([]);setWav(null);setBusy(false);setMessage(buffer?'Ready to analyse '+fileName+'.':'Load a solo vocal file, then analyse its notes.');},[buffer,fileName]);
  useEffect(()=>()=>job.current?.abort(),[]);
  const attempted=useRef<AudioBuffer|null>(null),panel=useRef<HTMLElement|null>(null);
  useEffect(()=>{if(active){panel.current?.scrollIntoView?.({behavior:'smooth',block:'start'});if(buffer&&attempted.current!==buffer){attempted.current=buffer;void analyse();}}},[active,buffer]);
  async function analyse(){if(!buffer)return;job.current?.abort();const token=++generation.current,controller=new AbortController();job.current=controller;setBusy(true);setMessage('Detecting monophonic notes in a background worker…');
    try{const result=await analyzeBuffer(buffer,controller.signal,p=>{if(token===generation.current)setMessage('Detecting notes… '+Math.round(p*100)+'%');});if(token!==generation.current)return;setAnalysis(result);setNotes(result.notes);setSelected(result.notes[0]?.id??null);setPast([]);setFuture([]);setWav(null);onKey(result.key);setMessage(result.notes.length?result.notes.length+' notes detected. Select or drag a note to change its pitch.':'No stable vocal notes detected. Try a clean solo vocal.');}
    catch(e){if(token===generation.current)setMessage(e instanceof Error?e.message:'Note analysis failed.');}finally{if(token===generation.current)setBusy(false);}
  }
  function update(next:VocalNote[]){setPast(p=>[...p.slice(-29),notes.map(n=>({...n}))]);setFuture([]);setNotes(next);setWav(null);}
  function change(id:number,target:number){update(notes.map(n=>n.id===id?{...n,target:Math.max(Math.round(n.midi)-12,Math.min(Math.round(n.midi)+12,Math.round(target))),edited:true}:n));}
  function undo(){if(!past.length)return;setFuture(f=>[notes,...f]);setNotes(past[past.length-1]);setPast(p=>p.slice(0,-1));setWav(null);}
  function redo(){if(!future.length)return;setPast(p=>[...p,notes]);setNotes(future[0]);setFuture(f=>f.slice(1));setWav(null);}
  function correct(){const masks=[4095,2741,1453,661,1193],mask=masks[scale]??4095;update(notes.map(n=>{let best=Math.round(n.midi),distance=Infinity;for(let m=Math.floor(n.midi)-12;m<=Math.ceil(n.midi)+12;m++)if((mask&(1<<((m-root+120)%12)))&&Math.abs(m-n.midi)<distance){best=m;distance=Math.abs(m-n.midi);}return {...n,target:best,edited:true};}));setMessage('Notes snapped to the selected key / scale. Render to audition.');}
  async function render(){if(!buffer)return;job.current?.abort();const token=++generation.current,controller=new AbortController();job.current=controller;setBusy(true);setMessage('Rendering note edits. The original vocal is retained.');
    try{const result=await renderBuffer(buffer,notes,controller.signal);if(token!==generation.current)return;setWav(result.wav);setMessage('Edited audio rendered. Preview it or export a 16-bit WAV.');return result.wav;}
    catch(e){if(token===generation.current)setMessage(e instanceof Error?e.message:'Rendering failed.');}finally{if(token===generation.current)setBusy(false);}
  }
  async function preview(){const audio=wav??await render();if(audio)onPreview(audio,'Edited vocal preview');}
  function download(){if(!wav)return;const url=URL.createObjectURL(new Blob([wav],{type:'audio/wav'})),a=document.createElement('a');a.href=url;a.download=(fileName.replace(/\.[^.]+$/,'')||'vocal')+'-edited.wav';a.click();setTimeout(()=>URL.revokeObjectURL(url),1000);}
  const tickStep=Math.max(1,Math.ceil((analysis?.duration??buffer?.duration??1)/20));
  const current=notes.find(n=>n.id===selected),duration=analysis?.duration??buffer?.duration??1;
  const pitches=notes.flatMap(n=>[Math.round(n.midi),n.target]);const low=notes.length?Math.min(...pitches)-2:48,high=notes.length?Math.max(...pitches)+2:72,rows=high-low+1,height=Math.max(360,rows*17);
  return <section ref={panel} className="panel note-editor" aria-label="Vocal note editor"><div className="panel-heading"><div><h2>Vocal Note Editor</h2><p>Independent monophonic pitch editing · up to 10 minutes · timing retained</p></div><button onClick={onLoad}>Load vocal file</button></div>
    <p className="editor-file">{buffer?fileName+' · '+buffer.duration.toFixed(2)+' s · '+(buffer.sampleRate/1000)+' kHz':'No vocal file loaded'}</p>
    <p className="editor-status" role="status" aria-live="polite">{message}</p>
    <p>1. Load / analyse → 2. Drag notes or turn the pitch knob → 3. Preview edited (renders automatically) → 4. Export WAV</p>
    {busy&&<progress aria-label="Note editor working"/>}
    <div className="editor-actions"><button disabled={!buffer||busy} onClick={()=>void analyse()}>Analyse notes</button><button disabled={!notes.length||busy} onClick={correct}>Correct to scale</button><button disabled={!past.length||busy} onClick={undo}>Undo</button><button disabled={!future.length||busy} onClick={redo}>Redo</button><button disabled={!notes.length||busy} onClick={()=>update(notes.map(n=>({...n,target:Math.round(n.midi),edited:false})))}>Reset edits</button>
      {busy&&<button onClick={()=>{generation.current++;job.current?.abort();setBusy(false);setMessage('Operation cancelled.');}}>Cancel operation</button>}</div>
    <div className="editor-scroll"><div className="piano-roll" style={{height,minWidth:Math.max(760,Math.min(3000,duration*90))}} aria-label="Detected vocal notes">
      {Array.from({length:rows},(_,i)=>{const m=high-i;return <div className={'piano-row '+([1,3,6,8,10].includes((m%12+12)%12)?'black-note':'')} key={m} style={{top:i/rows*100+'%',height:100/rows+'%'}}><span>{name(m)}</span></div>;})}
      <div className="note-time-axis">{Array.from({length:Math.floor(duration/tickStep)+1},(_,i)=><span key={i} style={{left:i*tickStep/duration*100+'%'}}>{i*tickStep}s</span>)}</div>
      {notes.map(n=><button key={n.id} className={'vocal-note '+(selected===n.id?'selected ':'')+(n.edited?'edited':'')} aria-label={'Note '+(n.id+1)+' '+name(n.target)} aria-pressed={selected===n.id} disabled={busy}
        style={{left:`calc(48px + (100% - 56px) * ${n.start/duration})`,width:`calc((100% - 56px) * ${(n.end-n.start)/duration})`,top:(high-n.target)/rows*100+'%',height:100/rows+'%'}}
        onClick={()=>setSelected(n.id)} onPointerDown={e=>{if(e.button!==0)return;setSelected(n.id);e.currentTarget.setPointerCapture(e.pointerId);drag.current={id:n.id,y:e.clientY,target:n.target};}}
        onPointerMove={e=>{if(drag.current?.id===n.id){const target=drag.current.target+Math.round((drag.current.y-e.clientY)/(height/rows));if(target!==n.target)change(n.id,target);}}}
        onPointerUp={()=>{drag.current=null;}} onPointerCancel={()=>{drag.current=null;}} onLostPointerCapture={()=>{drag.current=null;}}
        onKeyDown={e=>{if(e.key==='ArrowUp'||e.key==='ArrowDown'){e.preventDefault();change(n.id,n.target+(e.key==='ArrowUp'?1:-1));}}} title={`${name(n.midi)} → ${name(n.target)} · ${n.start.toFixed(2)}–${n.end.toFixed(2)} s`}>{name(n.target)}</button>)}
    </div></div>
    {!notes.length&&<p className="editor-empty">Analyse a solo vocal to populate the piano roll. This editor does not separate chords or instruments.</p>}
    <div className="editor-bottom">{current?<><Knob label="Selected note pitch" min={Math.round(current.midi)-12} max={Math.round(current.midi)+12} step={1} value={current.target} initial={Math.round(current.midi)} text={name(current.target)} disabled={busy} onChange={v=>change(current.id,v)} /><div><b>Note {current.id+1} · {name(current.target)}</b><p>Detected {name(current.midi)} {((current.midi-Math.round(current.midi))*100).toFixed(0)} cents · {(current.end-current.start).toFixed(2)} s</p><p>Drag vertically or use ↑ / ↓. Shifts are limited to one octave.</p></div></>:<p>Select a detected note to edit its target pitch.</p>}
      <div className="editor-actions"><button disabled={!notes.length||busy} onClick={()=>void render()}>Render edits</button><button disabled={!notes.length||busy} onClick={()=>void preview()}>Preview edited</button><button disabled={!buffer||busy} onClick={onOriginal}>Preview original</button><button disabled={!wav||busy} onClick={download}>Export WAV</button></div></div>
    <small>Render / export contains note edits only. Playback uses current vocal effects with realtime Auto Pitch bypassed. This is an original note editor, not Celemony Melodyne.</small>
  </section>;
}
