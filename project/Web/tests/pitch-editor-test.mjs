import assert from 'node:assert/strict';
import { analyzeVocal,detectKey,renderNotes,encodeWav,midiToHz } from '../public/pitch-editor.js';
function sine(hz,sr,seconds=1,amp=.15){return Float32Array.from({length:Math.round(sr*seconds)},(_,i)=>amp*Math.sin(2*Math.PI*hz*i/sr));}
for(const sr of [44100,48000,96000]){
  const analysis=analyzeVocal(sine(220,sr),sr);assert(analysis.notes.length>0);
  const midi=analysis.notes[0].midi;assert(Math.abs(midi-57)<.08,'Offline note detection outside 8 cents');
  assert.equal(analysis.key.candidates.length,0,'One note cannot establish a key');
  const clean=sine(220,sr,2),source=new Float32Array(clean);
  const edit={id:0,start:0,end:2,midi:57,target:59,edited:true,confidence:1};
  const output=renderNotes([clean],sr,[edit])[0];assert.equal(output.length,clean.length);assert.deepEqual(clean,source,'Original audio was mutated');
  const detected=analyzeVocal(output.slice(Math.round(sr*.6),Math.round(sr*1.7)),sr);
  assert(detected.notes.some(n=>Math.abs(n.midi-59)<.3),'Rendered upward note edit is not within 30 cents');
  const down=renderNotes([clean],sr,[{...edit,target:55}])[0],downAnalysis=analyzeVocal(down.slice(Math.round(sr*.6),Math.round(sr*1.7)),sr);
  assert(downAnalysis.notes.some(n=>Math.abs(n.midi-55)<.3),'Rendered downward note edit is not within 30 cents');
  assert.deepEqual(renderNotes([source],sr,[{...edit,edited:false}])[0],source,'Unedited render is not identical');
  const wav=encodeWav([output],sr),view=new DataView(wav);assert.equal(wav.byteLength,44+output.length*2);assert.equal(view.getUint32(24,true),sr);assert.equal(view.getUint16(22,true),1);
  console.log('PASS: note analysis, actual up/down pitch rendering, preserved timing/original and PCM WAV at',sr,'Hz');
}
const majorFrames=[{midi:60,duration:4,confidence:1},{midi:64,duration:2.5,confidence:1},{midi:67,duration:2.8,confidence:1},{midi:62,duration:1,confidence:1},{midi:65,duration:1,confidence:1},{midi:69,duration:.6,confidence:1},{midi:71,duration:.7,confidence:1}];
const key=detectKey(majorFrames);assert.equal(key.candidates[0].root,0);assert.equal(key.candidates[0].mode,'Major');
assert.equal(detectKey([]).candidates.length,0);assert.equal(analyzeVocal(new Float32Array(48000),48000).notes.length,0);
assert.throws(()=>renderNotes([sine(220,48000)],48000,[{start:0,end:1,midi:57,target:90,edited:true}]),/octave/);
assert.equal(midiToHz(69),440);
console.log('PASS: C-major key profile, insufficient-evidence gating, silence and invalid edits');
