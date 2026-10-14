import assert from 'node:assert/strict';
import { VocalTools } from '../public/vocal-tools.js';
function render(hz,settings={},enabled=true,rate=48000,amplitude=.1) {
  const fx=new VocalTools(rate);fx.set(settings,enabled);let sum=0,original=0;
  for(let n=0;n<rate;n++) {
    const x=amplitude*Math.sin(2*Math.PI*hz*n/rate);fx.observeInput(x,x);fx.process(x,x);
    assert(Number.isFinite(fx.left)&&Number.isFinite(fx.right));assert.equal(fx.left,fx.right);
    if(n>rate/2){sum+=fx.left**2;original+=x*x;}
  }
  return {gain:10*Math.log10(sum/original),fx,meter:fx.snapshot(hz)};
}
for(const rate of [44100,48000,96000]) {
  const neutral=render(1000,{},true,rate);
  assert(Math.abs(neutral.gain)<1e-9,'neutral path changed');
  assert(Math.abs(neutral.meter.lufs+20)<.2,'K-weighted stereo 1kHz calibration differs');
  assert(neutral.meter.confidence>.99,'periodic sine confidence too low');
  assert(Math.abs(neutral.meter.rms-Math.SQRT1_2*.1)<.001);
  const deess=render(8000,{deess:100,deessThreshold:-36},true,rate);
  assert(deess.gain < -1.5,'De-Esser does not attenuate sibilant band');
  assert(render(200,{deess:100,deessThreshold:-36},true,rate).gain>-.1,'De-Esser damages low-frequency tone');
  const focused=render(3300,{smooth:100,harsh:true},true,rate).gain;
  assert(focused < -1,'Smooth does not reduce harsh band');
  assert(render(3300,{smooth:100,harsh:false},true,rate).gain>focused+.5,'Harsh switch has no actual effect');
  assert(render(180,{warmth:100},true,rate).gain > 5,'Warmth control does not boost low mids');
  assert(render(3200,{presence:100},true,rate).gain > 5,'Presence control does not boost presence');
  assert(Math.abs(render(8000,{smooth:100,deess:100,warmth:100,presence:100},false,rate).gain)<1e-9,'Power bypass changes original signal');
  console.log('PASS: real attenuation/boost, exact bypass, confidence and LUFS at',rate,'Hz');
}
const silent=new VocalTools(48000);
for(let i=0;i<48000;i++){silent.observeInput(0,0);silent.process(0,0);}
const m=silent.snapshot(0);assert.equal(m.lufs,null);assert.equal(m.confidence,0);assert.equal(m.range,null);assert.equal(m.rms,0);
const tone=new VocalTools(48000);let range;for(let b=0;b<6;b++){for(let i=0;i<2048;i++){const x=.1*Math.sin(2*Math.PI*440*(b*2048+i)/48000);tone.observeInput(x,x);tone.process(x,x);}range=tone.snapshot(440);}assert.deepEqual(range.range,[69,69]);assert.equal(range.register,'More notes needed');
assert(tone.peakHold>0);tone.resetPeak();assert.equal(tone.peakHold,0);
console.log('PASS: silence, observed note range, honest register gating and peak reset');

const stereo=new VocalTools(48000);for(let i=0;i<24000;i++){stereo.observeInput(.3,.1);stereo.process(.3,.1);}const levels=stereo.snapshot(0);assert(Math.abs(levels.left-.3)<1e-8);assert(Math.abs(levels.right-.1)<1e-8);console.log('PASS: independent real L/R peak meters');
