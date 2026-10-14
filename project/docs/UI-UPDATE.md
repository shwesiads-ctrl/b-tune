# Native DAW UI update

This records the earlier native UI stage. The current browser-first stage is
described in `../BROWSER-FIRST.md`; the web UI was subsequently restyled.

The supplied reference guides the light blue/white palette, rounded cards,
sidebar navigation, large Auto action and rotary quick controls. This update
changes only `Source/PluginEditor.h` and `Source/PluginEditor.cpp` in the native
plugin. It builds on the previously corrected package.

The Home page contains live pitch detection, input waveform, input/output peak
meters, the existing three-second Auto analyser, processing bypass and six
quick controls. Pitch, Dynamics, EQ and Effects pages expose all the original
parameters through the original JUCE parameter attachments. Host automation,
parameter IDs, saved state and audio processing use the existing processor.

The reference contains features this processor does not implement (AI
confidence, voice classification, LUFS, dedicated de-esser, preset management,
A/B and CPU metering). They are intentionally not presented as working controls
or fabricated readings. The displayed latency is the real 2048-frame DSP
latency, with milliseconds calculated from the host sample rate. It does not
claim to measure total hardware/DAW latency.

For an existing source checkout, replace the two PluginEditor files and rebuild
using the existing macOS or Windows scripts in `scripts/`. Existing binaries
must be rebuilt for the new UI to appear. The archive is source, not a signed
Windows/macOS installer. Keep your current working plugin binary until you
have tested the rebuilt version in your DAW.

The UI supports 1180 x 820 through 1800 x 1200 pixels and starts at 1440 x 940.
The minimum size gives all 15 EQ controls room without clipping.

## Validation for this update

- JUCE 8.0.10 native shared-code and Linux VST3 targets compile successfully.
- All eight CTest DSP groups pass.
- A JUCE UI probe renders Home and EQ snapshots under Xvfb using a synthetic
  C4 input processed by the existing engine. These are native UI screenshots,
  not mockup images or screenshots of a Windows/macOS DAW.
- Visible component bounds are checked on all five pages at minimum, default
  and maximum sizes. The quick pitch knob updates the existing parameter and
  the Auto button sets the existing analyser request flag.
- DSP, processor, parameter definitions and WebBridge are byte-for-byte equal
  to the previous corrected package. Web UI source is unchanged.
- Windows/macOS compilation and host loading of this new UI remain untested.
  Prior success of the original plugin in a DAW is not a test of this update.

The full package includes regenerated embedded source downloads. The optional
UI-only patch contains the two editor files and this document; it does not
replace your processor or DSP files.
