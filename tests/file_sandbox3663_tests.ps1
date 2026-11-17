param([Parameter(Mandatory=$true)][string]$FixtureExe,[string]$SteamDll='')
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
function ProfileCase([string]$steam=''){
    $case=Join-Path $root 'profile-case';$real=Join-Path $case 'real';$backup=Join-Path $case 'backup'
    $profile=Join-Path $case 'profile';$local=Join-Path $profile 'AppData\Local'
    $sandbox=Join-Path $local 'Pearl Abyss\CD\save'
    New-Item -ItemType Directory -Path $real,(Split-Path $sandbox -Parent) | Out-Null
    [IO.File]::WriteAllText((Join-Path $real 'source.save'),'original',[Text.UTF8Encoding]::new($false))
    Copy-Item -LiteralPath $real -Destination $backup -Recurse
    Copy-Item -LiteralPath $backup -Destination $sandbox -Recurse
    $before=Manifest $real;$manifest=Join-Path $case 'manifest.tsv'
    [IO.File]::WriteAllText($manifest,"CDMP-R4-SAVE-1`n"+$before.Replace('`t',"`t")+"`n",[Text.UTF8Encoding]::new($false))
    $hash=(Get-FileHash -LiteralPath $manifest).Hash
    $mode=if($steam){'steam-profile'}else{'profile'}
    foreach($negative in @($true,$false)){
        $start=New-Object Diagnostics.ProcessStartInfo
        $start.FileName=$FixtureExe;$start.UseShellExecute=$false;$start.CreateNoWindow=$true
        $start.RedirectStandardOutput=$true;$start.RedirectStandardError=$true
        $args=@($real,$backup,$sandbox,$manifest,$hash,$mode)
        if($steam){$args+=@($steam)}
        $start.Arguments=($args | ForEach-Object {'"'+$_+'"'}) -join ' '
        $which=if($negative){Join-Path $case 'wrong-profile'}else{$profile}
        New-Item -ItemType Directory -Force -Path (Join-Path $which 'AppData\Local') | Out-Null
        $start.EnvironmentVariables['USERPROFILE']=$which
        $start.EnvironmentVariables['LOCALAPPDATA']=Join-Path $which 'AppData\Local'
        $child=[Diagnostics.Process]::Start($start)
        $output=$child.StandardOutput.ReadToEnd();$errorText=$child.StandardError.ReadToEnd()
        if(!$child.WaitForExit(15000)){throw 'Owned profile fixture timed out'}
        if($negative){if(!$child.ExitCode){throw 'Wrong native profile resolver was accepted'}}
        elseif($child.ExitCode){throw "Profile fixture failed $($child.ExitCode): $output $errorText"}
        if((Manifest $real) -ne $before -or (Manifest $backup) -ne $before){throw 'Profile guest changed original or backup'}
        Write-Output "Profile case negative=$negative exit=$($child.ExitCode); real SHA256 manifest unchanged=True"
        if(!$negative){Write-Output $output.Trim()}
    }
    if([IO.File]::ReadAllText((Join-Path $sandbox 'profile-renamed.save')) -ne 'isolated'){throw 'Profile mutation did not land in guest copy'}
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
    if($SteamDll){
        # Optional manual startup diagnostic, never part of CTest. Only the
        # owned fixture initializes SteamAPI; no real game executable runs.
        foreach($mode in @('steam-baseline','steam-sandbox')){
            & $FixtureExe $real $backup $sandbox $manifest $hash $mode $SteamDll
            $code=$LASTEXITCODE
            Write-Output "$mode exit $code; real manifest unchanged=$((Manifest $real) -eq $before)"
            if((Manifest $real) -ne $before){throw 'Steam diagnostic changed fixture real saves'}
        }
        ProfileCase $SteamDll
        return
    }
    & $FixtureExe $real $backup $sandbox $manifest $hash premapped
    if($LASTEXITCODE){throw 'Pre-existing protected view did not refuse'}
    & $FixtureExe $real $backup $sandbox $manifest $hash teardown
    if($LASTEXITCODE){throw "Native file hook CRT teardown failed $LASTEXITCODE"}
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
    # A concurrently active real-save host may legitimately change the source
    # after the guest backup was frozen. Guest operations still land only in
    # its verified copy, and ordinary guest-only startup still refuses drift.
    $case=Join-Path $root 'host-drift';$real2=Join-Path $case 'real';$backup2=Join-Path $case 'backup';$sandbox2=Join-Path $case 'sandbox'
    New-Item -ItemType Directory -Path $real2 | Out-Null
    [IO.File]::WriteAllText((Join-Path $real2 'source.save'),'original',[Text.UTF8Encoding]::new($false))
    Copy-Item -LiteralPath $real2 -Destination $backup2 -Recurse
    Copy-Item -LiteralPath $backup2 -Destination $sandbox2 -Recurse
    $manifest2=Join-Path $case 'manifest.tsv'
    [IO.File]::WriteAllText($manifest2,"CDMP-R4-SAVE-1`n"+(Manifest $backup2).Replace('`t',"`t")+"`n",[Text.UTF8Encoding]::new($false))
    $hash2=(Get-FileHash -LiteralPath $manifest2).Hash
    [IO.File]::WriteAllText((Join-Path $real2 'source.save'),'host-new',[Text.UTF8Encoding]::new($false))
    $hostBefore=Manifest $real2
    & $FixtureExe $real2 $backup2 $sandbox2 $manifest2 $hash2 refuse
    if($LASTEXITCODE){throw 'Guest-only source-drift refusal failed'}
    & $FixtureExe $real2 $backup2 $sandbox2 $manifest2 $hash2 host-drift
    if($LASTEXITCODE -or (Manifest $real2) -ne $hostBefore){throw 'Guest changed concurrent host source'}
    if([IO.File]::ReadAllText((Join-Path $sandbox2 'source.save')) -ne 'Griginal'){throw 'Concurrent-host guest writes missed isolation'}
    Write-Output 'R5 concurrent-host fixture: frozen backup verified; guest changes zero source bytes; strict guest-only drift still refused.'
    ProfileCase
    Write-Output 'R4 sandbox SHA256 manifest unchanged; create/read/write/rename/delete/relative/extended/mapped paths exercised; old handles/maps and deferred/cross-boundary/link operations denied.'
} finally {
    $verified=[IO.Path]::GetFullPath($root)
    if(!$verified.StartsWith($temp,[StringComparison]::OrdinalIgnoreCase) -or (Split-Path $verified -Leaf) -notlike 'cdmp_r4_sandbox_*'){throw 'Unsafe cleanup path'}
    if(Test-Path -LiteralPath $verified){Remove-Item -LiteralPath $verified -Recurse -Force}
}
