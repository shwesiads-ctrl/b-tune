# Matale Tune 0.9.1 — Windows x64 VST3 DAW preview

This package contains a native Windows plugin, not the browser application.
The full workbench remains in the browser; native Standalone is not included.

## Install / တပ်ဆင်ရန်

1. Close your DAW. Extract the ZIP completely.
2. Copy the **whole `Matale Tune.vst3` folder** into your DAW's VST3 scan folder,
   normally `C:\Program Files\Common Files\VST3\`. This folder may request
   administrator access. Do not copy only the internal module file.
3. Open your DAW and rescan VST3 plugins. Insert **Matale Tune** as an audio
   effect on a dry vocal track. It supports mono-to-mono and stereo-to-stereo.
4. If an older version is installed, back up its complete bundle first, then
   replace it with the DAW closed. Test in a new or copied project.

## AUTO စမ်းရန်

1. Vocal ဆိုနေတဲ့အပိုင်းကို DAW မှာ play လုပ်ပါ။
2. **AUTO** ကို တစ်ချက်နှိပ်ပြီး 3 စက္ကန့် voiced phrase ကို နားထောင်ခိုင်းပါ။
3. Included stages are applied automatically: natural pitch correction, broad
   EQ, opto compression/makeup, smoothing, de-essing, quiet-noise gating when a
   usable noise floor is observed, and restrained reverb/delay.
4. With Chromatic selected, Auto applies a major/minor key only when sufficiently
   clear. Otherwise it retains Chromatic and asks you to retry a more varied
   phrase. A short melody cannot always establish the song's key.
5. Untick an inclusion before Auto to preserve that group's current settings.
   Smooth Amount, Harsh Control, Warmth and Presence remain available as knobs
   or a switch. Additional audio parameters remain exposed to DAW automation.

The default plugin editor contains Vocal Analysis, Output, Auto Mode and Vocal
Smooth. Manual controls opens five parameter pages. File playback and Note
Editor remain outside the plugin.

## DAW checks

- Confirm the correct mono/stereo layout, clean input and audible output.
- Compare before/after with the plugin's On switch; the host should compensate
  the reported **2176-sample latency** (~45.33 ms at 48 kHz). Device/host latency
  is additional. Live monitoring is not near-zero-latency.
- Test AUTO during normal realtime playback, then close the editor and confirm
  processing continues. Auto application uses the host message thread; do not
  use offline rendering as the first analysis pass.
- Save, close and reopen a copied project: processing settings, inclusion
  switches and Smooth controls should return. Auto does not restart on recall.
- Export a short vocal and check timing, peaks, artifacts and tails. Report your
  DAW/version, sample rate, buffer size, mono/stereo and the failing operation.

## Build / validation boundaries

This is an unsigned DAW test build, not a signed installer or a claim of support
for every DAW. It is built on Windows by GitHub Actions and only packaged after
native core tests, JUCE processor/state/UI tests and pluginval strictness 5 pass.
The exact source revision and download SHA-256 accompany the package. Host-specific
real-world listening and project recall still need your DAW test.

Meters are measured: momentary K-weighted LUFS (not integrated loudness), sample
peak (not true peak), pitch periodicity confidence and an estimated register.
Stereo-linked safety limits active output to approximately -0.45 dBFS sample
peak. Native bypass keeps latency alignment. Pitch correction is monophonic;
large shifts/transients can produce grain artifacts. Quiet-noise gating does
not separate noise from a voice while singing.

Original project code is MIT. The binary also includes JUCE and other libraries
under their own licenses; see the included notices and source. This is not a
FabFilter, Teletronix or Universal Audio product or an exact circuit emulation.

## Manual controls (0.9.1)

Click **Manual controls** in the top-right header. Choose Pitch / Key, Compressor,
EQ, Delay / Reverb / Output, or Vocal / Denoise. Knobs update the live processor;
the number below each knob is editable. Double-click a knob to restore its
default. **Back to Auto** returns to the four panels without resetting settings.
All 48 manual controls use the existing host automation/state parameter IDs.
Opto mode uses its program-dependent attack/release and ratio; disable Opto
compressor to edit the standard compressor's ratio, attack and release. Host
tempo takes priority over Fallback tempo when the DAW provides BPM. Auto can
replace included groups on the next analysis; untick a group to retain its
manual settings. Key/scale selections other than Chromatic are retained by Auto.

DAW check: change parameters on each page during playback, automate them, save
and reopen the project, and confirm the values and sound are restored.

The 0.9.1 native interface adds a scalable vector Matale Tune logo and branded
header, refined rotary controls, spectrum shading and consistent switches. The
persistent footer reads “Make by Myominaung” with a “Telegram @kominit” link
to https://t.me/kominit in both Auto and Manual views. No DSP or parameter IDs
changed for this visual update.

## Presets and grouped Manual interface (0.10.0)

The header now has Natural, Clear, Warm, Bright and Hard Tune starting presets
and native Save / Load buttons. Selecting a character applies starting settings;
AUTO then adapts that character to the vocal, respecting the included groups.
Save and load full parameter settings as `.matalepreset` files. See
[NATIVE-PRESETS.md](NATIVE-PRESETS.md) for retained settings and host checks.
Manual uses a left navigation rail, grouped modules, live input feedback and
a five-band EQ response plot computed from the actual filter settings.
