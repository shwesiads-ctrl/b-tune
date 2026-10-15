# Native presets and Auto characters (0.10.0)

The header provides five ready-made vocal characters and Save / Load in both
Auto and Manual views. All continuous controls remain rotary knobs.

| Character | Starting sound and Auto direction |
| --- | --- |
| Natural | Natural pitch response, balanced tone and restrained ambience |
| Clear | Less low-mid buildup, more presence and additional smoothing/de-essing |
| Warm | More vocal body, softer top and a slower natural pitch response |
| Bright | Added presence/air with stronger sibilance control |
| Hard Tune | Hard Tune style, 5 ms retune, zero humanize, extra ambience |

Selecting a character immediately loads its starting settings. Your Key,
Scale, Output gain, Fallback tempo and Auto inclusion switches are retained.
Play a dry vocal and press AUTO to adapt that character to measured vocal
energy, spectrum and pitch. Auto keeps its voiced-input/key-confidence gates,
manual musical target and exclusion switches. Exclusions govern the Auto pass;
selecting a factory character deliberately loads a new starting sound.
Automating the Auto character parameter changes the direction of the next Auto
pass; it does not trigger a new analysis by itself.

Save opens a native file dialog for a `.matalepreset` file. The versioned XML
contains all 58 parameter values: musical target, Auto character/inclusions,
pitch, EQ, compression, cleanup, effects, output and safety. Load checks the
format, version, completeness, unique IDs, finite values, ranges and steps
before applying anything, with host parameter notifications. A rejected file
leaves the current sound intact. Writes use a temporary sibling file before
replacing the destination. Choose the existing `.matalepreset` filename in the
dialog when intentionally overwriting a saved preset.

Changing character or loading is blocked while Auto is pending/listening, so a
late Auto result cannot replace a newly loaded preset. Save captures current
settings. Preset files are separate from browser JSON presets. The full native
Standalone application remains planned.

## DAW check

1. Try all five characters while a vocal plays, then press AUTO on a voiced
   phrase and compare the result.
2. Set a manual Key/Scale and disable an Auto group; check that AUTO retains
   those choices for every character.
3. Adjust controls on several Manual pages. Save a preset, change character,
   Load the saved file and confirm values and sound return.
4. Try an invalid file and confirm an error appears without changing the sound.
5. Save/reopen a DAW project and check character and parameters are restored.

The automated native test uses real files for full parameter round-trip,
invalid root/value/range/missing/duplicate rejection and failed-save reporting.
It tests factory selection through the GUI, measured Hard Tune Auto, exclusion
preservation and rotary-control bounds across all five Manual pages. Native
file dialogs and DAW listening/recall still require the host check above.
