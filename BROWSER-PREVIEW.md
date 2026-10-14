# Matale Tune browser preview — 0.3.0

Download/extract this repository ZIP and open `MataleTune-Browser`.

- Windows: run `Start-Browser-Windows.bat`.
- macOS/Linux: run `python3 Start-Browser.py` in that folder.
- Python 3 is required. No npm installation is needed for the prebuilt preview.
- Choose Vocal file or Mic input. Wear headphones before enabling Monitor.

## New working features

Rotary knobs only (drag, keyboard and double-click reset); preset selection,
Save/export and Import; independent A/B snapshots; real Vocal Smooth, Harsh
control, Warmth, Presence and De-Esser processing; six processing stage switches;
Auto inclusion and per-stage analysis; live measured pitch confidence/range,
register estimate, RMS energy, spectrum/waveform, momentary LUFS, L/R and peak
hold; applied audio settings.

The editable project is now directly available in `project/`. A complete source
ZIP is also included at `MataleTune-Browser/dist/MataleTune_Source.zip` and can
be downloaded through the running app. See `project/docs/BROWSER-FEATURES.md`
for implementation details and executed validation.

New vocal processing currently runs in a browser AudioWorklet extension after
the original portable WASM chain. Native plugin parity and VST3 / Standalone /
AU release builds are deferred until browser review is complete. Browser tests
use synthetic audio; please check real vocal listening and hardware locally.
