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

## Branded interface 0.9.1

Native header/footer, vector mark, knobs and spectrum presentation were updated
without changing DSP or parameter/state IDs. Minimum interactive size is now
980×700; default is 1200×760. Existing bounds/manual-binding, state and DSP
checks passed on Linux: 10 native suites executed, 10 passed. Auto and Manual
screenshots were inspected for branding, legible values, controls and footer.
Windows run https://github.com/shwesiads-ctrl/b-tune/actions/runs/38617599693
passed for source `7190afb6c91f881c8a9ae4c98af48e07eee9273a`: MSVC x64 build,
10/10 native CTest suites and pluginval 1.0.4 strictness 5. Both Auto and Manual
Windows screenshots were captured; the final Auto screenshot was inspected to
confirm the logo, credits and consistent Segoe UI typography. Pluginval GUI
tests and the separate Steinberg validator remain skipped as documented above.

Archive: 3,693,310 bytes. CRC, matching source/version, PE x64 module and SHA-256
were independently verified. SHA-256:
`2160861ae9295e6433e028abf168ed8becd34663f061aada5aadd4561522c0a5`.
Binary commit: `67d5294076d67f1dfd52a89e9b65ea5ca74147d7`.
The user supplied a screenshot of the earlier version inserted and displaying
live analysis in FL Studio. Listening quality and DAW recall of 0.9.1 remain
user-host checks.

## Presets and grouped interface 0.10.0

Linux native build and all 10 CTest suites passed. New coverage includes:

- All five factory characters and factory selection through the actual GUI.
- Real-file round-trip of every parameter, including key, scale, character,
  output gain, denoise and inclusion settings.
- Invalid schema, non-numeric/out-of-range values, missing and duplicate IDs
  rejected without changing any current parameter; failed save reported.
- Preset changes rejected during active Auto analysis.
- Measured Auto in Hard Tune character produces style 1, 5 ms retune and
  zero humanize while retaining the manually selected musical target.
- Character range limits and exclusion-mask preservation.
- All five reorganized Manual pages captured for local visual review; all
  48 manual bindings, rotary-only controls and resize bounds remain checked.

Windows run https://github.com/shwesiads-ctrl/b-tune/actions/runs/38703172936
completed successfully for source `52bac034d90b7352d5fa614b2d8019848b714198`.
MSVC Windows x64 build, all 10 native suites and pluginval 1.0.4 strictness 5
passed. The actual Windows grouped EQ screenshot was inspected after download.
Pluginval GUI tests and the separate Steinberg validator remain skipped as
previously documented.

The archive was independently checked for CRC, matching source/version, a
PE machine 0x8664 VST3 module and SHA-256. Archive size: 3,713,391 bytes.
SHA-256: `7ad0a231ac08590ac431d21d03ee310fccb2518238edca0f67d2a4ca79dda6a4`.
Binary commit: `da1d18ad0973c53bb4ac9760cc327714d0b3e1f7`.
Native file-dialog interaction and listening/project recall remain DAW checks.

## Studio contrast and styles 0.11.0

New checks cover response-style bounds and Soft/Rap/Ballad behaviour, legacy
Hard Tune timing preservation, round-trip of the new style field and migration
of a 0.10.0 preset without that field. The existing GUI binding test now covers
49 manual controls, including the new Style selector. Linux native build and all 10 CTest suites passed, and the revised EQ and Pitch
screenshots were inspected. Windows run https://github.com/shwesiads-ctrl/b-tune/actions/runs/38705652203
passed for source `d9113a98facb0159f0b32a9bd04c430cb90a657f`: MSVC x64 build,
10/10 native CTest suites and pluginval 1.0.4 strictness 5. The Windows EQ
screenshot was inspected for renamed bands, contrast and card boundaries.
The ZIP CRC, source/version, PE x64 module and SHA-256 were independently
verified. Size: 3,717,858 bytes. SHA-256:
`90d6cadb3ced950734c6ce5ec3cab59e7d055c464869d93e1b1101f23d274378`.
Binary commit: `8c63a4a4c4542c89eeb95ebab0801766efb5517b`.
Pluginval GUI tests and the separate Steinberg validator remain skipped as
previously documented. Listening and DAW-specific recall remain host checks.

## Module presets 0.12.0

New actual-processor checks cover all 20 module recipes, matching detection,
unchanged parameters outside the selected module, valid ranges, invalid-index
and pending-Auto rejection, Custom detection after edits, GUI selection on all
four pages and Save/Load of a combined set of module choices. Linux native
build and all 10 CTest suites passed. EQ and Space screenshots were inspected
for the new module selector. Windows validation is pending for this revision.
