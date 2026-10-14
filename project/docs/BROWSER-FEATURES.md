# Browser features — version 0.3.0

All changes in this release target the browser workbench. VST3, Standalone and
AU packaging remains deferred. The portable WASM engine and native processor
are unchanged. The new VocalTools JavaScript AudioWorklet extension currently
runs only in the browser; native parity requires a later implementation step.

## Controls and state

- Every continuous control is a rotary knob, implemented with pointer capture
  (mouse/touch drag), arrow keys, Home/End, Shift adjustment and double-click
  reset. No range inputs or slider widgets are rendered. Values are clamped
  and quantized to their actual parameter steps.
- Four built-in presets affect the actual engine and vocal extension.
- Save stores a complete parameter/extension/stage snapshot in browser storage
  and exports a versioned JSON file. Import validates types, version, array
  length, finite values and bounds before changing any audio settings.
- A/B compares two independent in-memory snapshots of engine parameters,
  vocal settings and processing stage switches. Editing or loading a preset
  affects the active slot. Switching slots preserves the previous slot.
- Individual Pitch, Dynamics, EQ, De-Esser, Reverb and Delay switches bypass
  their stage while preserving its knob values. EQ bypass neutralizes the five
  core EQ gains; Vocal Smooth's independent warmth/presence filters remain
  under the Vocal Smooth controls and overall processing power switch.
- Auto includes checkboxes filter which recommendations are applied. Per-stage
  Auto buttons run the real analysis and apply only that stage's proposal.
  Key and scale remain unchanged. Recommendations are signal heuristics,
  not a trained AI model or a guarantee of improved vocal quality.
- Settings select the next input's sample rate and AudioContext latency hint;
  display mode and monitor level update immediately. Actual sample rate and
  DSP latency are shown. Browser/hardware latency is additional.

## Actual added processing

After the shared WASM chain, the worklet adds linked high-band de-essing,
level-dependent harshness suppression centered at 3.3kHz, and warmth/presence
bell filters at 180Hz/3.2kHz. De-Esser frequency, threshold and amount are
available in Dynamics. Smooth amount and gentle/focused harsh control adjust
real filter reduction. The overall power switch bypasses the extension along
with the existing processing path. Tonal boosts use a soft peak guard.

This is a compact browser DSP implementation, not a claim of a mastering-grade
multiband processor. Controls are smoothed; histories are preallocated, but
there is no promise of zero JavaScript GC or OS scheduling interruptions.

## Measurements, not demo numbers

- Input pitch comes from the existing WASM detector. Pitch confidence is
  normalized autocorrelation at that period on raw input, not AI confidence.
- Vocal range is the observed range of consecutive confident voiced readings
  in the current input session. Register is an explicitly labeled heuristic
  based on observed notes after enough readings and at least seven semitones.
  Otherwise it says "More notes needed". It cannot determine a singer's voice
  type from one note.
- Energy is input RMS amplitude as percent of digital full scale.
- The graph shows actual output spectrum or waveform from AnalyserNode.
- Output L/R peaks are measured separately; peak hold can be reset.
- LUFS M is a 400ms stereo, BS.1770 K-weighted momentary reading (no integrated
  gating, no true-peak oversampling). Silence and incomplete windows show —.
- System CPU and total hardware latency are not exposed as invented numbers.

## Executed validation

- Signal tests at 44.1/48/96kHz measure real sibilant attenuation, low tone
  preservation, smooth/harsh reduction, warmth and presence boosts, neutral and
  bypass behavior, finite/stereo output and K-weighted 1kHz calibration.
- Silence, confidence, observed note range, peak reset and independent L/R
  metering have direct tests.
- Real Chromium tests cover file decoding/processing/Auto, synthetic microphone
  capture/cleanup, every manual page, pointer and keyboard knobs/reset, zero
  sliders, preset save/export/import/rejection, A/B state, stage switches,
  applied audio settings and a narrow screen without horizontal overflow.
- Existing WASM, worklet and session cancellation tests remain part of checks.

Browser tests use synthetic fixtures. Real vocal listening, physical audio
hardware, other browsers, and native plugin parity remain to be checked.
