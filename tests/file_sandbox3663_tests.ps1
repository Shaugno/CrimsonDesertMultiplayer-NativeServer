param([Parameter(Mandatory=$true)][string]$FixtureExe)
$ErrorActionPreference='Stop'
Import-Module Microsoft.PowerShell.Utility
$temp=[IO.Path]::GetFullPath([IO.Path]::GetTempPath())
$root=[IO.Path]::GetFullPath((Join-Path $temp ('cdmp_r4_sandbox_'+[guid]::NewGuid().ToString('N'))))
if(!$root.StartsWith($temp,[StringComparison]::OrdinalIgnoreCase) -or (Split-Path $root -Leaf) -notlike 'cdmp_r4_sandbox_*'){throw 'Invalid fixture root'}
function Manifest([string]$dir) {
    @(Get-ChildItem -LiteralPath $dir -Recurse -File | Sort-Object FullName | ForEach-Object {
        '{0}`t{1}`t{2}' -f (Get-FileHash -LiteralPath $_.FullName).Hash,$_.Length,$_.FullName.Substring($dir.Length+1)
    }) -join "`n"
}
try {
    $real=Join-Path $root 'real';$backup=Join-Path $root 'backup';$sandbox=Join-Path $root 'sandbox'
    New-Item -ItemType Directory -Path $real | Out-Null
    [IO.File]::WriteAllText((Join-Path $real 'source.save'),'original',[Text.UTF8Encoding]::new($false))
    Copy-Item -LiteralPath $real -Destination $backup -Recurse
    Copy-Item -LiteralPath $backup -Destination $sandbox -Recurse
    $before=Manifest $real
    $manifest=Join-Path $root 'manifest.tsv'
    $body="CDMP-R4-SAVE-1`n"+$before.Replace('`t',"`t")+"`n"
    [IO.File]::WriteAllText($manifest,$body,[Text.UTF8Encoding]::new($false))
    $hash=(Get-FileHash -LiteralPath $manifest).Hash
    & $FixtureExe $real $backup $sandbox $manifest $hash premapped
    if($LASTEXITCODE){throw 'Pre-existing protected view did not refuse'}
    if($env:CDMP_R4_FIXTURE_DEBUGGER) { & $env:CDMP_R4_FIXTURE_DEBUGGER $FixtureExe $real $backup $sandbox $manifest $hash normal }
    else { & $FixtureExe $real $backup $sandbox $manifest $hash normal }
    if($LASTEXITCODE){
        Get-ChildItem -LiteralPath $root -Recurse | Select-Object FullName,Length | Format-Table -AutoSize
        throw "Sandbox worker failed $LASTEXITCODE; real manifest unchanged=$((Manifest $real) -eq $before)"
    }
    if((Manifest $real) -ne $before){throw 'Real fixture directory changed'}
    if([IO.File]::ReadAllText((Join-Path $sandbox 'source.save')) -ne 'Griginal'){throw 'Mapped write missed sandbox'}
    foreach($name in @('renamed.save','relative.save','dot.save','extended.save','newdir\nested.save')) {
        if(!(Test-Path -LiteralPath (Join-Path $sandbox $name))){throw "Missing sandbox effect $name"}
    }
    if(Test-Path -LiteralPath (Join-Path $root 'escape.save')){throw 'Rename escaped sandbox'}
    & $FixtureExe $real $backup $real $manifest $hash refuse
    if($LASTEXITCODE){throw 'Same-root setup did not refuse'}
    & $FixtureExe $real $backup $sandbox $manifest ('0'*64) refuse
    if($LASTEXITCODE){throw 'Bad manifest did not refuse'}
    if((Manifest $real) -ne $before){throw 'Negative fixture changed real directory'}
    Write-Output 'R4 sandbox SHA256 manifest unchanged; create/read/write/rename/delete/relative/extended/mapped paths exercised; old handles/maps and deferred/cross-boundary/link operations denied.'
} finally {
    $verified=[IO.Path]::GetFullPath($root)
    if(!$verified.StartsWith($temp,[StringComparison]::OrdinalIgnoreCase) -or (Split-Path $verified -Leaf) -notlike 'cdmp_r4_sandbox_*'){throw 'Unsafe cleanup path'}
    if(Test-Path -LiteralPath $verified){Remove-Item -LiteralPath $verified -Recurse -Force}
}
