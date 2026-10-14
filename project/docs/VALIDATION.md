# Validation — 2026-10-11

This report distinguishes executed checks from supplied but unexecuted build
paths. No Windows/macOS plugin binaries or signed installers are included.

## Executed successfully

| Check | Result |
|---|---|
| C++17 portable DSP compile | GCC 13.3.0, Release |
| CTest core tests | 8/8 pass |
| AddressSanitizer + UndefinedBehaviorSanitizer | 8/8 pass, leak detection disabled |
| Emscripten 3.1.64 standalone WASM compile | Pass, 32 MiB fixed memory |
| WASM DSP tests in Node | Pitch, finite output, Auto, no linear-memory growth pass |
| Actual AudioWorklet JS adapter in a Node harness | Stereo output, telemetry and Auto messages pass |
| Native/WASM waveform parity | Peak error 5.96e-8; RMS error 1.07e-9 on supplied fixture |
| React/TypeScript/Vite production build | Pass |

Core tests cover YIN at 44.1/48/96 kHz (80–880 Hz sine fixtures, <8-cent
tolerance), silence/noise rejection, scale quantization, downward/upward
pitch shifting, compressor attenuation, EQ response, formant colour, effect
tails, Auto acceptance/rejection, finite outputs at 44.1/48/96/192 kHz,
1–4096 frame blocks, prepare/reset, exact neutral/bypass nominal latency,
complete 445 Hz -> 440 Hz correction, and no operator new/new[] calls in the
instrumented engine processing loop.

SOLA sine tests at input 220 Hz measured:
- ratio 0.85: 186.895 Hz (0.97 cents error)
- ratio 1.122462: 246.725 Hz (1.52 cents error)
- ratio 1.25: 274.660 Hz (2.14 cents error)

These sine results do not establish perceptual vocal quality.

## Not executed / environment limits

- JUCE native target compile: Linux configure stopped at missing system
  PkgConfig/development dependencies. Native adapter source remains uncompiled
  here; macOS/Windows builds must be run on their target systems.
- macOS AU/VST3 and Windows VST3 host loading, automation/state round trips,
  DAW validation, Apple signing/notarization and installer verification.
- Browser visual inspection, actual microphone capture, playback and device
  latency. A Chromium installation attempt failed during download. The
  worklet harness is not an end-to-end browser/audio-device test.
- LeakSanitizer could not inspect /proc in this environment, so leak checking
  was disabled for the successful ASan/UBSan run. This does not disable their
  address/undefined behavior checks.
- CI jobs are authored but have not run on GitHub.

## Release gates

1. Build on clean Windows/macOS machines and resolve any platform compiler
   diagnostics before distribution.
2. Run AU/VST3 validators and test mono/stereo at each intended sample rate,
   block size, offline render mode, session restore and bypass condition.
3. Listen to male/female voices, breath/noise, sibilants, vibrato, note changes
   and large pitch shifts; measure glitch/CPU behavior during automation.
4. Review algorithmic latency and provide a genuinely lower-delay mode before
   marketing it as a low-latency stage-monitoring tool.
5. Replace dual-band colour shaping with true envelope estimation/resynthesis
   before claiming formant preservation.
6. Confirm browser playback/microphone permissions, monitor mute, Stop cleanup,
   narrow-screen layout and error handling in supported browsers.
7. Verify dependency licensing and perform distribution signing as needed.

Raw executed-check logs are alongside this file.
