# Matale Tune browser preview — 0.4.0

Download/extract this repository ZIP and open `MataleTune-Browser`.

- Windows: run `Start-Browser-Windows.bat`.
- macOS/Linux: run `python3 Start-Browser.py` in that folder.
- Python 3 is required. The prebuilt preview needs no npm installation.
- Choose Vocal file or Mic input; wear headphones before enabling Monitor.

This release adds a persistent realtime vocal panel, Auto Pitch controls,
loaded-file/live Detect Key, and an independent Vocal Note Editor with actual
pitch rendering and WAV export. It also fills wide browser windows, improves
labels and supports large interface sizing. Rotary controls, presets, A/B,
Vocal Smooth, De-Esser and the other processing tools remain available.

Read the root README for full instructions, the distinction between Auto Pitch
and setup AUTO, and the limits of file note editing / key estimation. This is
original note-editor code, not Celemony Melodyne.

Original project code is MIT-licensed; third-party dependency licenses remain
applicable. Native plugin release work and browser/native feature parity remain
separate. Editable source is in `project/`; browser source downloads also include
the new modules, README/license and tests. Physical vocal/audio-device listening
still needs testing on your computer.
