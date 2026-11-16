param([Parameter(Mandatory=$true)][string]$Launcher)
$ErrorActionPreference='Stop'
$temp=[IO.Path]::GetFullPath([IO.Path]::GetTempPath())
$root=Join-Path $temp ('cdmp_r4_launch_'+[guid]::NewGuid().ToString('N'))
$source=Join-Path $root 'real';$session=Join-Path $root 'session'
function Launch([string]$stage) {
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Launcher -Stage $stage -SessionRoot $session -SourceRoot $source -Fixture
    $script:code=$LASTEXITCODE
}
try {
    New-Item -ItemType Directory -Path (Join-Path $source 'account\slot108') | Out-Null
    [IO.File]::WriteAllText((Join-Path $source 'account\slot108\fixture.save'),'untouched')
    Launch Prepare
    if($code){throw 'Prepare failed'}
    Launch Verify
    if($code){throw 'Verify failed'}
    [IO.File]::WriteAllText((Join-Path $session 'guest-sandbox\account\slot108\fixture.save'),'guest progress')
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
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Launcher -Stage Finish -SessionRoot $session -SourceRoot $source -GameDir $fixtureGame -Fixture
    if($LASTEXITCODE){throw 'Controlled fixture cleanup failed'}
    if((Get-FileHash -LiteralPath $installed).Hash -ne (Get-FileHash -LiteralPath $original).Hash -or
        (Test-Path -LiteralPath $newIni) -or (Test-Path -LiteralPath $marker)){throw 'Installation restoration failed'}
    if(!(Test-Path -LiteralPath (Join-Path $session 'evidence\cdmp.host.log'))){throw 'Evidence not retained'}
    [IO.File]::WriteAllText((Join-Path $source 'account\slot108\fixture.save'),'changed source')
    Launch Verify
    if(!$code){throw 'Real save drift was accepted'}
    Write-Output 'R4 launcher: separate fresh copies, manifest equality, drift refusal, no fixture launch.'
} finally {
    $verified=[IO.Path]::GetFullPath($root)
    if(!$verified.StartsWith($temp,[StringComparison]::OrdinalIgnoreCase) -or (Split-Path $verified -Leaf) -notlike 'cdmp_r4_launch_*'){throw 'Unsafe fixture cleanup'}
    if(Test-Path -LiteralPath $verified){Remove-Item -LiteralPath $verified -Recurse -Force}
}
