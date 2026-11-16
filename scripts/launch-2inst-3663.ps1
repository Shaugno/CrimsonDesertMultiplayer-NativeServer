# R4: preparation and manifest verification are game-free. Run is separately
# gated by presence and a compiled C/D receipt; no legacy launcher behavior changes.
param(
    [Parameter(Mandatory=$true)][ValidateSet('Prepare','Run','Verify')][string]$Stage,
    [Parameter(Mandatory=$true)][string]$SessionRoot,
    [string]$SourceRoot = (Join-Path $env:LOCALAPPDATA 'Pearl Abyss\CD\save'),
    [string]$GameDir = 'D:\SteamLibrary\steamapps\common\Crimson Desert\bin64',
    [string]$ArmingReceipt = '',
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
} else {
    if ($SourceRoot -ne (Full (Join-Path $env:LOCALAPPDATA 'Pearl Abyss\CD\save'))) { throw 'Unexpected production source root' }
    # CIM failure is fatal, rather than interpreting denied process access as closed.
    if (@(Get-CimInstance Win32_Process -Filter "Name='CrimsonDesert.exe'").Count) { throw 'Close both games first' }
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
$record=Get-Content -LiteralPath $receipt -Raw | ConvertFrom-Json
if ($record.version -ne 1 -or $record.source -ne $SourceRoot -or $record.root -ne $SessionRoot -or $record.fixture -ne [bool]$Fixture) { throw 'Preparation receipt mismatch' }
if ((Get-FileHash -LiteralPath $manifest).Hash -ne $record.manifest_sha256) { throw 'Preparation manifest changed' }
$body=[IO.File]::ReadAllText($manifest,[Text.Encoding]::UTF8)
AssertManifest $SourceRoot $body
foreach ($role in 'host','guest') { AssertManifest (Join-Path $SessionRoot ($role+'-backup')) $body }
if ($Stage -eq 'Verify') {
    @{real_manifest_unchanged=$true;backup_manifests_verified=$true;manifest_sha256=$record.manifest_sha256;verified_utc=[DateTime]::UtcNow.ToString('o')} |
        ConvertTo-Json | Set-Content -LiteralPath (Join-Path $SessionRoot 'verification.json') -Encoding UTF8
    Write-Output 'Real saves unchanged by SHA256 manifest; both backups verified. Sandboxes retained.'
    return
}
if (!$Present) { throw 'Live presence required before Run' }
if (!$ArmingReceipt) { throw 'Compiled 3663 C/D arming receipt required; guest startup remains denied' }
# A later armed backend must supply and validate this receipt. A file claiming
# permission cannot currently enable an incomplete native join implementation.
throw 'R4 C/D native backend is not armed; launch refused'
