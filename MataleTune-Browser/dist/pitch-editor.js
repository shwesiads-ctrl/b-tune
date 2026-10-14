// Original monophonic pitch analysis/key estimation and offline note rendering.
// No Celemony code or product integration is included.
const clamp=(x,a,b)=>Math.max(a,Math.min(b,x));
export const midiToHz=m=>440*2**((m-69)/12);
export function detectKey(frames) {
  const histogram=new Float64Array(12);let voiced=0;
  for(const f of frames)if(Number.isFinite(f.midi)&&f.confidence>=.75&&f.duration>0){histogram[((Math.round(f.midi)%12)+12)%12]+=f.duration;voiced+=f.duration;}
  const distinct=Array.from(histogram).filter(v=>v>.06).length;
  if(voiced<1.5||distinct<3)return {candidates:[],ambiguous:true,reason:'Sing at least three different notes for 1.5 seconds or analyse a longer vocal file.'};
  const major=[6.35,2.23,3.48,2.33,4.38,4.09,2.52,5.19,2.39,3.66,2.29,2.88],minor=[6.33,2.68,3.52,5.38,2.6,3.53,2.54,4.75,3.98,2.69,3.34,3.17];
  const mean=voiced/12;let xx=0;for(const h of histogram)xx+=(h-mean)**2;
  const candidates=[];
  for(let root=0;root<12;root++)for(const [mode,profile] of [['Major',major],['Minor',minor]]) {
    const avg=profile.reduce((a,b)=>a+b)/12;let xy=0,yy=0;
    for(let pc=0;pc<12;pc++){const p=profile[(pc-root+12)%12]-avg;xy+=(histogram[pc]-mean)*p;yy+=p*p;}
    candidates.push({root,mode,score:xx>1e-12?xy/Math.sqrt(xx*yy):0});
  }
  candidates.sort((a,b)=>b.score-a.score);
  const ambiguous=candidates[0].score<.4||candidates[0].score-candidates[1].score<.035;
  return {candidates:candidates.slice(0,3),ambiguous,reason:ambiguous?'Several keys fit this melody. Audition the candidates before applying.':'Best matching pitch-class profile; confirm against the accompaniment.'};
}
export function analyzeVocal(samples,sr) {
  if(!(samples instanceof Float32Array)||!Number.isFinite(sr)||sr<8000||sr>192000)throw new Error('Invalid audio for note analysis.');
  if(samples.length/sr>120)throw new Error('Note editor supports clips up to 120 seconds. Trim the vocal first.');
  const dec=Math.max(1,Math.ceil(sr/12000)),rate=sr/dec,lp=1-Math.exp(-2*Math.PI*2000/sr),down=new Float32Array(Math.ceil(samples.length/dec));
  let a=0,b=0,count=0;
  for(let i=0;i<samples.length;i++){a+=lp*((Number.isFinite(samples[i])?samples[i]:0)-a);b+=lp*(a-b);if(i%dec===dec-1)down[count++]=b;}
  const frames=[],diff=new Float64Array(256),low=Math.max(2,Math.floor(rate/1000)),high=Math.min(254,Math.floor(rate/70)),hop=128;
  for(let end=1024;end<count;end+=hop) {
    let energy=0;for(let i=0;i<512;i++)energy+=down[end-1-i]**2;
    if(energy/512<1e-7)continue;
    let total=0;
    for(let lag=1;lag<=high;lag++){let sum=0;for(let i=0;i<512;i++){const v=down[end-1-i]-down[end-1-i-lag];sum+=v*v;}total+=sum;diff[lag]=total>1e-20?sum*lag/total:1;}
    for(let lag=low;lag<high-1;lag++)if(diff[lag]<.15) {
      while(lag+1<high&&diff[lag+1]<diff[lag])lag++;
      const den=diff[lag-1]-2*diff[lag]+diff[lag+1],offset=Math.abs(den)>1e-9?.5*(diff[lag-1]-diff[lag+1])/den:0;
      const hz=rate/(lag+clamp(offset,-.5,.5)),midi=69+12*Math.log2(hz/440),time=(end-256)*dec/sr;
      frames.push({time,duration:hop/rate,midi,hz,confidence:clamp(1-diff[lag],0,1)});break;
    }
  }
  const groups=[];
  for(const f of frames) {
    const previous=groups[groups.length-1],note=Math.round(f.midi);
    if(previous&&previous.note===note&&f.time-previous.end<.055){previous.end=f.time+f.duration/2;previous.values.push(f.midi);previous.confidence+=f.confidence;}
    else groups.push({note,start:Math.max(0,f.time-f.duration/2),end:f.time+f.duration/2,values:[f.midi],confidence:f.confidence});
  }
  const notes=groups.filter(g=>g.end-g.start>=.08).map((g,id)=>{
    g.values.sort((a,b)=>a-b);const midi=g.values[Math.floor(g.values.length/2)];
    return {id,start:g.start,end:Math.min(samples.length/sr,g.end),midi,target:Math.round(midi),edited:false,confidence:g.confidence/g.values.length};
  });
  return {frames,notes,key:detectKey(frames),duration:samples.length/sr};
}
// Fixed-duration two-grain SOLA-style shifter. Coordinates wrap into a fixed
// ring; source audio remains available around the 2048-sample nominal delay.
class Shifter {
  constructor(sr){this.grain=Math.max(512,Math.round(2048*sr/48000/2)*2);this.latency=this.grain;this.size=this.grain*8;this.factor=sr/48000;this.ring=new Float32Array(this.size);this.write=0;this.a={p:-this.latency,age:0};this.b={p:-this.latency,age:this.grain/2};}
  read(p){p=((p%this.size)+this.size)%this.size;const i=Math.floor(p),f=p-i;return this.ring[i]+f*(this.ring[(i+1)%this.size]-this.ring[i]);}
  launch(head,other,ratio){const anchor=this.write-this.latency-this.grain*.5*(ratio-1),search=Math.round(96*this.factor),step=Math.max(1,Math.round(4*this.factor));let best=-2,offsetBest=0;
    for(let offset=-search;offset<=search;offset+=step){let xy=0,xx=1e-15,yy=1e-15;for(let j=0;j<64*this.factor;j+=step){const x=this.read(other.p+j*ratio),y=this.read(anchor+offset+j*ratio);xy+=x*y;xx+=x*x;yy+=y*y;}const score=xy/Math.sqrt(xx*yy)-.0001*Math.abs(offset)/this.factor;if(score>best){best=score;offsetBest=offset;}}
    head.p=anchor+offsetBest;head.age=0;
  }
  process(x,ratio){this.ring[this.write]=x;if(this.a.age>=this.grain)this.launch(this.a,this.b,ratio);if(this.b.age>=this.grain)this.launch(this.b,this.a,ratio);
    const wa=.5-.5*Math.cos(2*Math.PI*this.a.age/this.grain),wb=.5-.5*Math.cos(2*Math.PI*this.b.age/this.grain);
    const y=(wa*this.read(this.a.p)+wb*this.read(this.b.p))/Math.max(.001,wa+wb);
    this.a.p+=ratio;this.b.p+=ratio;this.a.age++;this.b.age++;this.write=(this.write+1)%this.size;return y;
  }
}
export function renderNotes(channels,sr,notes) {
  if(!Array.isArray(channels)||!channels.length||channels.length>2||!channels.every(c=>c instanceof Float32Array&&c.length===channels[0].length))throw new Error('A mono or stereo vocal file is required.');
  const length=channels[0].length;
  if(!(sr>=8000&&sr<=192000)||length/sr>120)throw new Error('Note rendering supports clips up to 120 seconds.');
  const edits=notes.filter(n=>n.edited).sort((a,b)=>a.start-b.start);
  for(const n of edits)if(!Number.isFinite(n.start)||!Number.isFinite(n.end)||!Number.isFinite(n.midi)||!Number.isFinite(n.target)||n.start<0||n.end<=n.start||n.end>length/sr+.01||Math.abs(n.target-n.midi)>12.5)throw new Error('Invalid note edit or shift beyond one octave.');
  if(!edits.length)return channels.map(c=>new Float32Array(c));
  const result=channels.map(()=>new Float32Array(length)),shifters=channels.map(()=>new Shifter(sr)),k=1-Math.exp(-1/(sr*.01));let index=0,ratio=1,wet=0;
  const latency=shifters[0].latency;
  for(let n=0;n<length+latency;n++) {
    const time=(n-latency)/sr;
    while(index<edits.length&&edits[index].end<=time)index++;
    const note=edits[index],active=note&&time>=note.start&&time<note.end,wanted=active?clamp(2**((note.target-note.midi)/12),.5,2):1;
    ratio+=k*(wanted-ratio);wet+=k*((active?1:0)-wet);
    for(let c=0;c<channels.length;c++){
      const shifted=shifters[c].process(n<length?channels[c][n]:0,ratio),dry=n>=latency?channels[c][n-latency]:0;
      if(n>=latency)result[c][n-latency]=dry+(shifted-dry)*wet;
    }
  }
  return result;
}
export function encodeWav(channels,sr) {
  const frames=channels[0].length,bytes=new ArrayBuffer(44+frames*channels.length*2),view=new DataView(bytes);
  const str=(offset,s)=>{for(let i=0;i<s.length;i++)view.setUint8(offset+i,s.charCodeAt(i));};
  str(0,'RIFF');view.setUint32(4,bytes.byteLength-8,true);str(8,'WAVEfmt ');view.setUint32(16,16,true);view.setUint16(20,1,true);view.setUint16(22,channels.length,true);
  view.setUint32(24,sr,true);view.setUint32(28,sr*channels.length*2,true);view.setUint16(32,channels.length*2,true);view.setUint16(34,16,true);str(36,'data');view.setUint32(40,bytes.byteLength-44,true);
  let offset=44;for(let i=0;i<frames;i++)for(const channel of channels){const v=clamp(Number.isFinite(channel[i])?channel[i]:0,-1,1);view.setInt16(offset,Math.round(v*(v<0?32768:32767)),true);offset+=2;}
  return bytes;
}
