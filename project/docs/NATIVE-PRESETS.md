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
contains all 59 parameter values: musical target, Auto character/inclusions,
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

## Pitch response styles (0.11.0)

Pitch / Key now separates the existing **Tuning mode** (Natural / Hard Tune)
from **Style** (Neutral / Soft / Tight / Pop / Rap / Ballad). Styles modify
Natural-mode response around the base retune/humanize/correction knobs:

| Style | Response |
| --- | --- |
| Neutral | Base knob values |
| Soft | 1.8× retune, 1.6× humanize, 0.9× correction |
| Tight | 0.55× retune, 0.4× humanize, base correction |
| Pop | 0.8× retune, 1.1× humanize, 0.96× correction |
| Rap | 0.4× retune, 0.15× humanize, base correction |
| Ballad | 2.2× retune, 1.8× humanize, 0.85× correction |

Retune stays within 1–150 ms. Hard Tune preserves its original fast fixed
transition and disables the response Style selector. Auto character and pitch
Style are separate: Auto character controls the overall vocal chain, while
pitch Style shapes its Natural-mode pitch response. Selecting a factory
character resets Style to Neutral; subsequent Auto passes retain your chosen
Style. Existing 0.10.0 preset files load with Neutral as the new-field default.
The original style parameter IDs and two mode values remain unchanged.
