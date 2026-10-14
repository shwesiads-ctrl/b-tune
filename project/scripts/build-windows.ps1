param([string]$JuceDir = "")
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..")
$argsExtra = @()
if ($JuceDir) { $argsExtra += "-DJUCE_DIR=$JuceDir" }
& cmake -S . -B build-win -G "Visual Studio 17 2022" -A x64 @argsExtra
if ($LASTEXITCODE) { throw "Configure failed" }
& cmake --build build-win --config Release --parallel 2
if ($LASTEXITCODE) { throw "Build failed" }
& ctest --test-dir build-win -C Release --output-on-failure
if ($LASTEXITCODE) { throw "Tests failed" }
Write-Host "VST3: build-win/MataleTune_artefacts/Release/VST3/Matale Tune.vst3"
Write-Host "Copy that bundle to your DAW's VST3 scan folder. See README."
