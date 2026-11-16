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
    [IO.File]::WriteAllText((Join-Path $source 'account\slot108\fixture.save'),'changed source')
    Launch Verify
    if(!$code){throw 'Real save drift was accepted'}
    Write-Output 'R4 launcher: separate fresh copies, manifest equality, drift refusal, no fixture launch.'
} finally {
    $verified=[IO.Path]::GetFullPath($root)
    if(!$verified.StartsWith($temp,[StringComparison]::OrdinalIgnoreCase) -or (Split-Path $verified -Leaf) -notlike 'cdmp_r4_launch_*'){throw 'Unsafe fixture cleanup'}
    if(Test-Path -LiteralPath $verified){Remove-Item -LiteralPath $verified -Recurse -Force}
}
