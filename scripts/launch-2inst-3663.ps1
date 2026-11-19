# R7: host uses backed-up real saves; only guest gets verified NT save isolation.
# Preparation/verification are game-free; Run requires explicit live presence.
param(
    [Parameter(Mandatory=$true)][ValidateSet('Prepare','Run','Verify','Finish')][string]$Stage,
    [Parameter(Mandatory=$true)][string]$SessionRoot,
    [string]$SourceRoot = (Join-Path $env:LOCALAPPDATA 'Pearl Abyss\CD\save'),
    [string]$GameDir = 'D:\SteamLibrary\steamapps\common\Crimson Desert\bin64',
    [string]$ArmingReceipt = '',
    [ValidateSet('host','guest')][string]$LaunchRole='host',
    [ValidateSet('HostStartup','GuestStartup','L3','L4Host','L4Guest')][string]$SessionKind='L3',
    [ValidateSet('profile','nt')][string]$IsolationMethod='nt',
    [UInt64]$PeerSteamId=0,
    [int]$Port=39766,
    [switch]$Present,
    [switch]$Fixture,
    [switch]$DesktopLaunchPlan
)
$ErrorActionPreference='Stop'
$r7NotBefore=[DateTimeOffset]::Parse('2026-11-18T15:00:00Z').UtcDateTime
if($Stage -in @('Prepare','Run') -and $IsolationMethod -ne 'nt'){throw 'R7 requires guest-only NT save-directory isolation; profile redirection is forbidden'}
if($Stage -eq 'Run' -and !$Fixture -and [DateTime]::UtcNow -lt $r7NotBefore){throw 'R7 game launch forbidden before 2026-11-18 10:00 AM Eastern (15:00 UTC)'}
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
    if([IO.Path]::GetFileName((Full (Join-Path $GameDir 'CrimsonDesert.exe'))) -ne 'CrimsonDesert.exe'){throw 'Unexpected game executable'}
    # CIM failure is fatal, rather than interpreting denied process access as closed.
    $running=@(Get-CimInstance Win32_Process -Filter "Name='CrimsonDesert.exe'")
    if (($Stage -eq 'Run' -and $LaunchRole -eq 'guest' -and $SessionKind -eq 'L3') -or
        ($Stage -eq 'Verify' -and $SessionKind -eq 'L3' -and $running.Count -eq 1)) {
        $hostReceipt=Join-Path $SessionRoot 'launch-host.json'
        if (!(Test-Path -LiteralPath $hostReceipt)) { throw 'Launch and load the controlled host first' }
        $hostRecord=Get-Content -LiteralPath $hostReceipt -Raw | ConvertFrom-Json
        if($DesktopLaunchPlan){
            $hostWatch=Get-Content -LiteralPath (Join-Path $SessionRoot 'watchdog-host.json') -Raw | ConvertFrom-Json
            if($hostWatch.backend -ne 'desktop-app' -or $hostWatch.state -ne 'watching' -or
                $hostWatch.game_pid -ne $hostRecord.game_pid -or $hostWatch.game_start_filetime -ne $hostRecord.game_start_filetime){throw 'Desktop host controller identity/state mismatch'}
            # The desktop runner rechecks the exact live inventory and pinned
            # host itself before spawning. Do not substitute this command
            # context's window/process view for the visible desktop's view.
        }else{
            if ($running.Count -ne 1 -or $running[0].ProcessId -ne $hostRecord.game_pid) { throw 'Unexpected game process inventory before guest launch' }
            $hostProcess=Get-Process -Id $hostRecord.game_pid
            if ($hostProcess.StartTime.ToUniversalTime().ToFileTimeUtc().ToString() -ne $hostRecord.game_start_filetime -or
                $hostProcess.Path -ne $hostRecord.game_image) { throw 'Host process identity changed' }
        }
    } elseif ($running.Count) { throw 'Close both games first' }
    $expected='DFCF5C507D09AA0E09CF9766DB866FD1E7CF513B9DD7FDC72A8F121BA2D9A9D5'  # 1.0.0.3732 (R10; 3663 retired)
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
        Copy-Item -LiteralPath $SourceRoot -Destination $backup -Recurse
        AssertManifest $backup $body
    }
    $guestProfile='' # R7 never creates or redirects a user profile.
    $sandbox=Join-Path $SessionRoot 'guest-sandbox'
    New-Item -ItemType Directory -Force -Path (Split-Path $sandbox -Parent) | Out-Null
    Copy-Item -LiteralPath (Join-Path $SessionRoot 'guest-backup') -Destination $sandbox -Recurse
    AssertManifest $sandbox $body
    AssertManifest $SourceRoot $body
    [IO.File]::WriteAllText($manifest,$body,[Text.UTF8Encoding]::new($false))
    @{version=2;source=$SourceRoot;root=$SessionRoot;session_kind=$SessionKind;isolation_method=$IsolationMethod;
      guest_profile=$guestProfile;guest_sandbox=$sandbox;manifest_sha256=(Get-FileHash -LiteralPath $manifest).Hash;fixture=[bool]$Fixture} |
        ConvertTo-Json | Set-Content -LiteralPath $receipt -Encoding UTF8
    Write-Output "Prepared verified host/guest backups; host uses real saves, guest isolation $IsolationMethod : $SessionRoot"
    return
}
$saveVerificationError=''
$realUnchanged=$false
$hostMayWrite=$SessionKind -in @('HostStartup','L3','L4Host')
try{
    $record=Get-Content -LiteralPath $receipt -Raw | ConvertFrom-Json
    if ($record.version -ne 2 -or $record.source -ne $SourceRoot -or $record.root -ne $SessionRoot -or
        $record.fixture -ne [bool]$Fixture -or $record.session_kind -ne $SessionKind -or $record.isolation_method -ne $IsolationMethod) { throw 'Preparation receipt mismatch' }
    if ((Get-FileHash -LiteralPath $manifest).Hash -ne $record.manifest_sha256) { throw 'Preparation manifest changed' }
    $body=[IO.File]::ReadAllText($manifest,[Text.Encoding]::UTF8)
    $currentBody=Tree $SourceRoot
    $realUnchanged=$currentBody -ceq $body
    if(!$hostMayWrite -and !$realUnchanged){throw 'Guest-only real-save manifest changed'}
    foreach ($role in 'host','guest') { AssertManifest (Join-Path $SessionRoot ($role+'-backup')) $body }
}catch{
    if($Stage -ne 'Finish'){throw}
    # Failed save verification must not prevent removal of the controlled
    # installation. Preserve real data and backups; never restore user saves.
    $saveVerificationError=$_.Exception.Message
}
if ($Stage -in @('Verify','Finish')) {
    if($currentBody){[IO.File]::WriteAllText((Join-Path $SessionRoot 'save-after.tsv'),$currentBody,[Text.UTF8Encoding]::new($false))}
    @{real_manifest_unchanged=$realUnchanged;host_changes_allowed=$hostMayWrite;backup_manifests_verified=(!$saveVerificationError);
      error=$saveVerificationError;manifest_sha256=$record.manifest_sha256;
      after_manifest_sha256=if($currentBody){(Get-FileHash -LiteralPath (Join-Path $SessionRoot 'save-after.tsv')).Hash}else{''};
      verified_utc=[DateTime]::UtcNow.ToString('o')} |
        ConvertTo-Json | Set-Content -LiteralPath (Join-Path $SessionRoot 'verification.json') -Encoding UTF8
    if(!$saveVerificationError){Write-Output "Save policy verified: real unchanged=$realUnchanged host changes allowed=$hostMayWrite; frozen backups verified."}
    if($Stage -eq 'Verify'){return}
}
if($Stage -eq 'Finish'){
    # Desktop-launched games have a controller on the visible desktop. Never
    # infer that they are gone solely from another launch context's inventory.
    foreach($launch in @(Get-ChildItem -LiteralPath $SessionRoot -Filter 'launch-*.json' -File)){
        $roleName=$launch.BaseName.Substring(7);$watchPath=Join-Path $SessionRoot ("watchdog-$roleName.json")
        if(!(Test-Path -LiteralPath $watchPath)){throw 'Missing controller exit receipt; do not clean a possibly live game'}
        $watchRecord=Get-Content -LiteralPath $watchPath -Raw | ConvertFrom-Json
        if($watchRecord.state -notin @('exited','deadline_stopped')){throw 'Controller still watching; quit the controlled game first'}
    }
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
    @{restored=$true;real_saves_unchanged=$realUnchanged;host_changes_allowed=$hostMayWrite;save_verification_error=$saveVerificationError;
      files=$files;finished_utc=[DateTime]::UtcNow.ToString('o')} |
        ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $SessionRoot 'finish.json') -Encoding UTF8
    if($saveVerificationError){throw "Installation restored, but save verification failed: $saveVerificationError"}
    Write-Output 'Controlled installation restored; save policy verified; host changes retained and never restored over.'
    return
}
if (!$Present) { throw 'Live presence required before Run' }
if(($SessionKind -in @('HostStartup','L4Host') -and $LaunchRole -ne 'host') -or
   ($SessionKind -in @('GuestStartup','L4Guest') -and $LaunchRole -ne 'guest')){throw 'Role does not match session kind'}
