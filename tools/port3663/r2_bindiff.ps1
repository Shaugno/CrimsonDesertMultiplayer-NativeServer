param([Parameter(Mandatory=$true)][string]$AnalysisRoot,
      [ValidateSet('default','correlation','graph','direct','native','nativefix')][string]$Profile='default',
      [ValidateSet('default','conservative','structural')][string]$MatcherPreset='default')
$ErrorActionPreference = 'Stop'
if (Get-Process CrimsonDesert -ErrorAction SilentlyContinue) { throw 'Game must be closed' }
if ((Get-FileHash -LiteralPath 'D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe').Hash -ne '8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8') { throw 'Build drift' }
$output = Join-Path $AnalysisRoot $(if ($Profile -eq 'default') { 'r2' } else { "r2-$Profile" })
$primary = Join-Path $output 'old.BinExport'
$secondary = Join-Path $output 'new.BinExport'
foreach ($file in @($primary,$secondary)) { if (!(Test-Path -LiteralPath $file)) { throw "Missing $file" } }
$suffix=if ($MatcherPreset -eq 'default') { 'bindiff' } else { "bindiff-$MatcherPreset" }
$destination = Join-Path $output $suffix
New-Item -ItemType Directory -Force -Path $destination | Out-Null
$config=Join-Path $AnalysisRoot 'bindiff8\CommonAppData\BinDiff\bindiff.json'
if ($MatcherPreset -ne 'default') {
    # Additional whole-image stock profile. Keep the broad run and its files.
    # Omitted passes supply fewer candidates; no confidence is increased.
    $settings=Get-Content -Raw -LiteralPath $config | ConvertFrom-Json
    $names=@('function: name hash matching','function: hash matching','function: prime signature matching')
    $selected=if ($MatcherPreset -eq 'conservative') {
        @($settings.function_matching | Where-Object name -in $names)
    } else { @($settings.function_matching | Where-Object confidence -ge 0.8) }
    if (!$selected.Count -or ($MatcherPreset -eq 'conservative' -and $selected.Count -ne 3) -or
        @($selected | Where-Object confidence -lt 0.8).Count) {
        throw 'Unexpected installed stock BinDiff configuration'
    }
    $settings.function_matching=$selected
    $settings.log.directory=$destination
    $settings.log.to_stderr=$true
    $config=Join-Path $destination "stock-$MatcherPreset-config.json"
    [IO.File]::WriteAllText($config,($settings | ConvertTo-Json -Depth 20),[Text.UTF8Encoding]::new($false))
}
@{preset=$MatcherPreset;whole_image=$true;config=(Get-FileHash -LiteralPath $config).Hash;
  inputs=@(Get-FileHash -LiteralPath $primary,$secondary | Select-Object Path,Hash);
  started_utc=[DateTime]::UtcNow.ToString('o')} |
  ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $destination 'run-receipt.json')
& (Join-Path $AnalysisRoot 'bindiff8\ProgramFiles\BinDiff\bin\bindiff.exe') "--primary=$primary" "--secondary=$secondary" "--output_dir=$destination" --output_format=bin,log "--config=$config" *> (Join-Path $output "$suffix-run.log")
if ($LASTEXITCODE) { throw "BinDiff exit $LASTEXITCODE" }
$results = @(Get-ChildItem -LiteralPath $destination -Filter '*.BinDiff')
if ($results.Count -ne 1) { throw 'Expected one BinDiff result database' }
$results | Get-FileHash -Algorithm SHA256
