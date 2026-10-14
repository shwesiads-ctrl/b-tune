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
