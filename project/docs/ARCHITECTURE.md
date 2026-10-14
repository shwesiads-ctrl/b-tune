# Architecture and design resolutions

Dry input -> linked compressor -> 5-band EQ -> SOLA-style pitch shifter ->
dual-band formant colour -> tempo delay -> comb reverb -> soft saturation ->
smoothed latency-aligned bypass.

YIN and Auto analysis observe the unprocessed mono sum in parallel; processing
or compression does not drive the pitch estimator. Grain launches compare
normalized correlations across a bounded +/-96 sample region. There is no
pitch-epoch tracking, so this implementation is explicitly **not PSOLA**.

YIN anti-alias filtering and integer decimation produce <=12 kHz analysis.
The 512-sample difference window and lag search use fixed arrays; analysis hops
are 128 decimated samples. The first eligible analysis needs 1024 decimated
samples. The lower frequency bound is 70 Hz. Unvoiced audio fades to the
latency-aligned neutral chain.

Audio-thread ownership is single threaded. Delays/reverb vectors allocate in
prepare(); sample processing uses preallocated storage. JUCE parameter handles
are cached; telemetry and proposal values use atomics. Host parameter changes
for Auto are made from the editor timer/message thread, never the audio callback.
When the editor is closed during analysis the proposal waits for editor reopen.
Auto proposals are transient and are not serialized.

The portable C++ engine contains no locks, I/O, host notifications or heap
allocation inside process(). The C++ allocation guard covers operator new/new[];
it is not a timing guarantee for the complete OS, JS VM or DAW. Worklet control
messages can allocate in the browser; no claim of zero JS GC is made.

EQ/formant coefficients interpolate parameter values at a fixed 64-sample
control interval; makeup/output, bypass, effects mixes and compressor reduction
are smoothed. Delay time slews (Doppler modulation can occur on tempo changes).
Cubic retune ramps restart when the target changes; Humanize increases the
transition length. This is not sustained-note/vibrato classification.

WASM reactor imports are minimal, with 32 MiB fixed linear memory, no filesystem
and no memory growth. Native and browser share parameter order and DSP code.
The bridge consumes exactly 128 frames per AudioWorklet quantum. Future browsers
with other quantum sizes need an adapter update. Browser exposes fallback BPM;
native uses host tempo when available.

Official implementation references:
- https://docs.juce.com/master/classjuce_1_1AudioProcessor.html
- https://github.com/juce-framework/JUCE/blob/8.0.10/docs/CMake%20API.md
- https://emscripten.org/docs/compiling/Building-Projects.html
