param(
    [Parameter(Mandatory=$true)][ValidateSet('old','new')][string]$Image,
    [Parameter(Mandatory=$true)][string]$AnalysisRoot,
    [Parameter(Mandatory=$true)][string]$Output,
    [Parameter(Mandatory=$true)][string[]]$Rvas
)
$ErrorActionPreference = 'Stop'
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
if (Get-Process CrimsonDesert -ErrorAction SilentlyContinue) { throw 'Game must be closed' }
$target = 'D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe'
if ((Get-FileHash -LiteralPath $target).Hash -ne '8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8') { throw 'Build drift' }
$reference = Join-Path (Split-Path $AnalysisRoot -Parent) 'reference-bins\2976\CrimsonDesert.exe'
if ((Get-FileHash -LiteralPath $reference).Hash -ne '57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7') { throw 'Reference mismatch' }
$env:JAVA_HOME = Join-Path $AnalysisRoot 'jdk-21.0.13+9'
$env:PATH = (Join-Path $env:JAVA_HOME 'bin') + ';' + $env:PATH
$env:GHIDRA_HEADLESS_MAXMEM = '8G'
$project = if ($Image -eq 'old') { 'Port3663' } else { 'PortTarget3663' }
$log = "$Output.run.log"
& (Join-Path $AnalysisRoot 'ghidra_12.1.4_PUBLIC\support\analyzeHeadless.bat') (Join-Path $root 'build\ghidra') $project -process CrimsonDesert.exe -noanalysis -scriptPath $PSScriptRoot -postScript PortFunctionEvidence.java $Output @Rvas *> $log
if ($LASTEXITCODE -or !(Test-Path -LiteralPath $Output)) { throw "Query failed; see $log" }
Get-Item -LiteralPath $Output | Select-Object FullName,Length
