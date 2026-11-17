param([Parameter(Mandatory=$true)][string]$Launcher)
$ErrorActionPreference='Stop'
$temp=[IO.Path]::GetFullPath([IO.Path]::GetTempPath())
$root=Join-Path $temp ('cdmp_r4_launch_'+[guid]::NewGuid().ToString('N'))
$source=Join-Path $root 'real';$session=Join-Path $root 'session'
function Launch([string]$stage) {
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Launcher -Stage $stage -SessionRoot $session -SourceRoot $source -SessionKind GuestStartup -Fixture
    $script:code=$LASTEXITCODE
}
try {
    New-Item -ItemType Directory -Path (Join-Path $source 'account\slot108') | Out-Null
    [IO.File]::WriteAllText((Join-Path $source 'account\slot108\fixture.save'),'untouched')
    Launch Prepare
    if($code){throw 'Prepare failed'}
    Launch Verify
    if($code){throw 'Verify failed'}
    $prepared=Get-Content -LiteralPath (Join-Path $session 'session.json') -Raw | ConvertFrom-Json
    if(Test-Path -LiteralPath (Join-Path $session 'host-sandbox')){throw 'Host must have no sandbox'}
    [IO.File]::WriteAllText((Join-Path $prepared.guest_sandbox 'account\slot108\fixture.save'),'guest progress')
    Launch Verify
    if($code){throw 'Sandbox writes changed source verification'}
    Launch Run
    if(!$code){throw 'Fixture run must never launch'}
    Launch Prepare
    if(!$code){throw 'Existing session must not be overwritten'}
    $fixtureGame=Join-Path $root 'game';$prior=Join-Path $session 'installation-before'
    New-Item -ItemType Directory -Path $fixtureGame,$prior | Out-Null
    $original=Join-Path $prior 'cdmp.asi';$installed=Join-Path $fixtureGame 'cdmp.asi'
    [IO.File]::WriteAllText($original,'original fixture mod')
    [IO.File]::WriteAllText($installed,'controlled fixture mod')
    $newIni=Join-Path $fixtureGame 'cdmp.guest.ini';[IO.File]::WriteAllText($newIni,'controlled ini')
    $marker=Join-Path $fixtureGame 'cdmp.r4-test-lock';[IO.File]::WriteAllText($marker,'controlled marker')
    $files=@(
        @{name='cdmp.asi';existed=$true;before_sha256=(Get-FileHash -LiteralPath $original).Hash;installed_sha256=(Get-FileHash -LiteralPath $installed).Hash},
        @{name='cdmp.guest.ini';existed=$false;before_sha256='';installed_sha256=(Get-FileHash -LiteralPath $newIni).Hash},
        @{name='cdmp.r4-test-lock';existed=$false;before_sha256='';installed_sha256=(Get-FileHash -LiteralPath $marker).Hash})
    @{version=1;files=$files} | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $session 'deployment.json') -Encoding UTF8
    [IO.File]::WriteAllText((Join-Path $fixtureGame 'cdmp.host.log'),'fixture evidence')
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Launcher -Stage Finish -SessionRoot $session -SourceRoot $source -GameDir $fixtureGame -SessionKind GuestStartup -Fixture
    if($LASTEXITCODE){throw 'Controlled fixture cleanup failed'}
    if((Get-FileHash -LiteralPath $installed).Hash -ne (Get-FileHash -LiteralPath $original).Hash -or
        (Test-Path -LiteralPath $newIni) -or (Test-Path -LiteralPath $marker)){throw 'Installation restoration failed'}
    if(!(Test-Path -LiteralPath (Join-Path $session 'evidence\cdmp.host.log'))){throw 'Evidence not retained'}
    [IO.File]::WriteAllText((Join-Path $source 'account\slot108\fixture.save'),'changed source')
    Launch Verify
    if(!$code){throw 'Real save drift was accepted'}
    $hostSession=Join-Path $root 'host-session'
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Launcher -Stage Prepare -SessionRoot $hostSession -SourceRoot $source -SessionKind HostStartup -Fixture
    if($LASTEXITCODE){throw 'Host preparation failed'}
    if(Test-Path -LiteralPath (Join-Path $hostSession 'host-sandbox')){throw 'Host unexpectedly isolated'}
    [IO.File]::WriteAllText((Join-Path $source 'account\slot108\fixture.save'),'ordinary host autosave')
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Launcher -Stage Verify -SessionRoot $hostSession -SourceRoot $source -SessionKind HostStartup -Fixture
    if($LASTEXITCODE){throw 'Legitimate host autosave rejected'}
    $verified=Get-Content -LiteralPath (Join-Path $hostSession 'verification.json') -Raw | ConvertFrom-Json
    if($verified.real_manifest_unchanged -or !$verified.host_changes_allowed -or !$verified.backup_manifests_verified){throw 'Host save delta was hidden'}
    Write-Output 'R5 launcher: real-save host, verified guest-only copy, strict guest-only manifest, host autosave delta retained, cleanup and no fixture launch.'
} finally {
    $verified=[IO.Path]::GetFullPath($root)
    if(!$verified.StartsWith($temp,[StringComparison]::OrdinalIgnoreCase) -or (Split-Path $verified -Leaf) -notlike 'cdmp_r4_launch_*'){throw 'Unsafe fixture cleanup'}
    if(Test-Path -LiteralPath $verified){Remove-Item -LiteralPath $verified -Recurse -Force}
}
