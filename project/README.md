# Matale Tune source — 0.7.0

Original vocal-processing code under the MIT license; dependency licenses are
retained. See LICENSE and THIRD_PARTY.md. The ready-to-run browser package is
in MataleTune-Browser in the GitHub repository; this directory/source ZIP is
for development.

## Build and run the browser

Requires Node.js 22+ and current Chrome/Edge:

    cd Web
    npm ci
    npm run build
    npm run dev

Alternatively, after building, run `python3 Start-Browser.py` from this source
root. The supplied WASM is included; Emscripten is needed only if rebuilding
the portable C++ DSP. Do not open index.html directly.

## Vocal workflow

- Start Mic input or a vocal file; headphones are recommended for monitoring.
- Vocal Realtime Control adjusts input gain, Auto Pitch, correction, retune,
  humanize, output and monitor levels while processing runs.
- Detect Key analyses the loaded file or captures live voiced notes. Apply a
  listed key explicitly; a single note is not enough to determine key.
- Note Editor: Analyse notes → select/drag a note or turn its pitch knob →
  Render edits → Preview edited / Export WAV. Undo/Redo and scale correction
  are available. The original file and duration are preserved.
- The large AUTO button performs a separate three-second setup analysis.

The independent note editor is not Celemony Melodyne. It supports monophonic
mono/stereo clips up to 10 minutes and one-octave pitch edits with fixed
timing. Realtime Auto Pitch is bypassed on editor previews; export contains
note edits only. There is no polyphonic or formant-preservation claim.

စတင်သုံးရန်: Mic input / Vocal file ရွေးပြီး Realtime Control မှာ ချိန်ပါ။
ဖိုင်ထဲက note ပြင်ရန် Note Editor → Analyse → pitch ပြင် → Render → Export WAV။

## Validation

From Web:

    npm run test:dsp
    npm run test:session
    npx playwright install chromium
    npm run test:browser

MATALE_CHROMIUM_PATH selects an installed Chromium; MATALE_SCREENSHOTS retains
screenshots. See docs/PITCH-WORKFLOW.md and docs/BROWSER-FEATURES.md for the
executed checks, signal algorithms and limitations. Synthetic browser tests do
not replace physical-device or real vocal listening checks.

## Native builds and dependencies

The prior native build guide is preserved in docs/NATIVE-BUILD.md. Native
source/scripts remain available, but new browser tools are not yet native
features. VST3 / Standalone / AU releases remain a later task. JUCE 8 modules
retain their AGPLv3/commercial licensing despite MIT on original project code.

## Launcher troubleshooting

For a stale 404 tab, rerun the launcher and use its exact newly opened URL.
It verifies files/readiness and automatically falls back to a free port.
Use `python3 Start-Browser.py --port 0` to request a free port explicitly.
Keep the extracted build folder in place while the server runs.
Launcher regression checks: `python3 Tests/LauncherTests.py`.

### Browser 0.6.0

- Note Editor and Detect Key process vocals up to 10 minutes in 30-second analysis jobs, with progress and cancellation. The full file is retained; rendering accepts mono/stereo and at most 60 million total channel samples to bound memory. A sustained note crossing a job boundary can appear as adjacent notes. Long-file time labels are spaced automatically.
- Playback seeking stays transparent on hover, press and focus so the progress fill remains visible.
- Presets still apply on selection, with an explicit Apply preset button to reapply the selected preset and a visible operation status. Warm and Bright presets include more distinct EQ/dynamics settings. Existing saved presets retain their original compressor mode.
- EQ has a dark interactive response graph with five draggable band nodes and rotary controls for the selected band. The existing frequency ranges, gain and Q remain connected to the shared audio engine.
- The original Opto Leveling Compressor offers Peak Reduction, Gain, Compress/Limit and measured gain reduction. It uses an RMS detector and a two-timescale release; the standard compressor remains selectable. Opto mode bypasses the shared compressor to avoid double processing. This is not a FabFilter or LA-2A product or an exact circuit emulation.

### Browser 0.7.0 — adaptive Clean Vocal Auto

Auto now measures stereo dry input before denoise, compression and EQ, avoiding feedback from an earlier Auto pass. Three seconds of voiced material produces bounded broad EQ adjustments for rumble/mud/presence/air, opto compression and makeup based on RMS/crest/headroom, de-essing/smoothing and restrained 4% reverb / 2% delay. Included processing stages and master processing are enabled; included pitch returns to Natural style and full correction. Excluded groups and manually selected keys/scales are retained. Resume playback before running Auto.

With Auto detect key enabled and Chromatic selected, Auto evaluates the captured melody and applies only a sufficiently strong, unambiguous major/minor candidate. Ambiguous or insufficient melody asks for Key/Scale or a longer Detect Key pass. A short melody cannot guarantee the key of an entire song; badly clipped/noisy input and polyphonic recordings still require preparation.

Opto compression adds a 6 dB soft knee, program-dependent two-stage release, adjustable 20–250 Hz sidechain high-pass and parallel Mix. EQ adds a local bypass, selected-band reset and full EQ reset. All remain real audio controls.

A stereo-linked output safety limiter uses 128 samples of lookahead and a −0.45 dBFS sample-peak ceiling, with an 80 ms release. It preserves stereo ratios and meters measure its actual output. Settings exposes its switch and reduction. Enabled processing adds 128 samples to the shared 2048-sample DSP delay (2176 total; browser/device latency is additional). Master bypass remains dry. This is sample-peak protection, not true-peak mastering. Older presets load with default sidechain/mix/safety values.
