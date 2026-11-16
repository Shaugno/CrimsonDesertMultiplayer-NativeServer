# R4: preparation and manifest verification are game-free. Run is separately
# gated by presence and a compiled C/D receipt; no legacy launcher behavior changes.
param(
    [Parameter(Mandatory=$true)][ValidateSet('Prepare','Run','Verify','Finish')][string]$Stage,
    [Parameter(Mandatory=$true)][string]$SessionRoot,
    [string]$SourceRoot = (Join-Path $env:LOCALAPPDATA 'Pearl Abyss\CD\save'),
    [string]$GameDir = 'D:\SteamLibrary\steamapps\common\Crimson Desert\bin64',
    [string]$ArmingReceipt = '',
    [ValidateSet('host','guest')][string]$LaunchRole='host',
    [int]$Port=39766,
    [switch]$Present,
    [switch]$Fixture
)
$ErrorActionPreference='Stop'
Import-Module Microsoft.PowerShell.Utility
if (!('CdmpR4Files' -as [type])) {
    Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;
public static class CdmpR4Files {
    [StructLayout(LayoutKind.Sequential)] struct Info {
        public uint attrs, creationLow, creationHigh, accessLow, accessHigh, writeLow, writeHigh;
        public uint volume, sizeHigh, sizeLow, links, indexHigh, indexLow;
    }
    [DllImport("kernel32.dll",CharSet=CharSet.Unicode,SetLastError=true)]
    static extern SafeFileHandle CreateFileW(string p,uint access,uint share,IntPtr sa,uint mode,uint flags,IntPtr template);
    [DllImport("kernel32.dll",SetLastError=true)]
    static extern bool GetFileInformationByHandle(SafeFileHandle h,out Info i);
    public static string Identity(string path) {
        using(var h=CreateFileW(path,0x80,7,IntPtr.Zero,3,0x02200000,IntPtr.Zero)) {
            Info i;
            if(h.IsInvalid || !GetFileInformationByHandle(h,out i) || i.links!=1 || (i.attrs&0x400)!=0)
                throw new Exception("Unverifiable or linked save entry: "+path);
            return i.volume.ToString("X8")+i.indexHigh.ToString("X8")+i.indexLow.ToString("X8");
        }
    }
}
'@
}
function Full([string]$path) { [IO.Path]::GetFullPath($path).TrimEnd('\') }
function Under([string]$path,[string]$parent) {
    $path.Equals($parent,[StringComparison]::OrdinalIgnoreCase) -or $path.StartsWith($parent+'\',[StringComparison]::OrdinalIgnoreCase)
}
$SessionRoot=Full $SessionRoot
$SourceRoot=Full $SourceRoot
if ((Under $SessionRoot $SourceRoot) -or (Under $SourceRoot $SessionRoot)) { throw 'Session and source must be disjoint' }
foreach ($cloud in @($env:OneDrive,$env:OneDriveConsumer,$env:OneDriveCommercial)) {
    if ($cloud -and (Under $SessionRoot (Full $cloud))) { throw 'Backups/sandboxes must be outside OneDrive' }
}
# Fixture mode cannot launch and cannot select the real user save directory.
if ($Fixture) {
    $temp=Full ([IO.Path]::GetTempPath())
    if ($Stage -eq 'Run' -or !(Under $SessionRoot $temp) -or !(Under $SourceRoot $temp)) { throw 'Fixture paths must be temporary; launch forbidden' }
    if($Stage -eq 'Finish' -and !(Under (Full $GameDir) $temp)){throw 'Fixture cleanup requires a temporary game directory'}
} else {
    if ($SourceRoot -ne (Full (Join-Path $env:LOCALAPPDATA 'Pearl Abyss\CD\save'))) { throw 'Unexpected production source root' }
    if ((Full $GameDir) -ne (Full 'D:\SteamLibrary\steamapps\common\Crimson Desert\bin64')) { throw 'Unexpected production game directory' }
    # CIM failure is fatal, rather than interpreting denied process access as closed.
    $running=@(Get-CimInstance Win32_Process -Filter "Name='CrimsonDesert.exe'")
    if ($Stage -eq 'Run' -and $LaunchRole -eq 'guest') {
        $hostReceipt=Join-Path $SessionRoot 'launch-host.json'
        if (!(Test-Path -LiteralPath $hostReceipt)) { throw 'Launch and load the controlled host first' }
        $hostRecord=Get-Content -LiteralPath $hostReceipt -Raw | ConvertFrom-Json
        if ($running.Count -ne 1 -or $running[0].ProcessId -ne $hostRecord.game_pid) { throw 'Unexpected game process inventory before guest launch' }
        $hostProcess=Get-Process -Id $hostRecord.game_pid
        if ($hostProcess.StartTime.ToUniversalTime().ToFileTimeUtc().ToString() -ne $hostRecord.game_start_filetime -or
            $hostProcess.Path -ne $hostRecord.game_image) { throw 'Host process identity changed' }
    } elseif ($running.Count) { throw 'Close both games first' }
    $expected='8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8'
    if ((Get-FileHash -LiteralPath (Join-Path $GameDir 'CrimsonDesert.exe')).Hash -ne $expected) { throw 'Executable drift: rematch before launch' }
}
function Tree([string]$path) {
    $ids=New-Object 'Collections.Generic.HashSet[string]'
    # Check the root and ancestors too: a junction above the root is ambiguous.
    $walk=$path
    while ($walk) {
        if ((Get-Item -LiteralPath $walk -Force).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw "Linked ancestor: $walk" }
        $parent=Split-Path $walk -Parent
        if ($parent -eq $walk) { break }; $walk=$parent
    }
    [void][CdmpR4Files]::Identity($path)
    $lines=@()
    foreach ($entry in @(Get-ChildItem -LiteralPath $path -Recurse -Force | Sort-Object FullName)) {
        $id=[CdmpR4Files]::Identity($entry.FullName)
        if (!$ids.Add($id)) { throw 'Duplicate file identity in save tree' }
        if (!$entry.PSIsContainer) {
            $relative=$entry.FullName.Substring($path.Length+1)
            if ($relative -match '[\t\r\n:]') { throw 'Unrepresentable manifest name' }
            $lines += "{0}`t{1}`t{2}" -f (Get-FileHash -LiteralPath $entry.FullName).Hash,$entry.Length,$relative
        }
    }
    if (!$lines.Count) { throw 'No source save files' }
    "CDMP-R4-SAVE-1`n"+($lines -join "`n")+"`n"
}
function AssertManifest([string]$dir,[string]$body) { if ((Tree $dir) -cne $body) { throw "Manifest changed: $dir" } }
$manifest=Join-Path $SessionRoot 'save-manifest.tsv'
$receipt=Join-Path $SessionRoot 'session.json'
if ($Stage -eq 'Prepare') {
    if (Test-Path -LiteralPath $SessionRoot) { throw 'Session exists: use a fresh directory' }
    $body=Tree $SourceRoot
    New-Item -ItemType Directory -Path $SessionRoot | Out-Null
    foreach ($role in 'host','guest') {
        $backup=Join-Path $SessionRoot ($role+'-backup')
        $sandbox=Join-Path $SessionRoot ($role+'-sandbox')
        Copy-Item -LiteralPath $SourceRoot -Destination $backup -Recurse
        AssertManifest $backup $body
        Copy-Item -LiteralPath $backup -Destination $sandbox -Recurse
        AssertManifest $sandbox $body
    }
    AssertManifest $SourceRoot $body
    [IO.File]::WriteAllText($manifest,$body,[Text.UTF8Encoding]::new($false))
    @{version=1;source=$SourceRoot;root=$SessionRoot;manifest_sha256=(Get-FileHash -LiteralPath $manifest).Hash;fixture=[bool]$Fixture} |
        ConvertTo-Json | Set-Content -LiteralPath $receipt -Encoding UTF8
    Write-Output "Prepared verified fresh host/guest backups and separate sandbox copies: $SessionRoot"
    return
}
$saveVerificationError=''
try{
    $record=Get-Content -LiteralPath $receipt -Raw | ConvertFrom-Json
    if ($record.version -ne 1 -or $record.source -ne $SourceRoot -or $record.root -ne $SessionRoot -or $record.fixture -ne [bool]$Fixture) { throw 'Preparation receipt mismatch' }
    if ((Get-FileHash -LiteralPath $manifest).Hash -ne $record.manifest_sha256) { throw 'Preparation manifest changed' }
    $body=[IO.File]::ReadAllText($manifest,[Text.Encoding]::UTF8)
    AssertManifest $SourceRoot $body
    foreach ($role in 'host','guest') { AssertManifest (Join-Path $SessionRoot ($role+'-backup')) $body }
}catch{
    if($Stage -ne 'Finish'){throw}
    # Failed save verification must not prevent removal of the controlled
    # installation. Preserve real data and backups; never restore user saves.
    $saveVerificationError=$_.Exception.Message
}
if ($Stage -in @('Verify','Finish')) {
    @{real_manifest_unchanged=(!$saveVerificationError);backup_manifests_verified=(!$saveVerificationError);
      error=$saveVerificationError;manifest_sha256=$record.manifest_sha256;verified_utc=[DateTime]::UtcNow.ToString('o')} |
        ConvertTo-Json | Set-Content -LiteralPath (Join-Path $SessionRoot 'verification.json') -Encoding UTF8
    if(!$saveVerificationError){Write-Output 'Real saves unchanged by SHA256 manifest; both backups verified. Sandboxes retained.'}
    if($Stage -eq 'Verify'){return}
}
if($Stage -eq 'Finish'){
    if(Test-Path -LiteralPath (Join-Path $SessionRoot 'finish.json')){throw 'Session already finished; do not reuse cleanup'}
    $deploymentPath=Join-Path $SessionRoot 'deployment.json'
    if(!(Test-Path -LiteralPath $deploymentPath)){throw 'No controlled deployment receipt to restore'}
    $deployment=Get-Content -LiteralPath $deploymentPath -Raw | ConvertFrom-Json
    foreach($entry in $deployment.files){
        if($entry.name -notin @('cdmp.asi','cdmp.pdb','winmm.dll','cdmp.host.ini','cdmp.guest.ini','cdmp.r4-test-lock')){throw 'Unexpected deployment target'}
        $target=Join-Path $GameDir $entry.name
        if(Test-Path -LiteralPath $target){
            $hash=(Get-FileHash -LiteralPath $target).Hash
            if($hash -ne $entry.installed_sha256 -and (!$entry.existed -or $hash -ne $entry.before_sha256)){throw "Installed file changed: $target"}
        }
        if($entry.existed -and (Get-FileHash -LiteralPath (Join-Path (Join-Path $SessionRoot 'installation-before') $entry.name)).Hash -ne $entry.before_sha256){throw 'Installation backup changed'}
    }
    $evidenceDir=Join-Path $SessionRoot 'evidence'
    New-Item -ItemType Directory -Path $evidenceDir -Force | Out-Null
    foreach($name in @('cdmp.host.log','cdmp.guest.log','cdmp.host.extra.log','cdmp.guest.extra.log')){
        $path=Join-Path $GameDir $name
        if(Test-Path -LiteralPath $path){
            $destination=Join-Path $evidenceDir $name
            if(Test-Path -LiteralPath $destination){
                if((Get-FileHash -LiteralPath $destination).Hash -ne (Get-FileHash -LiteralPath $path).Hash){throw 'Do not overwrite completed session evidence'}
            }else{Copy-Item -LiteralPath $path -Destination $destination}
        }
    }
    foreach($entry in $deployment.files){
        if($entry.name -notin @('cdmp.asi','cdmp.pdb','winmm.dll','cdmp.host.ini','cdmp.guest.ini','cdmp.r4-test-lock')){throw 'Unexpected deployment target'}
        $target=Join-Path $GameDir $entry.name
        $targetExists=Test-Path -LiteralPath $target
        $targetHash=if($targetExists){(Get-FileHash -LiteralPath $target).Hash}else{''}
        if($targetExists -and $targetHash -ne $entry.installed_sha256 -and
            (!$entry.existed -or $targetHash -ne $entry.before_sha256)){throw "Installed file changed: $target"}
        if($entry.existed){
            $prior=Join-Path (Join-Path $SessionRoot 'installation-before') $entry.name
            if((Get-FileHash -LiteralPath $prior).Hash -ne $entry.before_sha256){throw 'Original installation backup changed'}
            Copy-Item -LiteralPath $prior -Destination $target -Force
            if((Get-FileHash -LiteralPath $target).Hash -ne $entry.before_sha256){throw 'Restoration hash mismatch'}
        }elseif($targetExists){Remove-Item -LiteralPath $target -Force}
    }
    $files=@(Get-ChildItem -LiteralPath $evidenceDir -File | ForEach-Object {@{name=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash}})
    @{restored=$true;real_saves_unchanged=(!$saveVerificationError);save_verification_error=$saveVerificationError;
      files=$files;finished_utc=[DateTime]::UtcNow.ToString('o')} |
        ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $SessionRoot 'finish.json') -Encoding UTF8
    if($saveVerificationError){throw "Installation restored, but save verification failed: $saveVerificationError"}
    Write-Output 'Controlled test installation restored; real saves unchanged; local logs and sandboxes retained.'
    return
}
if (!$Present) { throw 'Live presence required before Run' }
if (!$ArmingReceipt) { throw 'Compiled 3663 C/D arming receipt required; guest startup remains denied' }
if($Port -lt 1024 -or $Port -gt 65535){throw 'Invalid loopback port'}
if(!(Get-Process steam -ErrorAction SilentlyContinue)){throw 'Open Steam first'}
$arming=Get-Content -LiteralPath $ArmingReceipt -Raw | ConvertFrom-Json
if($arming.version -ne 1 -or $arming.compiled_tier_mask -ne 15 -or $arming.executable_sha256 -ne $expected -or
    !$arming.full_suite_passed -or !$arming.synthetic_v1_metadata_verified){throw 'Incomplete compiled arming receipt'}
$repo=Split-Path $PSScriptRoot -Parent
foreach($source in $arming.source_manifest){
    $sourcePath=Full (Join-Path $repo $source.path)
    if(!(Under $sourcePath (Full $repo)) -or (Get-FileHash -LiteralPath $sourcePath).Hash -ne $source.sha256){throw 'Source changed after armed validation'}
}
if((Get-FileHash -LiteralPath $arming.asi_path).Hash -ne $arming.asi_sha256){throw 'Armed ASI changed'}
$watchdog=Join-Path $repo 'tools\port3663\r4_session_watchdog.ps1'
if((Get-FileHash -LiteralPath $watchdog).Hash -ne $arming.watchdog_sha256){throw 'Watchdog changed after validation'}
$deploymentPath=Join-Path $SessionRoot 'deployment.json'
if($LaunchRole -eq 'host'){
    if(Test-Path -LiteralPath $deploymentPath){throw 'Session already deployed; do not reuse it'}
    if(Get-NetUDPEndpoint -LocalPort $Port -ErrorAction SilentlyContinue){throw 'Loopback test port is occupied'}
    foreach($role in 'host','guest'){AssertManifest (Join-Path $SessionRoot ($role+'-sandbox')) $body}
    $priorDir=Join-Path $SessionRoot 'installation-before'
    New-Item -ItemType Directory -Path $priorDir | Out-Null
    $iniDir=Join-Path $SessionRoot 'configuration'
    New-Item -ItemType Directory -Path $iniDir | Out-Null
    foreach($role in 'host','guest'){
        $hostMode=if($role -eq 'host'){1}else{0};$guestMode=1-$hostMode
        $ini=@"
[general]
player_name=R4-$role
startup_delay_ms=0
[net]
enabled=1
transport=udp
role=$role
peer_host=127.0.0.1
peer_port=$Port
host_loopback_only=1
send_rate_hz=20
[overlay]
enabled=0
[sniffer]
enabled=0
[coop]
enabled=0
[session2]
enabled=1
core_v1=1
probe=0
guest=$guestMode
create_session=$hostMode
accept_tunnel=$hostMode
"@
        [IO.File]::WriteAllText((Join-Path $iniDir ("cdmp.$role.ini")),$ini,[Text.UTF8Encoding]::new($false))
    }
    $marker=Join-Path $GameDir 'cdmp.r4-test-lock'
    if(Test-Path -LiteralPath $marker){throw 'Another controlled session marker exists'}
    $markerSource=Join-Path $iniDir 'cdmp.r4-test-lock'
    [IO.File]::WriteAllText($markerSource,"CDMP-R4-L3`n$SessionRoot`n",[Text.UTF8Encoding]::new($false))
    $loader=Join-Path $repo 'third_party\ultimate-asi-loader\v9.7.4\dinput8.dll'
    $existingLoader=Join-Path $GameDir 'winmm.dll'
    if((Test-Path -LiteralPath $existingLoader) -and (Get-FileHash -LiteralPath $existingLoader).Hash -ne (Get-FileHash -LiteralPath $loader).Hash){throw 'Different installed loader; preserve it'}
    $sources=@{'cdmp.asi'=$arming.asi_path;'cdmp.pdb'=$arming.pdb_path;'winmm.dll'=$loader;
        'cdmp.host.ini'=(Join-Path $iniDir 'cdmp.host.ini');'cdmp.guest.ini'=(Join-Path $iniDir 'cdmp.guest.ini');'cdmp.r4-test-lock'=$markerSource}
    $deployed=@()
    foreach($name in @('cdmp.asi','cdmp.pdb','winmm.dll','cdmp.host.ini','cdmp.guest.ini','cdmp.r4-test-lock')){
        $target=Join-Path $GameDir $name;$exists=Test-Path -LiteralPath $target;$hash=''
        if($exists){$hash=(Get-FileHash -LiteralPath $target).Hash;Copy-Item -LiteralPath $target -Destination (Join-Path $priorDir $name)}
        $installed=(Get-FileHash -LiteralPath $sources[$name]).Hash
        $deployed+=@{name=$name;existed=[bool]$exists;before_sha256=$hash;installed_sha256=$installed}
    }
    # Record every original and planned copy BEFORE the first installation
    # mutation. Finish can restore even a partially completed deployment.
    @{version=1;files=$deployed;arming_sha256=(Get-FileHash -LiteralPath $ArmingReceipt).Hash;port=$Port} |
        ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $deploymentPath -Encoding UTF8
    foreach($entry in $deployed){
        $target=Join-Path $GameDir $entry.name
        Copy-Item -LiteralPath $sources[$entry.name] -Destination $target -Force
        if((Get-FileHash -LiteralPath $target).Hash -ne $entry.installed_sha256){throw 'Deployment copy mismatch; Finish restores the recorded installation'}
    }
}else{
    $deployment=Get-Content -LiteralPath $deploymentPath -Raw | ConvertFrom-Json
    if($deployment.port -ne $Port -or $deployment.arming_sha256 -ne (Get-FileHash -LiteralPath $ArmingReceipt).Hash){throw 'Deployment receipt mismatch'}
    foreach($entry in $deployment.files){if((Get-FileHash -LiteralPath (Join-Path $GameDir $entry.name)).Hash -ne $entry.installed_sha256){throw 'Deployed file changed'}}
    AssertManifest (Join-Path $SessionRoot 'guest-sandbox') $body
    $hostLogPath=Join-Path $GameDir 'cdmp.host.log'
    $stream=New-Object IO.FileStream($hostLogPath,[IO.FileMode]::Open,[IO.FileAccess]::Read,[IO.FileShare]'ReadWrite, Delete')
    try{$hostLog=(New-Object IO.StreamReader($stream)).ReadToEnd()}finally{$stream.Dispose()}
    if($hostLog -notmatch "\(PID $($hostRecord.game_pid)\)" -or $hostLog -notmatch '3663 v1 host world ready'){throw 'Host has not reached the verified native world-ready state'}
    if([DateTime]::UtcNow -gt [DateTime]::FromFileTimeUtc([long]$hostRecord.game_start_filetime).AddMinutes(6)){throw 'Insufficient host watchdog time; close and prepare a fresh session'}
}
$recordPath=Join-Path $SessionRoot ("launch-$LaunchRole.json")
if(Test-Path -LiteralPath $recordPath){throw 'Role already launched in this session'}
$beforePids=@(Get-CimInstance Win32_Process -Filter "Name='CrimsonDesert.exe'" | ForEach-Object {$_.ProcessId})
$psi=New-Object Diagnostics.ProcessStartInfo
$psi.FileName=Join-Path $GameDir 'CrimsonDesert.exe';$psi.WorkingDirectory=$GameDir
$psi.Arguments='PlatformServiceType=Steam';$psi.UseShellExecute=$false
$psi.EnvironmentVariables['SteamAppId']='3321460';$psi.EnvironmentVariables['SteamGameId']='3321460'
$psi.EnvironmentVariables['CDMP_ROLE']=$LaunchRole
$psi.EnvironmentVariables['CDMP_SAVE_SANDBOX']=Join-Path $SessionRoot ($LaunchRole+'-sandbox')
$psi.EnvironmentVariables['CDMP_SAVE_BACKUP']=Join-Path $SessionRoot ($LaunchRole+'-backup')
$psi.EnvironmentVariables['CDMP_SAVE_MANIFEST']=$manifest
$psi.EnvironmentVariables['CDMP_SAVE_MANIFEST_SHA256']=$record.manifest_sha256
$game=[Diagnostics.Process]::Start($psi)
function Arm([Diagnostics.Process]$process,[string]$name){
    $birth=$process.StartTime.ToUniversalTime().ToFileTimeUtc().ToString()
    if($process.Path -ne $psi.FileName){throw 'Launched image identity mismatch'}
    @{role=$LaunchRole;game_pid=$process.Id;game_start_filetime=$birth;game_image=$process.Path;launched_utc=[DateTime]::UtcNow.ToString('o')} |
        ConvertTo-Json | Set-Content -LiteralPath (Join-Path $SessionRoot ("launch-$name.json")) -Encoding UTF8
    $arguments=@('-NoProfile','-ExecutionPolicy','Bypass','-File',('"'+$watchdog+'"'),'-GamePid',$process.Id,
        '-SessionRoot',('"'+$SessionRoot+'"'),'-Role',$LaunchRole,'-RecordName',$name,'-Present')
    $controller=Start-Process powershell.exe -ArgumentList $arguments -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $SessionRoot ("watchdog-$name.stdout.txt")) -RedirectStandardError (Join-Path $SessionRoot ("watchdog-$name.stderr.txt"))
    $deadline=[DateTime]::UtcNow.AddSeconds(12);$watchPath=Join-Path $SessionRoot ("watchdog-$name.json")
    while([DateTime]::UtcNow -lt $deadline){
        if(Test-Path -LiteralPath $watchPath){
            try{$watch=Get-Content -LiteralPath $watchPath -Raw | ConvertFrom-Json}catch{$watch=$null}
            if($watch -and $watch.game_start_filetime -eq $birth -and $watch.game_pid -eq $process.Id -and
                $watch.controller_pid -eq $controller.Id -and $watch.helper_sha256 -eq $arming.watchdog_sha256 -and $watch.state -in @('watching','exited')){
                Write-Output "Watchdog verified: $name PID $($process.Id), deadline $($watch.deadline_utc)";return
            }
        }
        if($controller.HasExited){break};Start-Sleep -Milliseconds 100
    }
    throw "Watchdog did not arm for $name; quit this controlled game immediately"
}
Arm $game $LaunchRole
Start-Sleep -Milliseconds 300
$newGames=@(Get-CimInstance Win32_Process -Filter "Name='CrimsonDesert.exe'" | Where-Object {$_.ProcessId -notin $beforePids})
foreach($extra in $newGames | Where-Object {$_.ProcessId -ne $game.Id}){
    $extraProcess=Get-Process -Id $extra.ProcessId -ErrorAction SilentlyContinue
    if($extraProcess -and $extraProcess.Path -eq $psi.FileName){Arm $extraProcess ($LaunchRole+'-extra-'+$extraProcess.Id)}
}
if($game.HasExited -or $newGames.Count -ne 1){throw 'Bootstrap/extra process detected; quit all test copies before offline analysis'}
Write-Output "Controlled $LaunchRole launched: PID $($game.Id). Stay at title until collection/preflight is confirmed."
