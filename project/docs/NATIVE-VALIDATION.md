# Native 0.8.0 validation record

## Current cloud machine: Linux x86_64

- JUCE 8.0.10 native VST3 and the compact editor compiled successfully.
- CTest: 10 executed, 10 passed (core 8 suites, NativeVocal, actual JUCE processor).
- Processor test uses a worker to feed stereo audio; Auto produces a C Major
  proposal, applies on the message thread with no editor open, then a compact
  editor is created and its live spectrum/UI is captured.
- Project state round-trip, legacy native-field defaults, restored-value ranges,
  inclusion/manual-key preservation, silence handling and minimum/default/maximum
  editor bounds are checked.
- Raw-stage tests verify no allocation during processing/Auto, stereo sample
  ceiling, aligned dry bypass, Auto/key and mono/stereo K-weighted calibration
  at 44.1/48/96 kHz.

## Windows x64

Outcome is recorded by CI in `daw-preview-windows/build-status.json`, with the
exact source SHA and run URL. A success means MSVC build, native tests and
pluginval strictness 5 passed and a PE x64 bundle was packaged. It does not mean
that every DAW has been tested. A failed run leaves any earlier ZIP explicitly
identified as old; follow the current status and hash.

## Not yet verified

Real Windows DAW insertion/listening, DAW-specific save/reopen/export, macOS AU
and auval, signed distribution, and a native full-feature Standalone application.
The cloud Linux VST3 is not a Windows binary and is not offered for Windows use.

## Completed Windows build

Run: https://github.com/shwesiads-ctrl/b-tune/actions/runs/38609482052

Source: `7671f7b343c52f57de48ea6600d9f2de48157f7d`.

- MSVC Windows x64 VST3 build: passed.
- CTest on Windows: 10 executed, 10 passed, including actual native processor.
- pluginval 1.0.4 strictness 5: SUCCESS; audio/state/automation/bus checks passed.
- pluginval GUI tests deliberately skipped; actual JUCE editor tests and a
  Windows screenshot cover layout. The separate Steinberg validator was not
  configured and is explicitly reported skipped in the pluginval log.
- ZIP independently fetched through Git, CRC/source SHA checked, and the
  internal module verified as PE machine 0x8664 (Windows x64).
- ZIP bytes: 3,679,975. SHA-256:
  `e76c3562c7fb2dfd3c97f25521cc2ee1603f99c3a623366cc8d57dc5d8aa256d`.
- Binary archive pinned on branch commit
  `8777bb968c233d74285a75c2e04677a225a2b337`.

This updates the Windows outcome above. Actual DAW listening/project recall
and macOS/AU are still unverified.
