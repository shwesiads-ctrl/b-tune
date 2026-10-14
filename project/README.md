# Matale Tune 0.2 — source / build package

A working development implementation of a monophonic vocal processor, with a
C++17 DSP core, JUCE 8.0.10 VST3/AU adapter and React/AudioWorklet workbench.
**This package is source plus build tooling, not a signed, DAW-certified
Windows/macOS installer.** See docs/VALIDATION.md for actual verification.

## Start here — Apple Silicon Mac

Install Xcode Command Line Tools, CMake 3.22+ and Git. From this folder:

```bash
bash scripts/build-apple-silicon.sh
bash scripts/install-apple-silicon.sh
auval -v aufx Mtn1 Mtla
```

CMake downloads pinned JUCE 8.0.10 on first configure. For an offline checkout:
`JUCE_DIR=/absolute/path/to/JUCE bash scripts/build-apple-silicon.sh`.

The build creates:
- `build-mac-arm64/MataleTune_artefacts/Release/VST3/Matale Tune.vst3`
- `build-mac-arm64/MataleTune_artefacts/Release/AU/Matale Tune.component`

The install script backs up previous copies and installs in your user
`~/Library/Audio/Plug-Ins/` directories. It applies an ad-hoc local signature,
not an Apple Developer ID / notarized distribution signature.

For Intel or Universal: `bash scripts/build-macos.sh x86_64` or
`bash scripts/build-macos.sh 'arm64;x86_64'`. Copy those build outputs manually
into the corresponding user VST3 / Components folders.

## Windows 10/11 x64

Install Visual Studio 2022 with Desktop development with C++, Windows SDK,
CMake 3.22+ and Git. In PowerShell:

```powershell
./scripts/build-windows.ps1
# Offline JUCE checkout:
./scripts/build-windows.ps1 -JuceDir C:/SDK/JUCE
```

The complete VST3 bundle is at
`build-win/MataleTune_artefacts/Release/VST3/Matale Tune.vst3`.
Copy the whole bundle into a VST3 folder scanned by your DAW, commonly
`C:/Program Files/Common Files/VST3/` (that folder can require administrator
rights), then rescan. No AAX, VST2 or 32-bit target is included.

## Browser workbench

If `Web/dist/` is present, it includes the built workbench and DSP WASM.
Serve it over localhost; opening index.html directly with file:// will not work:

```bash
python3 -m http.server 8080 --directory Web/dist
# Open http://localhost:8080
```

For development/rebuilding, activate Emscripten 3.1.64 and run:

```bash
bash scripts/build-web-dsp.sh
cd Web
npm ci
npm run build
npm run test:dsp
npm run dev
```

Use a dry, isolated vocal file or Mic input. Files loop; Stop releases the
microphone and closes the audio context. Microphone monitoring starts muted.
Enable monitoring with headphones. Browser audio/device latency adds to the
engine's delay. Audio is processed locally; there is no upload/backend.

## Tests without JUCE

```bash
cmake -S . -B build-tests -DMATALE_BUILD_PLUGIN=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build-tests --config Release
ctest --test-dir build-tests -C Release --output-on-failure
```

The included GitHub Actions workflow builds native macOS/Windows targets,
runs core tests and builds/tests the WASM workbench when placed in a repository.
The workflow is provided, not proof those hosted jobs have run.

## What's implemented

- YIN CMNDF pitch tracker, 70–1000 Hz, silence/noise gating.
- Five scales, 12 keys, cubic S-curve retuning, Humanize transition length.
- Correlation-aligned overlapping resampling grains (SOLA-style).
- Dual-band formant *colour* filter, linked soft-knee compressor, five
  parametric peaking EQ bands, tempo delay, four-comb algorithmic reverb,
  soft output saturation.
- Three-second Auto analyser and bounded settings for pitch, compression, EQ,
  delay and reverb. Insufficient voiced signal leaves settings unchanged.
- JUCE APVTS automation/state, mono-to-mono/stereo-to-stereo buses, latency
  reporting, smoothed latency-aligned internal/host bypass.
- Native parameter tabs and live input waveform/peak dBFS display.
- React workbench with live output waveform, pitch history, EQ response,
  file/mic input, monitor mute and automatic setup.

## Important implementation boundaries

1. **Not yet a production-quality pitch corrector.** Grain artifacts on
   consonants, transients, vibrato and large shifts need listening refinement.
2. **2048 frames nominal signal delay** (42.67 ms at 48 kHz; 46.44 ms at
   44.1 kHz). Pitch analysis uses additional history. This is not a
   near-zero-latency live monitor solution. Shifted grains introduce local
   time variation around the nominal delay.
3. Formant control is two broad filters; it is **not** LPC/cepstral formant
   preservation or a calibrated semitone formant shift.
4. Auto is a deterministic RMS/crest/voicing/brightness heuristic, not AI,
   loudness mastering or musical-key detection. It preserves key, scale,
   style, formant and power; use your song's known key.
5. Browser and native use **the same C++ engine via WASM**. This intentionally
   replaces separate JS/C++ algorithm copies to avoid drift.
6. Native UI uses a compact tab layout. The browser has the large power ring.
   Meters are peak dBFS, not calibrated analog VU meters.
7. Browser cannot run native CTest, build JUCE, or load VST3/AU. Those actions
   run in your terminal/CI. The workbench provides commands and source access.
8. Tested math/finite-output behavior is not a substitute for DAW validation,
   perceptual evaluation or Windows/macOS binary testing.

## Repository map

- Source/VocalDSP.h — portable DSP, no JUCE dependency
- Source/Parameters.h — shared parameter IDs, ranges and mappings
- Source/PluginProcessor.* — JUCE host adapter
- Source/PluginEditor.* — native editor
- Source/WebBridge.cpp — WASM reactor interface
- Web/src/components/PluginUiWorkbench.tsx — React workbench
- Web/public/matale-worklet.js — AudioWorklet adapter
- Tests/ — automated DSP checks
- scripts/ — build/install helpers
- docs/ — architecture, validation and release checklist

Existing v0.1 parameter IDs and plugin identity are retained. DSP behavior and
formant behavior have changed; back up old sessions and renders before replacing
a plugin used in a session. Saved "auto" remains processing power; the new
"Set Audio On Auto" is a separate analysis action.
