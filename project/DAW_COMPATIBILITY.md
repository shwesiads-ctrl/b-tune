# Intended formats, not certification

| Platform / host | Target | Status in this package |
|---|---|---|
| macOS Logic Pro / GarageBand | AU component | Build source supplied; host test pending |
| macOS VST3 hosts | VST3 arm64 / x86_64 / universal | Build source supplied; host test pending |
| Windows 10/11 x64 VST3 hosts | VST3 x64 | Build source supplied; host test pending |
| Pro Tools | AAX | Explicitly excluded |
| Browser | AudioWorklet + WASM | Separate workbench; does not load native plugins |

Supported buses: mono input/output or stereo input/output with linked pitch and
compression control. Stereo material must represent one singer, not different
notes in each channel. Antiphase stereo can cancel in the detector downmix.
No surround, sidechain, MIDI or polyphonic correction support.

Before a release: perform AU validation, pluginval/VST3 validator, parameter
automation/state restore, sample-rate changes, bypass, rapid editor reopen,
offline render, long playback and listening checks in actual target DAWs.
