// Browser extension after the portable WASM chain. All sample histories are
// allocated in construction; UI controls arrive through the worklet port.
const clamp = (x, lo, hi) => Math.max(lo, Math.min(hi, x));
const db = x => 20 * Math.log10(Math.max(1e-12, x));
const smoothKeys=['smooth','warmth','presence','deess'];
const limit=v=>Math.abs(v)>.98?Math.sign(v)*(.98+.02*Math.tanh((Math.abs(v)-.98)/.02)):v;
class Filter {
  constructor() { this.b0=1;this.b1=this.b2=this.a1=this.a2=this.z1=this.z2=0; }
  set(b0,b1,b2,a1,a2) { Object.assign(this,{b0,b1,b2,a1,a2}); }
  peak(sr,hz,q,gain) {
    const a=10**(gain/40),w=2*Math.PI*Math.min(hz,sr*.45)/sr,alpha=Math.sin(w)/(2*q),c=Math.cos(w),norm=1+alpha/a;
    this.set((1+alpha*a)/norm,-2*c/norm,(1-alpha*a)/norm,-2*c/norm,(1-alpha/a)/norm);
  }
  process(x) { const y=this.b0*x+this.z1;this.z1=this.b1*x-this.a1*y+this.z2;this.z2=this.b2*x-this.a2*y;return y; }
}
function weighting(sr) {
  const shelf=new Filter(),hp=new Filter();
  let k=Math.tan(Math.PI*1681.974450955533/sr),q=.7071752369554196,vh=10**(3.999843853973347/20),vb=vh**.4996667741545416,a=1+k/q+k*k;
  shelf.set((vh+vb*k/q+k*k)/a,2*(k*k-vh)/a,(vh-vb*k/q+k*k)/a,2*(k*k-1)/a,(1-k/q+k*k)/a);
  k=Math.tan(Math.PI*38.13547087602444/sr);q=.5003270373238773;a=1+k/q+k*k;
  hp.set(1,-2,1,2*(k*k-1)/a,(1-k/q+k*k)/a);
  return [shelf,hp];
}
export class VocalTools {
  constructor(sr) {
    this.sr=sr;this.target={smooth:0,harsh:true,warmth:0,presence:0,deess:0,deessHz:6000,deessThreshold:-28};
    this.smooth={smooth:0,warmth:0,presence:0,deess:0};this.enabled=true;this.wet=0;
    this.lp=[0,0];this.filters=Array.from({length:2},()=>Array.from({length:3},()=>new Filter()));
    this.k=[weighting(sr),weighting(sr)];this.energy=new Float64Array(Math.round(sr*.4));this.energyPos=this.energyCount=this.energySum=0;
    this.history=new Float32Array(4096);this.historyPos=this.historyCount=0;
    this.inputSum=this.inputN=this.inputPeak=this.leftPeak=this.rightPeak=this.peakHold=0;
    this.envHigh=this.envFull=this.reduction=this.harshCut=0;this.control=0;this.left=this.right=0;
    this.minMidi=Infinity;this.maxMidi=-Infinity;this.voicedCount=0;this.voicedRun=0;this.notes=new Uint32Array(128);
    this.slew=1-Math.exp(-1/(sr*.02));this.attack=1-Math.exp(-1/(sr*.002));this.release=1-Math.exp(-1/(sr*.08));
    this.autoHigh=this.autoFull=this.autoN=0;this.capturing=false;
  }
  set(values={},enabled=true) {
    for(const [k,lo,hi] of [['smooth',0,100],['warmth',-100,100],['presence',-100,100],['deess',0,100],['deessHz',3000,10000],['deessThreshold',-48,-6]]) {
      if(Number.isFinite(values[k]))this.target[k]=clamp(values[k],lo,hi);
    }
    if(typeof values.harsh==='boolean')this.target.harsh=values.harsh;
    this.enabled=!!enabled;
  }
  beginAuto() { this.autoHigh=this.autoFull=this.autoN=0;this.capturing=true; }
  autoSuggestion() {
    this.capturing=false;
    const ratio=this.autoHigh/Math.max(1e-12,this.autoFull);
    return {smooth:ratio>.15?45:25,harsh:true,warmth:ratio>.3?15:0,presence:ratio<.08?10:0,deess:Math.round(clamp(ratio*140,15,65)),deessHz:6000,deessThreshold:-28};
  }
  observeInput(l,r) {
    const x=(l+r)*.5;this.history[this.historyPos]=x;this.historyPos=(this.historyPos+1)%4096;this.historyCount=Math.min(4096,this.historyCount+1);
    this.inputSum+=(l*l+r*r)*.5;++this.inputN;this.inputPeak=Math.max(this.inputPeak,Math.abs(l),Math.abs(r));
  }
  process(l,r,measure=true) {
    this.wet+=this.slew*((this.enabled?1:0)-this.wet);
    for(const key of smoothKeys)this.smooth[key]+=this.slew*(this.target[key]-this.smooth[key]);
    const lpCoef=1-Math.exp(-2*Math.PI*this.target.deessHz/this.sr);
    this.lp[0]+=lpCoef*(l-this.lp[0]);this.lp[1]+=lpCoef*(r-this.lp[1]);
    const hl=l-this.lp[0],hr=r-this.lp[1],full=(l*l+r*r)*.5,high=(hl*hl+hr*hr)*.5;
    this.envFull+=this.slew*(full-this.envFull);
    this.envHigh+=(high>this.envHigh?this.attack:this.release)*(high-this.envHigh);
    if(this.capturing){this.autoFull+=full;this.autoHigh+=high;++this.autoN;}
    const dominance=this.envHigh/Math.max(1e-12,this.envFull);
    const wanted=dominance>.08?clamp((db(Math.sqrt(this.envHigh))-this.target.deessThreshold)*.8,0,18)*this.smooth.deess/100:0;
    this.reduction+=(wanted>this.reduction?this.attack:this.release)*(wanted-this.reduction);
    const highGain=10**(-this.reduction/20);
    l-=hl*(1-highGain);r-=hr*(1-highGain);
    const cut=-(this.target.harsh?9:4)*this.smooth.smooth/100*clamp(dominance*3,0,1);
    this.harshCut+=this.slew*(cut-this.harshCut);
    if(this.control++%64===0)for(const filters of this.filters) {
      filters[0].peak(this.sr,3300,.8,this.harshCut);
      filters[1].peak(this.sr,180,.7,this.smooth.warmth*.06);
      filters[2].peak(this.sr,3200,.7,this.smooth.presence*.06);
    }
    let x=l,y=r;
    for(const filter of this.filters[0])x=filter.process(x);
    for(const filter of this.filters[1])y=filter.process(y);
    // The wet mix bypasses every added effect. Avoid division or allocation.
    // Reconstruct the WASM output before high-band attenuation for bypass.
    const originalL=l+hl*(1-highGain),originalR=r+hr*(1-highGain);
    x=originalL+(x-originalL)*this.wet;y=originalR+(y-originalR)*this.wet;
    // Only engage the extra peak guard when a tonal boost is requested.
    if(this.wet>.0001&&(this.smooth.warmth>0||this.smooth.presence>0)){x=limit(x);y=limit(y);}
    this.left=Number.isFinite(x)?x:0;this.right=Number.isFinite(y)?y:0;
    if(measure)this.measureOutput(this.left,this.right);
  }
  measureOutput(l,r){
    this.left=l;this.right=r;
    this.leftPeak=Math.max(this.leftPeak,Math.abs(this.left));this.rightPeak=Math.max(this.rightPeak,Math.abs(this.right));
    this.peakHold=Math.max(this.peakHold,this.leftPeak,this.rightPeak);
    let kl=this.left,kr=this.right;
    for(const f of this.k[0])kl=f.process(kl);for(const f of this.k[1])kr=f.process(kr);
    const energy=kl*kl+kr*kr;
    this.energySum+=energy-this.energy[this.energyPos];this.energy[this.energyPos]=energy;
    this.energyPos=(this.energyPos+1)%this.energy.length;this.energyCount=Math.min(this.energy.length,this.energyCount+1);
  }
  confidence(hz) {
    if(!(hz>0)||this.historyCount<2048)return 0;
    const lag=Math.round(this.sr/hz),n=Math.min(2048,this.historyCount-lag-2);if(n<256)return 0;
    let best=0;
    for(let offset=-1;offset<=1;offset++) {
      let xy=0,xx=0,yy=0,sx=0,sy=0;
      for(let i=0;i<n;i+=2) {
        const x=this.history[(this.historyPos-1-i+8192)%4096],y=this.history[(this.historyPos-1-i-lag-offset+8192)%4096];
        xy+=x*y;xx+=x*x;yy+=y*y;sx+=x;sy+=y;
      }
      const count=Math.ceil(n/2);xx-=sx*sx/count;yy-=sy*sy/count;xy-=sx*sy/count;
      if(xx>1e-10&&yy>1e-10)best=Math.max(best,xy/Math.sqrt(xx*yy));
    }
    return clamp(best,0,1);
  }
  resetPeak() { this.peakHold=0; }
  snapshot(hz) {
    const confidence=this.confidence(hz),rms=Math.sqrt(this.inputSum/Math.max(1,this.inputN));
    if(confidence>.65&&rms>.0001) {
      if(++this.voicedRun>=3){const midi=clamp(Math.round(69+12*Math.log2(hz/440)),0,127);this.minMidi=Math.min(this.minMidi,midi);this.maxMidi=Math.max(this.maxMidi,midi);this.notes[midi]++;this.voicedCount++;}
    } else this.voicedRun=0;
    let register='More notes needed';
    if(this.voicedCount>=30&&this.maxMidi-this.minMidi>=7) {
      let total=0,median=0;for(;median<128;median++){total+=this.notes[median];if(total>=this.voicedCount/2)break;}
      register=median<48?'Bass':median<55?'Baritone':median<60?'Tenor':median<65?'Alto':median<69?'Mezzo':'Soprano';
    }
    const energy=this.energySum/Math.max(1,this.energyCount);
    const result={confidence,rms,input:this.inputPeak,left:this.leftPeak,right:this.rightPeak,output:Math.max(this.leftPeak,this.rightPeak),peak:this.peakHold,
      lufs:this.energyCount===this.energy.length&&energy>1e-12?-.691+10*Math.log10(energy):null,
      range:Number.isFinite(this.minMidi)?[this.minMidi,this.maxMidi]:null,register,deessReduction:this.reduction*this.wet,smoothReduction:-this.harshCut*this.wet};
    this.inputSum=this.inputN=this.inputPeak=this.leftPeak=this.rightPeak=0;return result;
  }
}

