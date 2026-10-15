# Matale Tune product scope

User-approved product direction. The native 0.10.0 processor/editor implements
the compact profile for DAW preview builds. Windows build/validation status is
published separately. The browser remains the full workbench; native Standalone
with the full workbench is still planned.

## VST3 and AU: compact vocal processor

Use the user's four-panel reference, arranged as two columns on desktop:

| Left | Right |
| --- | --- |
| Vocal Analysis | Output |
| Auto Mode | Vocal Smooth |

- **Vocal Analysis:** detected note/frequency, confidence, observed vocal range,
  register estimate, RMS, live spectrum and input level.
- **Output:** dBFS, level meter, momentary LUFS, held sample peak and L/R meters.
- **Auto Mode:** Auto button, analysis progress/status, optional automatic key
  estimation, and inclusion switches for smoothing, dynamics, de-essing, EQ,
  pitch, reverb and delay.
- **Vocal Smooth:** Amount, Harsh Control, Warmth and Presence, with measured
  smoothing/de-esser reduction. Continuous controls remain rotary knobs.

Audio comes from the DAW and follows host transport. Keep the processing needed
by these panels, including Auto's EQ/compression/pitch/effects and output safety;
restricting the interface must not turn them into decorative controls. Retain
host state restoration, automation and the required bypass/latency behavior.
Keep key uncertainty visible in Auto's existing status area.

The user additionally requested Manual controls after testing the compact plugin
in Cubase. Keep the default four panels and expose a **Manual controls** button
with five native pages: Pitch/Key, Compressor, EQ, Delay/Reverb/Output and
Vocal/Denoise. These edit the same automated, saved DSP parameters as Auto.
Use rotary knobs, named selectors and switches. Back to Auto returns to the
compact view without resetting edits. The manual view is not the full workbench.

Do not include file/microphone selection, independent playback/seeking, Note
Editor or offline rendering/export. Native factory characters and Save / Load
`.matalepreset` files are now requested plugin features, available in Auto and
Manual views.

## Standalone: full workbench

Include the complete browser-first workflow: file and microphone input,
playback/seeking, analysis/meters, Auto and musical target controls, Detect Key,
Note Editor with rendering/export, presets/import/export/A–B, denoise,
interactive EQ, opto/standard compressor, de-esser, effects and audio/interface
settings. Continue sharing the actual DSP and state logic where applicable.

## Implementation boundary

Choose the profile from the native wrapper/build target, not an editable user
preference. Validate the compact profile in actual VST3/AU hosts and the full
profile in the standalone application. A browser mock of the compact layout
alone is not native integration or DAW verification.
