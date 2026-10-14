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
