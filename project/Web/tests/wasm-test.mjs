import { readFileSync } from 'node:fs';
import assert from 'node:assert/strict';
const bytes=readFileSync(new URL('../public/matale-dsp.wasm',import.meta.url));
const module=await WebAssembly.compile(bytes);
console.log('WASM imports',WebAssembly.Module.imports(module));
const e=new WebAssembly.Instance(module,{
  env:{emscripten_notify_memory_growth(){}},
  wasi_snapshot_preview1:{proc_exit(c){throw Error('Exit '+c);},fd_write(){return 0;},fd_close(){return 0;},fd_seek(){return 0;}}
}).exports;
e._initialize?.();
e.mt_init(48000);
const l=new Float32Array(e.memory.buffer,e.mt_left(),128),r=new Float32Array(e.memory.buffer,e.mt_right(),128);
const initial=e.memory.buffer.byteLength;
e.mt_set(7,0);e.mt_set(9,1);
for(let b=0;b<400;b++) {
  for(let i=0;i<128;i++)l[i]=r[i]=.2*Math.sin(2*Math.PI*440*(b*128+i)/48000);
  e.mt_process();
  for(const v of l)assert(Number.isFinite(v)&&Math.abs(v)<=1);
}
assert(Math.abs(e.mt_pitch()-440)<2,'WASM pitch detection');
e.mt_auto();
for(let b=0;b<1125;b++){
  for(let i=0;i<128;i++)l[i]=r[i]=.2*Math.sin(2*Math.PI*220*(b*128+i)/48000);
  e.mt_process();
}
assert.equal(e.mt_auto_ready(),1,'WASM Auto analyser');
assert(e.mt_auto_value(8)<-15);
assert.equal(e.memory.buffer.byteLength,initial,'No wasm memory growth');
console.log('PASS: WASM init, block processing, pitch, finite output, Auto and fixed memory');
