# Reproduce the file-only catalog/join/header pipeline; fail before later stages on any error.
param(
    [Parameter(Mandatory=$true)][string]$Reference,
    [Parameter(Mandatory=$true)][string]$Target,
    [Parameter(Mandatory=$true)][string]$Vt,
    [Parameter(Mandatory=$true)][string]$BinDiff,
    [string]$Python='C:\Users\shaug\source\repos\beamng-earth-s3-review\python312\python.exe'
)
$ErrorActionPreference='Stop'
if (Get-Process CrimsonDesert -ErrorAction SilentlyContinue) { throw 'Game must be closed' }
$root=Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
if ((Get-FileHash -LiteralPath $Reference).Hash -ne '57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7' -or
    (Get-FileHash -LiteralPath $Target).Hash -ne '8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8') {
    throw 'Image identity drift; rematch first'
}
foreach ($path in @($Vt,$BinDiff)) {
    if (!(Test-Path -LiteralPath $path -PathType Leaf)) { throw "Missing standard output: $path" }
}
function RunPython([string]$Script,[string[]]$Arguments=@()) {
    & $Python (Join-Path $PSScriptRoot $Script) @Arguments
    if ($LASTEXITCODE -ne 0) { throw "Catalog stage failed: $Script (exit $LASTEXITCODE)" }
}
$joinArgs=@('--vt',$Vt,'--bindiff',$BinDiff,'--reviewed',(Join-Path $root 'docs\probe-3663\r2-reviewed.json'))
RunPython 'r2_core.py'
RunPython 'r2_rva_checks.py'
RunPython 'r2_wire_review.py'
RunPython 'r2_reviewed.py'
RunPython 'r2_matches.py' $joinArgs
RunPython 'r2_signature_review.py' @('--reference',$Reference,'--target',$Target)
RunPython 'r2_reviewed.py'
RunPython 'r2_matches.py' $joinArgs
RunPython 'r2_layout_header.py'
Write-Output 'Catalog refresh completed. No tier permissions changed; build and runtime validation are separate.'
