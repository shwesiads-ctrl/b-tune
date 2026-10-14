# Matale Tune browser preview 0.4.1

Extract the repository ZIP and open `MataleTune-Browser`:

- Windows: double-click `Start-Browser-Windows.bat`.
- macOS/Linux: run `python3 Start-Browser.py` in that folder.
- Python 3 and current desktop Chrome/Edge are required. No npm setup is needed
  to use the included ready-built preview. Do not open index.html directly.

Choose Vocal file (looping playback) or Mic input. Wear headphones before
turning on microphone Monitor, which starts muted. Choose key and scale, then
sing for three seconds and press AUTO. Stop releases the input; Ctrl+C in the
launcher terminal stops its local web server. Audio stays on your computer.

Knobs: drag up/down, arrow keys, Home/End, Shift-drag for fine adjustment,
Shift+arrows for coarse adjustment, double-click to reset. There are no sliders.

Version 0.4 adds live Input Gain / Auto Pitch / Correction / Retune / Humanize /
Output / Monitor controls, Detect Key with explicitly applied candidates, and
an independent Vocal Note Editor (Analyse → edit → Render → Preview/Export WAV).
The editor supports monophonic mono/stereo clips up to 120 seconds and shifts
up to one octave. It is not Celemony Melodyne. See docs/PITCH-WORKFLOW.md.

Other working features: preset selection and Save/Import JSON, A/B comparison,
Vocal Smooth, Harsh control, Warmth, Presence, De-Esser, stage bypass and
per-stage Auto, measured pitch confidence/range/register estimate/RMS energy,
actual spectrum/waveform, momentary LUFS and L/R peaks, audio settings.

Editable source is in `project/` in the GitHub repository. The running preview
also includes a source ZIP download. See `docs/BROWSER-FEATURES.md` in the source
for implementation, validation and limits. New vocal processing currently runs
in the browser AudioWorklet; native plugin parity is a future step.

For source development, from `project/Web` (or `Web` in the source ZIP) run:

    npm ci
    npm run build
    npm run test:dsp
    npm run test:session
    npx playwright install chromium
    npm run test:browser

MATALE_CHROMIUM_PATH selects an installed Chromium for tests;
MATALE_SCREENSHOTS selects a persistent screenshot folder. The chat has no
supported public preview URL. This launcher opens the browser on your machine.

VST3 / Standalone / AU release builds remain deferred until browser review.

If an old tab shows 404, rerun this launcher and use the new URL it opens.
It now verifies files/readiness and chooses another port if one is occupied.
Do not move or delete the extracted folder while it runs. To request a new
port manually, use `py -3 Start-Browser.py --port 0` on Windows, or
`python3 Start-Browser.py --port 0` on macOS/Linux.

### Browser 0.4.2

Vocal playback now shows elapsed / total time with a live progress bar, Pause / Resume, Restart and ±10-second buttons (no sliders). Opening Note Editor automatically analyses the loaded vocal and scrolls to the piano roll. Status and errors appear above the notes. Preview edited renders the current edits automatically before playback; Export WAV becomes available after rendering. Mono/stereo note editing still supports clips up to 120 seconds; use a clean solo vocal.

Browser 0.4.3 fixes a global 130px CSS cap on the vocal playback progress bar. The timeline now fills its panel and uses a consistent green / blue fill in Chromium and Firefox.

Browser 0.4.4 adds click-to-seek on the full playback timeline. Tab to the timeline and use Left/Right for 5-second jumps, Home for the beginning and End for the end. Seeking preserves the paused/playing state. Effect controls remain rotary knobs.

### Browser 0.5.0 — compact tabs and Denoise

Only Home shows the full analysis dashboard. Pitch shows its musical target and realtime controls; Dynamics, EQ and Effects show their own controls. Settings and Note Editor hide unrelated dashboard panels. The header, navigation and playback transport are more compact.

The Denoise tab provides rotary Amount and Threshold controls for a stereo-linked downward expander before the shared pitch/effects engine. It attenuates quiet background noise between phrases, with a 2ms opening envelope and 120ms release; it does not separate noise underneath a singing voice. Amount 0 and master bypass preserve the dry input. Start low and lower the threshold if soft words are cut. Denoise defaults to bypass; older presets remain compatible and new presets / A–B snapshots retain these controls.
