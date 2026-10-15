# Third-party licenses and notices

The repository's MIT license applies to original Matale Tune project code.
It does not replace licenses on dependencies or third-party tools.

- React 19.0.0, React DOM 19.0.0 and Scheduler: MIT, copyright Meta Platforms,
  Inc. and affiliates. Full license text is included with the browser build in
  `THIRD-PARTY-NOTICES.txt` and source under `project/Web/public/`.
- Vite, TypeScript and Playwright are development dependencies with their own
  MIT/Apache-2.0 licenses and transitive notices. Exact dependencies are pinned
  in `project/Web/package-lock.json`.
- JUCE 8.0.10 is fetched only for native builds and is not included in the
  browser bundle. JUCE modules are dual-licensed AGPLv3/commercial. Using MIT
  for original project code does not waive those obligations.
  https://github.com/juce-framework/JUCE/blob/8.0.10/LICENSE.md
- The supplied portable WASM engine was built with Emscripten 3.1.64. Emscripten
  and included C/C++ runtime components retain their upstream MIT/University of
  Illinois, Apache/LLVM-exception and other applicable notices:
  https://github.com/emscripten-core/emscripten/blob/3.1.64/LICENSE
  https://github.com/llvm/llvm-project/blob/main/libcxx/LICENSE.TXT
- Celemony Melodyne is not a dependency or included product. The note editor,
  profile key estimation and granular renderer here are original code.
- Reference design screenshots supplied in chat are not distributed as
  repository source assets. Included preview images render this project's UI
  with synthetic test signals, not third-party music or vocal recordings.
