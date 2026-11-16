param(
    [Parameter(Mandatory=$true)][ValidateSet('old','new')][string]$Image,
    [Parameter(Mandatory=$true)][string]$AnalysisRoot,
    [ValidateSet('analyze','export','vt')][string]$Mode = 'analyze',
    [ValidateSet('default','correlation','graph','direct','native')][string]$Profile = 'default'
)
$ErrorActionPreference = 'Stop'
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
if (Get-Process CrimsonDesert -ErrorAction SilentlyContinue) { throw 'Game must be closed' }
$target = 'D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe'
$hash = (Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash
if ($hash -ne '8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8') { throw "Build drift: $hash" }
$reference = Join-Path (Split-Path $AnalysisRoot -Parent) 'reference-bins\2976\CrimsonDesert.exe'
if ((Get-FileHash -LiteralPath $reference -Algorithm SHA256).Hash -ne '57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7') { throw 'Reference mismatch' }
$env:JAVA_HOME = Join-Path $AnalysisRoot 'jdk-21.0.13+9'
$env:PATH = (Join-Path $env:JAVA_HOME 'bin') + ';' + $env:PATH
$env:GHIDRA_HEADLESS_MAXMEM = '24G'
if ($Mode -eq 'vt') { $env:GHIDRA_HEADLESS_MAXMEM = '48G' }
$projects = Join-Path $root 'build\r2\projects'
$output = Join-Path $AnalysisRoot $(if ($Profile -eq 'default') { 'r2' } else { "r2-$Profile" })
New-Item -ItemType Directory -Force -Path $projects,$output | Out-Null
$program = if ($Image -eq 'old') { $reference } else { $target }
$labels = Join-Path $PSScriptRoot "r2-seeds-$Image.tsv"
$summary = Join-Path $output "analysis-$Image.json"
$log = Join-Path $output "$Mode-$Image.log"
Write-Output "Starting ${Image}: target SHA256=$hash; whole-image Ghidra analysis; profile=$Profile"
$headless = Join-Path $AnalysisRoot 'ghidra_12.1.4_PUBLIC\support\analyzeHeadless.bat'
$project = if ($Image -eq 'old') { 'R2Match/old' } else { 'R2Target/new' }
$sourceProject = 'R2Match'
$options = @()
if ($Profile -ne 'default') {
    if ($Mode -eq 'analyze') { $env:GHIDRA_HEADLESS_MAXMEM = '16G' }
    if ($Mode -eq 'export') { $env:GHIDRA_HEADLESS_MAXMEM = '32G' }
    $prefix = switch ($Profile) { 'native' { 'R2NativeClean' }; 'direct' { 'R2Direct' }; 'graph' { 'R2Graph' }; default { 'R2Correlation' } }
    $sourceProject = "${prefix}Source"
    $project = if ($Image -eq 'old') { "${prefix}Source/old" } else { "${prefix}Target/new" }
    $options = @('-preScript', 'R2CorrelationOptions.java', (Join-Path $output "options-$Image.txt"), $Profile)
}
if ($Mode -eq 'analyze') {
    & $headless $projects $project -import $program -max-cpu 8 -scriptPath $PSScriptRoot -preScript R2SeedRtti.java $labels @options -postScript R2AnalysisSummary.java $summary -postScript R2Export.java (Join-Path $output "$Image.BinExport") *> $log
} elseif ($Mode -eq 'export') {
    & $headless $projects $project -process CrimsonDesert.exe -noanalysis -scriptPath $PSScriptRoot -postScript R2Export.java (Join-Path $output "$Image.BinExport") *> $log
} else {
    if ($Image -ne 'new') { throw 'VT runs on destination only' }
    & $headless $projects $project -process CrimsonDesert.exe -noanalysis -scriptPath $PSScriptRoot -preScript R2CopySource.java $projects $sourceProject -postScript AutoVersionTrackingScript.java / R2Auto /CrimsonDesert.exe -postScript R2ExportMatches.java /R2Auto (Join-Path $output 'vt-matches.tsv') *> $log
}
$result = $LASTEXITCODE
Get-Content -LiteralPath $log -Tail 18
if ($result) { throw "Headless analysis exit $result" }
if (Select-String -LiteralPath $log -SimpleMatch 'REPORT SCRIPT ERROR' -Quiet) { throw 'Ghidra script failed; outputs cannot be used as completion evidence' }
if ($Mode -eq 'analyze') {
    if (-not (Test-Path -LiteralPath $summary)) { throw 'Missing analysis completion summary; inspect log' }
    Get-Content -LiteralPath $summary
}
if ($Mode -eq 'vt') {
    if (-not (Test-Path -LiteralPath (Join-Path $output 'vt-matches.tsv'))) { throw 'Missing VT results; inspect log' }
} else {
    if (-not (Test-Path -LiteralPath (Join-Path $output "$Image.BinExport"))) { throw 'Missing BinExport; inspect log' }
}
