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
    this.meter = { pitch: 0, input: 0, output: 0, progress: 0 };
    this.port.onmessage = ({ data }) => {
      if (data.auto) this.e.mt_auto();
      if (data.params) for (let i = 0; i < data.params.length; i++) this.e.mt_set(i, data.params[i]);
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
    this.e.mt_process();
    output[0].set(this.l);
    if (output[1]) output[1].set(this.r);
    if (++this.ticks % 16 === 0) {
      this.meter.pitch = this.e.mt_pitch(); this.meter.input = this.e.mt_input();
      this.meter.output = this.e.mt_output(); this.meter.progress = this.e.mt_progress();
      this.port.postMessage(this.meter);
    }
    const ready = this.e.mt_auto_ready();
    if (ready) {
      // Infrequent control message, outside the C++ sample loop.
      const ids = [8, 12, 4, 5, 17, 23, 28, 31];
      this.port.postMessage({ autoResult: ready > 0 ? ids.map(i => [i, this.e.mt_auto_value(i)]) : null });
      this.e.mt_auto_ack();
    }
    return true;
  }
}
registerProcessor('matale', MataleProcessor);
