#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
base="build-mac-arm64/MataleTune_artefacts/Release"
for pair in "VST3:VST3:vst3" "AU:Components:component"; do
  IFS=: read -r format folder extension <<< "$pair"
  src="$base/$format/Matale Tune.$extension"
  [[ -d "$src" ]] || { echo "Build missing: $src"; exit 1; }
  dest="$HOME/Library/Audio/Plug-Ins/$folder/Matale Tune.$extension"
  mkdir -p "$(dirname "$dest")"
  if [[ -e "$dest" ]]; then
    backup="$dest.backup-$(date +%Y%m%d-%H%M%S)"
    mv "$dest" "$backup"
    echo "Previous plugin backed up: $backup"
  fi
  ditto "$src" "$dest"
  codesign --force --deep --sign - "$dest"
done
echo "Installed to your user Audio/Plug-Ins folder. Restart your DAW."
echo "Run: auval -v aufx Mtn1 Mtla"
