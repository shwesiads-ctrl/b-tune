# Matale Tune — browser preview first

No new VST3, Standalone or AU release is produced in this step. Validate the
browser UI and audio before returning to platform builds.

## Run the ready-built preview

Requires Python 3 and a current desktop Chrome/Edge browser. No npm installation
or DSP rebuild is needed to try the included prebuilt preview.

1. Extract the ZIP completely; do not open index.html directly.
2. Windows: double-click Start-Browser-Windows.bat.
3. macOS: open Start-Browser-macOS.command. Alternatively open Terminal in the
   extracted folder and run `python3 Start-Browser.py`.
4. A browser opens. Choose Vocal file (loops on playback) or Mic input.
5. Wear headphones before enabling microphone Monitor. Monitoring starts muted.
6. Choose key/scale, sing for three seconds and press AUTO. Try each manual page.
7. Stop releases audio resources. Close the terminal or press Ctrl+C to stop
   the preview server. If the port is occupied, run `python3 Start-Browser.py
   --port 0` (Windows: `py -3 Start-Browser.py --port 0`).

This runs only on your computer; audio is not uploaded. The chat environment
has no supported public browser-preview link. The included launcher lets you
review and listen in your own browser.

## What has been verified

- TypeScript checking and Vite production build.
- Existing WASM and AudioWorklet DSP checks and three session regression checks.
- Real headless Chromium: PCM WAV decoding, AudioWorklet processing with the
  supplied WASM, A4 pitch detection, non-silent output telemetry, keyboard
  parameter changes, Auto recommendations with key/scale preserved, all manual
  pages, EQ adjustment and Stop.
- Chromium's synthetic microphone: muted startup, monitor toggle and tracks
  ending after Stop. This does not test a physical microphone or loudspeakers.
- Narrow 390px viewport without horizontal overflow and no uncaught page errors.

Browser screenshots use an automated sine-wave fixture, not a recorded vocal
or a DAW session. Listening quality, real hardware and other browsers still need
user testing. Browser/device latency is additional to the 2048-frame DSP delay.

The underlying DSP, parameter definitions, WASM binary and native processor
remain unchanged. The web presentation uses existing parameters only. Preset
management, voice classification, confidence scores, LUFS, a separate de-esser
and AI claims from the reference are not simulated.

## For development (full source package)

From Web: `npm ci`, `npm run build`, `npm run test:dsp`, `npm run test:session`.
For full browser automation install Chromium using `npx playwright install
chromium` and run `npm run test:browser`. An existing Chromium can be selected
with MATALE_CHROMIUM_PATH; MATALE_SCREENSHOTS can retain screenshots. Native
platform packaging is deferred until browser review is complete.
