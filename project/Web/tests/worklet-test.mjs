import { readFileSync } from 'node:fs';
import { runInNewContext } from 'node:vm';
import assert from 'node:assert/strict';
import { VocalTools } from '../public/vocal-tools.js';
let Processor;const messages=[];
const bytes=readFileSync(new URL('../public/matale-dsp.wasm',import.meta.url));
const module=await WebAssembly.compile(bytes);
runInNewContext(readFileSync(new URL('../public/matale-worklet.js',import.meta.url),'utf8').replace(/^import .*;\n/, ''),{
  VocalTools,
  AudioWorkletProcessor:class {constructor(){this.port={postMessage:m=>messages.push(structuredClone(m))};}},
  registerProcessor:(name,c)=>{Processor=c;},
  sampleRate:48000,WebAssembly,Float32Array,Error
});
const p=new Processor({processorOptions:{module}});
const defaults=JSON.parse(readFileSync(new URL('../src/parameters.json',import.meta.url)));
const values=defaults.map(p=>p.initial);values[9]=1;
p.port.onmessage({data:{params:values}});
const a=new Float32Array(128),b=new Float32Array(128),left=new Float32Array(128),right=new Float32Array(128);
for(let block=0;block<1200;block++){
  if(block===10)p.port.onmessage({data:{auto:true}});
  for(let i=0;i<128;i++)a[i]=b[i]=.2*Math.sin(2*Math.PI*220*(block*128+i)/48000);
  assert.equal(p.process([[a,b]],[[left,right]]),true);
  for(let i=0;i<128;i++){assert(Number.isFinite(left[i]));assert.equal(left[i],right[i]);}
}
assert(messages.some(m=>m.pitch>215&&m.pitch<225));
assert(messages.some(m=>m.autoResult?.length===8));
console.log('PASS: actual worklet adapter stereo output, telemetry and Auto messages');
if(process.argv[2]){
 const raw=readFileSync(process.argv[2]),fixture=new Float32Array(raw.buffer,raw.byteOffset,raw.byteLength/4);
 const q=new Processor({processorOptions:{module}});q.port.onmessage({data:{params:values}});
 let peak=0,energy=0,n=0;
 for(let offset=0;offset<fixture.length;offset+=256){
   a.set(fixture.subarray(offset,offset+128));b.set(a);
   q.process([[a,b]],[[left,right]]);
   for(let i=0;i<128;i++){const diff=Math.abs(left[i]-fixture[offset+128+i]);peak=Math.max(peak,diff);energy+=diff*diff;n++;}
 }
 console.log('Native/WASM parity peak:',peak,'RMS:',Math.sqrt(energy/n));
 assert(peak<.001,'Native / WASM output differs by >= .001');
}