// Stereo-linked downward expansion. Reduces quiet noise between phrases;
// it does not separate noise from a voice while both are sounding.
export class VocalDenoise {
  constructor(sr){this.envelope=0;this.gain=1;this.amount=0;this.threshold=-45;this.enabled=true;this.attack=1-Math.exp(-1/(sr*.002));this.release=1-Math.exp(-1/(sr*.12));}
  set(values={},enabled=true){this.amount=clamp(Number.isFinite(values.denoise)?values.denoise:0,0,100);this.threshold=clamp(Number.isFinite(values.noiseThreshold)?values.noiseThreshold:-45,-72,-12);this.enabled=enabled;}
  process(l,r){const peak=Math.max(Math.abs(l),Math.abs(r));this.envelope+=(peak>this.envelope?this.attack:this.release)*(peak-this.envelope);const below=Math.min(0,db(this.envelope)-this.threshold);const target=this.enabled?10**(Math.max(-48,below*2)*this.amount/100/20):1;this.gain+=(target>this.gain?this.attack:this.release)*(target-this.gain);return this.amount===0||!this.enabled?1:this.gain;}
}

// Original opto-style envelope compressor, not a circuit model of branded hardware.
export class VocalOpto {
  constructor(sr){this.energy=0;this.reduction=0;this.slow=0;this.threshold=-18;this.makeup=0;this.limit=false;this.enabled=false;this.mix=1;this.sidechain=80;this.lp=[0,0];this.rate=sr;this.detector=1-Math.exp(-1/(sr*.01));this.attack=1-Math.exp(-1/(sr*.01));this.fast=1-Math.exp(-1/(sr*.08));this.tail=1-Math.exp(-1/(sr*1.5));this.slew=1-Math.exp(-1/(sr*.02));}
  set(params,extras,enabled){this.enabled=!!extras.opto&&enabled&&params[9]>1;this.threshold=params[8];this.makeupTarget=params[12];this.limit=!!extras.limit;this.mixTarget=clamp((extras.optoMix??100)/100,0,1);this.sidechain=clamp(extras.sidechainHz??80,20,250);}
  process(l,r){const k=1-Math.exp(-2*Math.PI*this.sidechain/this.rate);this.lp[0]+=k*(l-this.lp[0]);this.lp[1]+=k*(r-this.lp[1]);const x=l-this.lp[0],y=r-this.lp[1];this.energy+=this.detector*((x*x+y*y)*.5-this.energy);const over=db(Math.sqrt(this.energy))-this.threshold,slope=this.limit?.95:.75,knee=6;const wanted=this.enabled?(over<=-knee/2?0:over>=knee/2?over*slope:slope*(over+knee/2)**2/(2*knee)):0;this.slow+=this.tail*(wanted-this.slow);const target=Math.max(wanted,this.slow*.55);this.reduction+=(target>this.reduction?this.attack:this.fast)*(target-this.reduction);this.makeup+=this.slew*((this.makeupTarget??0)-this.makeup);this.mix+=this.slew*((this.mixTarget??1)-this.mix);return this.enabled?((1-this.mix)+this.mix*10**(-this.reduction/20))*10**(this.makeup/20):1;}
}

