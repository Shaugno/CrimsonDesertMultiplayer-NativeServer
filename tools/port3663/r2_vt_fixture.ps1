# Exercise stock AutoVT and the export lock sequence on an owned, unexecuted PE.
# Synthetic tooling regression only; never feeds the game's R2-MATCHES.
param([Parameter(Mandatory=$true)][string]$AnalysisRoot,
      [ValidatePattern('^[A-Za-z0-9_-]+$')][string]$Run='validated')
$ErrorActionPreference='Stop'
$root=Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$folder=Join-Path $root 'build\r2\vt-fixture'
$inputPe=Join-Path $root 'build\r2\export-fixture\Release\r2_export_fixture.exe'
if (!(Test-Path -LiteralPath $inputPe)) { throw 'Build the owned exporter fixture first' }
New-Item -ItemType Directory -Force -Path $folder | Out-Null
if (Test-Path -LiteralPath (Join-Path $folder "$Run.gpr")) { throw 'Use a new fixture run name' }
$env:JAVA_HOME=Join-Path $AnalysisRoot 'jdk-21.0.13+9'
$env:GHIDRA_HEADLESS_MAXMEM='4G'
$headless=Join-Path $AnalysisRoot 'ghidra_12.1.4_PUBLIC\support\analyzeHeadless.bat'
$sourceLog=Join-Path $folder "$Run-source.log"
& $headless $folder "$Run/old" -import $inputPe *> $sourceLog
if ($LASTEXITCODE -or (Select-String -LiteralPath $sourceLog -SimpleMatch 'REPORT SCRIPT ERROR' -Quiet)) { throw 'Fixture source import failed' }
$targetLog=Join-Path $folder "$Run-target.log"
$tsv=Join-Path $folder "$Run-matches.tsv"
& $headless $folder "$Run/new" -import $inputPe *> $targetLog
if ($LASTEXITCODE -or (Select-String -LiteralPath $targetLog -SimpleMatch 'REPORT SCRIPT ERROR' -Quiet)) { throw 'Fixture target import failed' }
# A newly imported program is not yet a writable project file in post-scripts.
# Save/close first, as in the whole-image pipeline, then run the real task.
$targetLog=Join-Path $folder "$Run-vt.log"
& $headless $folder "$Run/new" -process r2_export_fixture.exe -noanalysis -scriptPath $PSScriptRoot -preScript R2VTOptions.java (Join-Path $folder "$Run-options.txt") -postScript AutoVersionTrackingScript.java / FixtureSession /old/r2_export_fixture.exe -postScript R2ExportMatches.java /FixtureSession $tsv *> $targetLog
if ($LASTEXITCODE -or (Select-String -LiteralPath $targetLog -SimpleMatch 'REPORT SCRIPT ERROR' -Quiet) -or
    !(Select-String -LiteralPath $targetLog -SimpleMatch 'R2 VT export completed' -Quiet)) { throw 'Fixture AutoVT/export failed' }
$accepted=@(Import-Csv -LiteralPath $tsv -Delimiter "`t" | Where-Object { $_.type -eq 'FUNCTION' -and $_.status -eq 'ACCEPTED' })
if (!$accepted.Count) { throw 'Fixture did not export accepted function matches' }
Write-Output "PASS: stock AutoVT/export fixture, $($accepted.Count) accepted functions; tooling regression only"
