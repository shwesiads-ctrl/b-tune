import { VocalTools, VocalDenoise, VocalOpto, VocalSafety, AutoVocal } from './vocal-tools.js';
// Audio processing never runs on React's/main UI thread.
class MataleProcessor extends AudioWorkletProcessor {
  constructor(options) {
    super();
    const imports = {
      env: { emscripten_notify_memory_growth: () => {} },
      wasi_snapshot_preview1: {
        proc_exit: code => { throw new Error('WASM exit ' + code); },
        fd_close: () => 0, fd_write: () => 0, fd_seek: () => 0
      }
    };
    this.e = new WebAssembly.Instance(options.processorOptions.module, imports).exports;
    this.e._initialize?.();
    this.e.mt_init(sampleRate);
    this.l = new Float32Array(this.e.memory.buffer, this.e.mt_left(), 128);
    this.r = new Float32Array(this.e.memory.buffer, this.e.mt_right(), 128);
    this.ticks = 0;
    this.tools = new VocalTools(sampleRate);
    this.denoise = new VocalDenoise(sampleRate);
    this.opto = new VocalOpto(sampleRate);
    this.safety = new VocalSafety(sampleRate);this.autoVocal=new AutoVocal(sampleRate);
    this.extras = {}; this.processing = true;
    this.meter = { pitch: 0, input: 0, output: 0, progress: 0 };
    this.port.onmessage = ({ data }) => {
      if (data.auto) { this.e.mt_auto();this.autoVocal.start(); }
      if (data.resetPeak) this.tools.resetPeak();
      if (data.extras) this.extras = data.extras;
      if (data.params) this.processing = data.params[3] > .5;
      this.tools.set(this.extras, this.processing);
      this.denoise.set(this.extras, this.processing);
      if (data.params){this.opto.set(data.params,this.extras,this.processing);for(let i=0;i<data.params.length;i++)this.e.mt_set(i,this.opto.enabled&&(i===9||i===12)?(i===9?1:0):data.params[i]);}
    };
  }
  process(inputs, outputs) {
    const output = outputs[0], input = inputs[0];
    if (!output?.length) return true;
    this.l.fill(0); this.r.fill(0);
    if (input?.[0]) {
      this.l.set(input[0].subarray(0, 128));
      this.r.set((input[1] || input[0]).subarray(0, 128));
    }
    for (let i=0;i<128;i++){this.tools.observeInput(this.l[i],this.r[i]);this.autoVocal.observe(this.l[i],this.r[i]);}
    for(let i=0;i<128;i++){const gain=this.denoise.process(this.l[i],this.r[i]);this.l[i]*=gain;this.r[i]*=gain;}
    for(let i=0;i<128;i++){const gain=this.opto.process(this.l[i],this.r[i]);this.l[i]*=gain;this.r[i]*=gain;}
    this.e.mt_process();
    for (let i=0;i<128;i++) { this.tools.process(this.l[i],this.r[i],false);this.safety.process(this.tools.left,this.tools.right,this.processing&&!!this.extras.safety);this.tools.measureOutput(this.safety.left,this.safety.right);this.l[i]=this.tools.left;this.r[i]=this.tools.right; }
    output[0].set(this.l);
    if (output[1]) output[1].set(this.r);
    if (++this.ticks % 16 === 0) {
      this.meter.pitch = this.e.mt_pitch(); this.meter.input = this.e.mt_input();
      this.meter.output = this.e.mt_output(); this.meter.progress = this.e.mt_progress();
      Object.assign(this.meter,this.tools.snapshot(this.meter.pitch));
      this.meter.compressionReduction=this.opto.enabled?this.opto.reduction:0;this.meter.safetyReduction=this.extras.safety&&this.processing?-20*Math.log10(this.safety.gain):0;
      this.port.postMessage(this.meter);
    }
    const ready = this.e.mt_auto_ready();
    if (ready) {
      // Infrequent control message, outside the C++ sample loop.
      const proposal=this.autoVocal.result();
      this.port.postMessage({autoResult:ready>0?proposal.params:null,extras:ready>0?proposal.extras:null,autoStats:proposal.stats});
      this.e.mt_auto_ack();
    }
    return true;
  }
}
registerProcessor('matale', MataleProcessor);
