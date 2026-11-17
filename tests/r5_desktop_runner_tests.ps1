param([Parameter(Mandatory=$true)][string]$Runner)
$ErrorActionPreference='Stop'
$r5Test=Start-Process -FilePath $Runner -ArgumentList '--self-test' -WindowStyle Hidden -PassThru -Wait
if($r5Test.ExitCode){throw 'Owned desktop-runner deadline/identity/path/manifest fixtures failed'}
$r5Temp=[IO.Path]::GetFullPath([IO.Path]::GetTempPath()).TrimEnd('\')
$r5Root=Join-Path $r5Temp ('cdmp-r5-runner-'+[guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $r5Root | Out-Null
try{
    $copy=Join-Path $r5Root 'r5_host_desktop.exe';Copy-Item -LiteralPath $Runner -Destination $copy
    $plan=@{version=1;present=$false;role='host';session_root=$r5Root;repo_root=$r5Root;game_image=(Join-Path $r5Root 'CrimsonDesert.exe')}
    $plan | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $r5Root 'r5_host_desktop.plan.json') -Encoding UTF8
    $process=Start-Process -FilePath $copy -WindowStyle Hidden -PassThru -Wait
    $errorPath=Join-Path $r5Root 'r5_host_desktop.error.json'
    $refusal=if([DateTime]::UtcNow -lt [DateTimeOffset]::Parse('2026-11-18T15:00:00Z').UtcDateTime){'R7 game launch forbidden before'}else{'Presence/plan version missing'}
    if($process.ExitCode -ne 1 -or (Get-Content -LiteralPath $errorPath -Raw) -notmatch $refusal){throw 'Time/presence refusal not enforced before any executable access'}
    $process=Start-Process -FilePath $copy -ArgumentList '--launch','arbitrary.exe' -WindowStyle Hidden -PassThru -Wait
    if($process.ExitCode -ne 1 -or (Get-Content -LiteralPath $errorPath -Raw) -notmatch 'No arbitrary command line accepted'){throw 'Arbitrary command-line refusal failed'}
    Write-Output 'R5 desktop runner: owned normal/deadline/identity/path/save mutation fixtures, absent presence and arbitrary command refusal PASS; no game launch'
}finally{
    $resolved=[IO.Path]::GetFullPath($r5Root)
    if(!$resolved.StartsWith($r5Temp+'\',[StringComparison]::OrdinalIgnoreCase) -or (Split-Path $resolved -Leaf) -notlike 'cdmp-r5-runner-*'){throw 'Unsafe fixture cleanup'}
    Remove-Item -LiteralPath $resolved -Recurse -Force
}
