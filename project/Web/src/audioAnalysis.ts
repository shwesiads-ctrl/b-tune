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
export async function analyzeBuffer(buffer:AudioBuffer,signal?:AbortSignal,onProgress?:(fraction:number)=>void) {
  if(buffer.duration>600)throw new Error('Use a vocal up to 10 minutes for note/key analysis.');
  const frames:AnalysisResult['frames']=[],notes:VocalNote[]=[];
  const chunk=Math.round(buffer.sampleRate*30);
  for(let start=0;start<buffer.length;start+=chunk){
    if(signal?.aborted)throw new DOMException('Audio operation cancelled.','AbortError');
    const length=Math.min(chunk,buffer.length-start),samples=new Float32Array(length);
    for(let c=0;c<buffer.numberOfChannels;c++){const channel=buffer.getChannelData(c);for(let i=0;i<length;i++)samples[i]+=channel[start+i]/buffer.numberOfChannels;}
    const result=await runJob<AnalysisResult>({type:'analyze',samples,rate:buffer.sampleRate},[samples.buffer],signal),offset=start/buffer.sampleRate;
    frames.push(...result.frames.map(f=>({...f,time:f.time+offset})));
    for(const n of result.notes)notes.push({...n,id:notes.length,start:n.start+offset,end:n.end+offset});
    onProgress?.(Math.min(1,(start+length)/buffer.length));
  }
  const key=await keyFromFrames(frames,signal);
  return {frames,notes,key,duration:buffer.duration};
}
export function renderBuffer(buffer:AudioBuffer,notes:VocalNote[],signal?:AbortSignal) {
  if(buffer.duration>600||buffer.numberOfChannels>2||buffer.length*buffer.numberOfChannels>60000000)throw new Error('Use a mono/stereo vocal up to 10 minutes and 60 million channel samples for rendering.');
  const channels=Array.from({length:Math.min(2,buffer.numberOfChannels)},(_,i)=>new Float32Array(buffer.getChannelData(i)));
  return runJob<{channels:Float32Array[];wav:ArrayBuffer}>({type:'render',channels,rate:buffer.sampleRate,notes},channels.map(c=>c.buffer),signal);
}

export function keyFromFrames(frames:Pick<PitchFrame,'duration'|'midi'|'confidence'>[],signal?:AbortSignal){return runJob<KeyResult>({type:'key',frames},[],signal);}
