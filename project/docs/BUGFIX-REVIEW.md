# Bug-fix review — 2026-10-14

Changes made to the uploaded source/build package:

- Fixed Stop/start cancellation: a restart waiting for the previous AudioContext
  to close can no longer open a new context after Stop. Context ownership is
  detached before asynchronous cleanup, and cancelled startup checks occur
  before acquiring sources. Late microphone permissions release their tracks.
- Old startup cleanup and telemetry cannot overwrite a newer session. Stop
  disconnects the graph, clears pitch history and removes its message callback.
- AudioWorklet processor errors now stop the session, release microphone tracks
  and show a retry message instead of leaving a silently active session.
- EQ response redraws when the audio session starts/stops, using its sample rate.
- Raised the native editor minimum height from 660 to 720 so all five rows of
  EQ controls fit inside the tab content area.
- Added three component regression checks and wired them into the web CI job.
  Rebuilt the web distribution and embedded downloadable source archive.

Validation in this Linux environment:

- All eight portable C++ DSP test groups pass (GCC, C++17, optimized build).
- All eight groups also pass with AddressSanitizer and UndefinedBehaviorSanitizer.
- Existing WASM and AudioWorklet tests pass.
- Three new component tests pass: late microphone permission cleanup, Stop during
  a pending restart, and processor failure cleanup/error reporting.
- The restart regression fails against the original component (it creates a
  second AudioContext after Stop) and passes against the corrected component.
- TypeScript checking and Vite production build pass.

The component tests mock browser audio APIs; they do not establish actual audio
hardware behavior. Native JUCE plugin compilation, Windows/macOS host loading,
visual resizing in a DAW, and real microphone/playback checks were not performed.
The native minimum-height fix was checked by inspecting the control bounds.
DSP source and WASM binary were not changed. The original validation report
records historical results; this file describes this review only.

Run checks from the package root:

    bash scripts/test-core.sh  # Requires CMake >= 3.22 and a C++17 compiler
    cd Web
    npm ci
    npm run test:dsp
    npm run test:session
    npm run build
