import type { AnalysisResult, VocalNote, KeyResult, PitchFrame } from '../public/pitch-editor.js';
function runJob<T>(payload:unknown,transfer:Transferable[],signal?:AbortSignal):Promise<T> {
  return new Promise((resolve,reject)=>{
    const worker=new Worker('/note-worker.js',{type:'module'});
    const clean=()=>{worker.terminate();signal?.removeEventListener('abort',abort);};
    const abort=()=>{clean();reject(new DOMException('Audio operation cancelled.','AbortError'));};
    if(signal?.aborted){abort();return;}signal?.addEventListener('abort',abort,{once:true});
    worker.onmessage=({data})=>{clean();if(data.ok)resolve(data.result);else reject(new Error(data.error));};
    worker.onerror=()=>{clean();reject(new Error('Note worker could not run. Reload the page and retry.'));};
    worker.postMessage(payload,transfer);
  });
}
export function analyzeBuffer(buffer:AudioBuffer,signal?:AbortSignal) {
  if(buffer.duration>120)throw new Error('Note/key analysis supports clips up to 120 seconds. Trim the vocal first.');
  const samples=new Float32Array(buffer.length);
  for(let c=0;c<buffer.numberOfChannels;c++){const channel=buffer.getChannelData(c);for(let i=0;i<samples.length;i++)samples[i]+=channel[i]/buffer.numberOfChannels;}
  return runJob<AnalysisResult>({type:'analyze',samples,rate:buffer.sampleRate},[samples.buffer],signal);
}
export function renderBuffer(buffer:AudioBuffer,notes:VocalNote[],signal?:AbortSignal) {
  if(buffer.duration>120||buffer.numberOfChannels>2)throw new Error('Use a mono/stereo vocal clip up to 120 seconds for note editing.');
  const channels=Array.from({length:Math.min(2,buffer.numberOfChannels)},(_,i)=>new Float32Array(buffer.getChannelData(i)));
  return runJob<{channels:Float32Array[];wav:ArrayBuffer}>({type:'render',channels,rate:buffer.sampleRate,notes},channels.map(c=>c.buffer),signal);
}

export function keyFromFrames(frames:Pick<PitchFrame,'duration'|'midi'|'confidence'>[],signal?:AbortSignal){return runJob<KeyResult>({type:'key',frames},[],signal);}
