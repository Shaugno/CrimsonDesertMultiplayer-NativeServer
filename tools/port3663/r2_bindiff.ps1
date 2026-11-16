param([Parameter(Mandatory=$true)][string]$AnalysisRoot,
      [ValidateSet('default','correlation','graph')][string]$Profile='default')
$ErrorActionPreference = 'Stop'
if (Get-Process CrimsonDesert -ErrorAction SilentlyContinue) { throw 'Game must be closed' }
if ((Get-FileHash -LiteralPath 'D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe').Hash -ne '8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8') { throw 'Build drift' }
$output = Join-Path $AnalysisRoot $(if ($Profile -eq 'default') { 'r2' } else { "r2-$Profile" })
$primary = Join-Path $output 'old.BinExport'
$secondary = Join-Path $output 'new.BinExport'
foreach ($file in @($primary,$secondary)) { if (!(Test-Path -LiteralPath $file)) { throw "Missing $file" } }
$destination = Join-Path $output 'bindiff'
New-Item -ItemType Directory -Force -Path $destination | Out-Null
& (Join-Path $AnalysisRoot 'bindiff8\ProgramFiles\BinDiff\bin\bindiff.exe') "--primary=$primary" "--secondary=$secondary" "--output_dir=$destination" --output_format=bin,log "--config=$(Join-Path $AnalysisRoot 'bindiff8\CommonAppData\BinDiff\bindiff.json')" *> (Join-Path $output 'bindiff-run.log')
if ($LASTEXITCODE) { throw "BinDiff exit $LASTEXITCODE" }
$results = @(Get-ChildItem -LiteralPath $destination -Filter '*.BinDiff')
if ($results.Count -ne 1) { throw 'Expected one BinDiff result database' }
$results | Get-FileHash -Algorithm SHA256
