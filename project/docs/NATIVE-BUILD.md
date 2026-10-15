# Matale Tune 0.9.1 native DAW preview

## Windows x64 — recommended download

Use the `daw-preview-windows` branch's build-status.json and README in GitHub.
Only a successful Windows build publishes a new `MataleTune-Windows-x64-VST3.zip`.
The download includes its source revision, source/build package, license notices
and DAW installation guide. See [DAW-TESTING.md](DAW-TESTING.md).

The `.github/workflows/native-windows.yml` workflow at repository root builds
with Visual Studio 2022 x64 and JUCE 8.0.10, runs native tests plus pluginval 1.0.4
strictness 5 (GUI behavior is checked separately by the JUCE test), and packages
an actual PE x64 module. Failed builds publish logs/status, not a new binary.

## Windows source build

Install Visual Studio 2022 Desktop development with C++, CMake 3.22+ and Git:

```powershell
./scripts/build-windows.ps1
# Optional existing JUCE checkout:
./scripts/build-windows.ps1 -JuceDir C:/SDK/JUCE
```

Copy the entire `build-win/MataleTune_artefacts/Release/VST3/Matale Tune.vst3`
folder to a VST3 scan folder with the DAW closed. The MSVC runtime is linked
statically. Building from source is not a substitute for host validation.

## macOS / AU

The same compact editor and processor are selected by VST3 and AU. AU is built
only on macOS; a Windows DLL is not an AU. Existing scripts remain available:

```sh
bash scripts/build-macos.sh arm64
# Intel: bash scripts/build-macos.sh x86_64
# Universal: bash scripts/build-macos.sh 'arm64;x86_64'
```

Xcode command-line tools, CMake and Git are required. This Windows-focused
preview does not claim macOS/AU build or auval verification.

## Implemented native profile

- Four panels: Vocal Analysis, Output, Auto Mode and Vocal Smooth. No sidebar,
  file playback or offline note editor. Manual controls opens five native parameter pages.
- Actual stereo dry-input Auto features, bounded adaptive EQ/opto dynamics,
  smoothing/de-essing, usable quiet-floor gating and restrained effects.
- Raw YIN pitch confidence, observed range and estimated register, live output
  spectrum, stereo peak meters and calibrated momentary K-weighted LUFS.
- Optional key inference for Chromatic; ambiguous melody does not overwrite the
  key. Manual key/scale and excluded Auto groups are retained.
- Auto parameter/gesture application on the processor's message-thread timer,
  including with the editor closed. The audio thread never notifies the host.
- Legacy parameter IDs preserved; native controls appended with APVTS state
  restoration, validation and defaults for old projects.
- Mono/stereo, host tempo, 2176-sample latency and aligned native bypass.
- Fixed-memory stereo safety guard; prepare allocates buffers, processing and
  Auto feature/proposal work do not allocate.

The full workbench remains the browser version. A full native Standalone is
planned separately and is not included in this DAW preview.

## Checks

```sh
cmake -S . -B build-tests -DMATALE_BUILD_PLUGIN=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build-tests --parallel 2
ctest --test-dir build-tests --output-on-failure
```

With JUCE enabled, the build also adds `MataleNativeProcessorTests`: actual JUCE
processing, Auto key/application with the editor closed, host notification
threading, state migration/ranges, compact editor bounds, exclusions and silence.
It requires a GUI/message environment (on Linux use an available X display).

`NativeVocalTests` additionally measures stereo safety/bypass, zero realtime
allocations, Auto/key math, mono/stereo LUFS calibration at 44.1/48/96 kHz.
Local Linux VST3 compilation and these tests verify the implementation; actual
Windows packaging/validation and real DAW compatibility are separate outcomes.

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

## Studio contrast and response styles (0.11.0)

EQ cards are labelled Low / Low Mid / Mid / High Mid / High, with Frequency,
Gain and Q controls. Card boundaries, panel background contrast and band accent
colours are strengthened. Pitch / Key offers six Natural-mode response styles;
see NATIVE-PRESETS.md for their real response changes and Hard Tune behaviour.
Legacy 0.10 preset files migrate the new style field to Neutral.

## Per-module presets (0.12.0)

EQ, Compressor, Space and Vocal Cleanup each have five ready-made choices in
the Manual page header. Selecting one changes only that module. Manually
modified settings display Custom; Save/Load retains the combined settings.
See NATIVE-PRESETS.md for the recipes and test each dropdown during playback.
