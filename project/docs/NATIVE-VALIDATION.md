# Native validation record

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

## Manual controls 0.9.0

Linux build and all 10 native suites passed. The actual JUCE test opens all five
Manual pages and changes each of the 48 controls, checking the processor value
after each change. It checks bounds at 980×620, 1200×720 and 1800×1000, verifies
rotary sliders, and returns to the compact view. Manual EQ preview is captured.
Windows run https://github.com/shwesiads-ctrl/b-tune/actions/runs/38614002580
completed successfully for source `e50b67fbbb1a11b0381e0b18e77fad94067a1d33`.
MSVC x64 build, all 10 native CTest suites and pluginval 1.0.4 strictness 5
passed. Both native UI screenshots were captured on Windows. Pluginval GUI
tests and the separate Steinberg validator remain skipped as in the earlier run.
The published archive was independently checked for CRC, matching source SHA,
version 0.9.0 and a PE machine 0x8664 VST3 module. Archive size: 3,688,397 bytes.
SHA-256: `4e9bbf79c3d119580f7d02e1f3178d67a8e74a6685ee13f7dd92dd5e7edb430d`.
Binary commit: `56c669cf0bc3919926de2cb592e45300dc45deba`.

The user supplied a Cubase screenshot showing the earlier 0.8.0 VST3 inserted
and reporting live pitch/meters with Auto applied. Listening quality and project
recall were not established by that screenshot; 0.9.0 still needs DAW testing.