// Fixed-memory stereo lookahead guard. Its extra 128 samples are exposed in UI.
export class VocalSafety {
  constructor(sr){this.l=new Float32Array(128);this.r=new Float32Array(128);this.peaks=new Float32Array(256);this.indices=new Float64Array(256);this.head=0;this.tail=0;this.n=0;this.gain=1;this.release=1-Math.exp(-1/(sr*.08));this.left=0;this.right=0;}
  process(l,r,enabled=true){const n=this.n++,pos=n%128,dl=this.l[pos],dr=this.r[pos];this.l[pos]=l;this.r[pos]=r;const peak=Math.max(Math.abs(l),Math.abs(r));while(this.head<this.tail&&this.indices[this.head%256]<n-128)this.head++;while(this.tail>this.head&&this.peaks[(this.tail-1)%256]<=peak)this.tail--;this.peaks[this.tail%256]=peak;this.indices[this.tail%256]=n;this.tail++;const max=this.peaks[this.head%256],wanted=max>.95?.95/max:1;this.gain=wanted<this.gain?wanted:this.gain+this.release*(wanted-this.gain);this.left=enabled?dl*this.gain:l;this.right=enabled?dr*this.gain:r;}
}

// Pre-effect stereo band energies: repeated Auto passes do not analyse their own EQ.
export class AutoVocal {
  constructor(sr){this.coef=[150,500,2500,6000].map(f=>1-Math.exp(-2*Math.PI*f/sr));this.lp=new Float64Array(8);this.bands=new Float64Array(5);this.start();this.running=false;}
  start(){this.sum=0;this.peak=0;this.count=0;this.lp?.fill(0);this.bands?.fill(0);this.running=true;}
  observe(l,r){if(!this.running)return;this.sum+=(l*l+r*r)*.5;this.peak=Math.max(this.peak,Math.abs(l),Math.abs(r));this.count++;for(let c=0;c<2;c++){const x=c?r:l;let previous=0;for(let b=0;b<4;b++){const i=c*4+b;this.lp[i]+=this.coef[b]*(x-this.lp[i]);const band=this.lp[i]-previous;this.bands[b]+=band*band*.5;previous=this.lp[i];}this.bands[4]+=(x-previous)**2*.5;}}
  result(){this.running=false;const rms=Math.sqrt(this.sum/Math.max(1,this.count)),level=db(rms),crest=db(this.peak)-level,total=this.bands.reduce((a,b)=>a+b,0)||1,ratios=Array.from(this.bands,v=>v/total),threshold=clamp(level-5,-36,-10),gr=Math.max(0,level-threshold)*.75,makeup=clamp(Math.min(-20-level+gr,-2-db(this.peak)+gr),0,9),mud=clamp((ratios[1]-.22)*-10,-4,0),presence=ratios[3]<.08?2:ratios[3]>.4?-2:0;
    return {params:[[2,0],[4,crest>14?45:25],[5,35],[7,100],[8,threshold],[9,4],[10,10],[11,180],[12,makeup],[13,90],[14,ratios[0]>.35?-3:0],[15,.7],[16,300],[17,mud],[18,.9],[19,1200],[20,0],[21,1],[22,3500],[23,presence],[24,.8],[25,10000],[26,ratios[4]<.04?1:0],[27,.7],[28,.02],[31,.04]],extras:{smooth:ratios[3]+ratios[4]>.3?40:20,harsh:true,warmth:ratios[0]<.08?8:0,presence:0,deess:clamp(Math.round(ratios[4]*180),15,55),deessHz:6000,deessThreshold:clamp(level-8,-40,-18),opto:true,limit:false,optoMix:100,sidechainHz:80,safety:true},stats:{inputRms:level,crest,ratios}};
  }
}