if (!$ArmingReceipt) { throw 'Compiled 3663 C/D arming receipt required; guest startup remains denied' }
if($Port -lt 1024 -or $Port -gt 65535){throw 'Invalid loopback port'}
if(!(Get-Process steam -ErrorAction SilentlyContinue)){throw 'Open Steam first'}
$arming=Get-Content -LiteralPath $ArmingReceipt -Raw | ConvertFrom-Json
if($arming.version -ne 3 -or $arming.round -ne 7 -or $arming.isolation_method -ne 'nt' -or $arming.host_isolation -ne 'none' -or
    $arming.not_before_utc -ne '2026-11-18T15:00:00Z' -or $arming.compiled_tier_mask -ne 15 -or $arming.executable_sha256 -ne $expected -or
    !$arming.full_suite_passed -or !$arming.synthetic_v1_metadata_verified){throw 'Incomplete compiled arming receipt'}
if($DesktopLaunchPlan){
    if($IsolationMethod -ne 'nt' -or !$arming.desktop_runner_path -or
        (Get-FileHash -LiteralPath $arming.desktop_runner_path).Hash -ne $arming.desktop_runner_sha256){throw 'Validated desktop runner required'}
}
$repo=Split-Path $PSScriptRoot -Parent
foreach($source in $arming.source_manifest){
    $sourcePath=Full (Join-Path $repo $source.path)
    if(!(Under $sourcePath (Full $repo)) -or (Get-FileHash -LiteralPath $sourcePath).Hash -ne $source.sha256){throw 'Source changed after armed validation'}
}
if((Get-FileHash -LiteralPath $arming.asi_path).Hash -ne $arming.asi_sha256){throw 'Armed ASI changed'}
$watchdog=Join-Path $repo 'tools\port3663\r5_session_watchdog.ps1'
if((Get-FileHash -LiteralPath $watchdog).Hash -ne $arming.watchdog_sha256){throw 'Watchdog changed after validation'}
$deploymentPath=Join-Path $SessionRoot 'deployment.json'
if(!(Test-Path -LiteralPath $deploymentPath)){
    if(Test-Path -LiteralPath $deploymentPath){throw 'Session already deployed; do not reuse it'}
    if(Get-NetUDPEndpoint -LocalPort $Port -ErrorAction SilentlyContinue){throw 'Loopback test port is occupied'}
    AssertManifest $record.guest_sandbox $body
    $priorDir=Join-Path $SessionRoot 'installation-before'
    New-Item -ItemType Directory -Path $priorDir | Out-Null
    $iniDir=Join-Path $SessionRoot 'configuration'
    New-Item -ItemType Directory -Path $iniDir | Out-Null
    foreach($role in 'host','guest'){
        $hostMode=if($role -eq 'host'){1}else{0};$guestMode=1-$hostMode
        $startupOnly=if($SessionKind -eq 'GuestStartup'){1}else{0};$enabled=1-$startupOnly
        $transport=if($SessionKind -in @('L4Host','L4Guest')){'steam'}else{'udp'}
        # R10 Step 2: the host asks the game for the guest's own 2941 (milestone3663.h); the guest never does.
        $milestone=if($role -eq 'host'){2}else{0}
        # R10 Step 3: remote-controlled actor containment (character_guard3663). Decided before
        # arming (this file is in the armed source manifest): 0 = record controller evidence only.
        $remoteActorGuard=0
        $ini=@"
[general]
player_name=R5-$role
startup_delay_ms=0
[net]
enabled=$enabled
transport=$transport
role=$role
peer_host=127.0.0.1
peer_port=$Port
host_loopback_only=1
send_rate_hz=20
[steam]
peer_steamid=$PeerSteamId
[overlay]
enabled=0
[sniffer]
enabled=0
[coop]
enabled=0
[session2]
enabled=$enabled
core_v1=1
core_milestone=$milestone
remote_actor_guard=$remoteActorGuard
guest_startup_only=$startupOnly
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
    [IO.File]::WriteAllText($markerSource,"CDMP-R5-$SessionKind`n$SessionRoot`n",[Text.UTF8Encoding]::new($false))
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
    AssertManifest $record.guest_sandbox $body
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
foreach($key in @('CDMP_SAVE_SANDBOX','CDMP_SAVE_BACKUP','CDMP_SAVE_MANIFEST','CDMP_SAVE_MANIFEST_SHA256','CDMP_HOST_REAL_SAVES','CDMP_HOST_CRASH_DIR','CDMP_SAVE_ISOLATION','CDMP_REAL_SAVE_ROOT','CDMP_GUEST_WITH_HOST')){$psi.EnvironmentVariables.Remove($key)}
if($LaunchRole -eq 'host'){
    $psi.EnvironmentVariables['CDMP_HOST_REAL_SAVES']='1'
    $psi.EnvironmentVariables['CDMP_HOST_CRASH_DIR']=Join-Path $SessionRoot 'evidence'
}
else{
    $psi.EnvironmentVariables['CDMP_SAVE_SANDBOX']=$record.guest_sandbox
    $psi.EnvironmentVariables['CDMP_SAVE_BACKUP']=Join-Path $SessionRoot 'guest-backup'
    $psi.EnvironmentVariables['CDMP_SAVE_MANIFEST']=$manifest
    $psi.EnvironmentVariables['CDMP_SAVE_MANIFEST_SHA256']=$record.manifest_sha256
    $psi.EnvironmentVariables['CDMP_REAL_SAVE_ROOT']=$SourceRoot
    $psi.EnvironmentVariables['CDMP_SAVE_ISOLATION']=$IsolationMethod
    $psi.EnvironmentVariables['CDMP_GUEST_WITH_HOST']=if($SessionKind -eq 'L3'){'1'}else{'0'}
    # USERPROFILE and LOCALAPPDATA are inherited unchanged, for both roles.
}
$maxAge=if($SessionKind -in @('HostStartup','GuestStartup')){600}else{840}
if($DesktopLaunchPlan){
    # Export only the explicit role/isolation values, never the inherited
    # environment (which may contain credentials). The desktop app starts the
    # exact game directly; it never invokes a shell or a terminal.
    $export=@{}
    $keys=@('SteamAppId','SteamGameId','CDMP_ROLE')
    if($LaunchRole -eq 'host'){$keys+=@('CDMP_HOST_REAL_SAVES','CDMP_HOST_CRASH_DIR')}else{
        $keys+=@('CDMP_SAVE_SANDBOX','CDMP_SAVE_BACKUP','CDMP_SAVE_MANIFEST','CDMP_SAVE_MANIFEST_SHA256',
          'CDMP_REAL_SAVE_ROOT','CDMP_SAVE_ISOLATION','CDMP_GUEST_WITH_HOST')
    }
    foreach($key in $keys){$export[$key]=$psi.EnvironmentVariables[$key]}
    $runner=Join-Path $SessionRoot ("r5_${LaunchRole}_desktop.exe")
    $planPath=Join-Path $SessionRoot ("r5_${LaunchRole}_desktop.plan.json")
    if((Test-Path -LiteralPath $runner) -or (Test-Path -LiteralPath $planPath)){throw 'Desktop role plan already exists'}
    Copy-Item -LiteralPath $arming.desktop_runner_path -Destination $runner
    if((Get-FileHash -LiteralPath $runner).Hash -ne $arming.desktop_runner_sha256){throw 'Desktop runner copy changed'}
    $plan=@{version=1;present=$true;role=$LaunchRole;session_root=$SessionRoot;repo_root=$repo;game_image=$psi.FileName;
      arming_receipt=(Full $ArmingReceipt);arming_sha256=(Get-FileHash -LiteralPath $ArmingReceipt).Hash;
      session_kind=$SessionKind;max_age_seconds=$maxAge;environment=$export;
      inherited_environment=@{USERPROFILE=$env:USERPROFILE;LOCALAPPDATA=$env:LOCALAPPDATA};planned_utc=[DateTime]::UtcNow.ToString('o')}
    [IO.File]::WriteAllText($planPath,($plan | ConvertTo-Json -Depth 5),[Text.UTF8Encoding]::new($false))
    Write-Output "Desktop plan prepared; no game launched. Open the validated desktop app: $runner"
    return
}
$game=[Diagnostics.Process]::Start($psi)
function Arm([Diagnostics.Process]$process,[string]$name){
    $birth=$process.StartTime.ToUniversalTime().ToFileTimeUtc().ToString()
    if($process.Path -ne $psi.FileName){throw 'Launched image identity mismatch'}
    @{role=$LaunchRole;game_pid=$process.Id;game_start_filetime=$birth;game_image=$process.Path;max_age_seconds=$maxAge;
      isolation=if($LaunchRole -eq 'guest'){$IsolationMethod}else{'none'};session_kind=$SessionKind;launched_utc=[DateTime]::UtcNow.ToString('o')} |
        ConvertTo-Json | Set-Content -LiteralPath (Join-Path $SessionRoot ("launch-$name.json")) -Encoding UTF8
    $arguments=@('-NoProfile','-ExecutionPolicy','Bypass','-File',('"'+$watchdog+'"'),'-GamePid',$process.Id,
        '-SessionRoot',('"'+$SessionRoot+'"'),'-Role',$LaunchRole,'-RecordName',$name,'-MaxAgeSeconds',$maxAge,'-Present')
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
