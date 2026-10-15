# Pitch / key / note editing — 0.4.0

## Implemented

- A persistent Vocal Realtime Control panel sends changes to the running graph:
  a smoothed AudioNode input gain, core Auto Pitch switch/correction/retune/
  humanize/output parameters, and monitor gain. The large three-second AUTO
  action remains a separate setup-analysis command.
- Detect Key analyses the unprocessed loaded vocal in a Web Worker, or gathers
  confident live pitch readings for up to 10 seconds from microphone input.
  Major/minor profile correlations produce up to three candidates. Applying a
  candidate explicitly changes key and scale. One-note/insufficient input does
  not invent a key; uncertain profiles are marked ambiguous.
- The original Vocal Note Editor uses anti-aliased decimated YIN analysis,
  note segmentation, a piano-roll selection/vertical pitch editor, a target
  pitch knob, scale correction, Undo/Redo and an offline granular renderer.
  Rendering retains the original audio and clip duration. The edited WAV can
  be previewed or exported as 16-bit PCM. Editor jobs run in a worker and can
  be cancelled. Note state survives navigation and stopping playback.
- Editor preview bypasses core realtime pitch correction so deliberate edited
  pitches outside the current scale are not immediately corrected again.
  Other vocal effects remain available during playback; exported WAV contains
  note edits only. Input gain is an operational live setting, not part of A/B
  effect presets.
- Layout now fills wide browser windows instead of stopping at a fixed content
  width. Settings offer 100%, 115% and 130% interface sizes; labels distinguish
  realtime Auto Pitch, setup Auto, sample peak and momentary LUFS.

## Limits

This is an independent monophonic editor, not Celemony Melodyne or an
integration with that commercial product. No Celemony code or binary is used.
Supported files are mono/stereo solo-vocal clips up to 120 seconds. Pitch shifts
are limited to one octave, with fixed timing; granular modulation/artifacts
can occur. There is no polyphonic separation, pitch-epoch resynthesis, detailed
vibrato editing, timing editing or formant-preservation claim.

Melody alone cannot always distinguish a key, especially relative major/minor.
Key profile score is a correlation, not probability. Live pitch detection still
requires periodic voiced audio in the engine's approximate 70–1000Hz range.
Monitor latency includes the existing 2048-frame core delay plus browser/device
latency. No low-latency performance guarantee is made.

## Executed checks

- New signal tests detect stable notes and render real upward/downward edits at
  44.1/48/96kHz, check pitch within 30 cents on sine fixtures, preserve length
  and original samples, validate WAV headers, reject excessive edits, and
  exercise C-major key estimation and insufficient-evidence/silence behavior.
- WASM output tests measure 445Hz input corrected toward A4, then restore the
  original pitch by changing the running correction parameter to zero.
- Chromium checks live input-gain attenuation/recovery, Auto Pitch switching,
  loaded-file key detection and Apply, note analysis, knob and pointer edits,
  Undo/Redo, rendering, downloaded WAV pitch/duration, editor preview bypass,
  wide-screen coverage, interface scaling and narrow-screen overflow.
- Existing browser, session cancellation, vocal effects, WASM and worklet
  checks remain in the validation commands.

Fixtures are synthetic. Real vocal listening, physical devices, other browsers
and native plugin parity/host validation remain separate checks. No VST3,
Standalone or AU release is built in this browser feature update.

## Licensing

Original project code is MIT-licensed. Dependency licenses are retained; in
particular JUCE native modules remain AGPLv3/commercial licensed. See LICENSE,
THIRD_PARTY.md and the browser-distributed notices.
