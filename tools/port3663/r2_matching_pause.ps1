# Pause only explicitly identified R2 analysis jobs, never a game or other app.
# Native declarations: https://github.com/winsiderss/phnt/blob/master/ntpsapi.h
param(
    [Parameter(Mandatory=$true)][ValidateSet('Pause','Verify','Resume')][string]$Stage,
    [Parameter(Mandatory=$true)][ValidatePattern('^R2-MATCH-PAUSE-[0-9]{8}-[0-9]{6}$')][string]$Session,
    [int[]]$ProcessIds=@(),
    [string]$AnalysisRoot=''
)
$ErrorActionPreference='Stop'
$root=Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$folder=Join-Path $root 'build\r2\matching-pause'
$receiptPath=Join-Path $folder "$Session.json"
if ($Stage -ne 'Verify' -and (Get-Process CrimsonDesert -ErrorAction SilentlyContinue)) {
    throw 'Pause/resume analysis only with the game closed'
}
if (!('R2MatchingNative' -as [type])) {
    Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class R2MatchingNative {
    [DllImport("kernel32.dll",SetLastError=true)] public static extern IntPtr OpenProcess(uint rights,bool inherit,uint pid);
    [DllImport("kernel32.dll",SetLastError=true)] public static extern IntPtr OpenThread(uint rights,bool inherit,uint tid);
    [DllImport("kernel32.dll")] public static extern bool CloseHandle(IntPtr handle);
    [DllImport("kernel32.dll",SetLastError=true)] public static extern bool GetProcessTimes(IntPtr handle,out long creation,out long exit,out long kernel,out long user);
    [DllImport("ntdll.dll")] public static extern int NtSuspendProcess(IntPtr handle);
    [DllImport("ntdll.dll")] public static extern int NtResumeProcess(IntPtr handle);
    [DllImport("ntdll.dll")] public static extern int NtQueryInformationThread(IntPtr handle,int kind,out uint value,uint size,out uint returned);
}
'@
}
function TextHash([string]$value) {
    $hash=[Security.Cryptography.SHA256]::Create()
    try { [BitConverter]::ToString($hash.ComputeHash([Text.Encoding]::UTF8.GetBytes($value))).Replace('-','') }
    finally { $hash.Dispose() }
}
function Snapshot([int]$number,[string]$ar) {
    $info=Get-CimInstance Win32_Process -Filter "ProcessId=$number"
    if (!$info) { throw "Owned analysis process $number is absent" }
    $process=Get-Process -Id $number
    $java=[IO.Path]::GetFullPath((Join-Path $ar 'jdk-21.0.13+9\bin\java.exe'))
    $bin=[IO.Path]::GetFullPath((Join-Path $ar 'bindiff8\ProgramFiles\BinDiff\bin\bindiff.exe'))
    $command=[string]$info.CommandLine
    $role=''
    if ($info.ExecutablePath -eq $java -and $command.Contains('R2NativeSavedTarget') -and
        $command.Contains('AutoVersionTrackingScript.java') -and $command.Contains($root)) { $role='VT' }
    if ($info.ExecutablePath -eq $bin -and $command.Contains('r2-nativefix') -and
        $command.Contains('old.BinExport') -and $command.Contains('new.BinExport')) { $role='BinDiff' }
    if (!$role) { throw "Process $number is not an allowed owned R2 matcher" }
    [pscustomobject]@{pid=$number;role=$role;exe=$info.ExecutablePath;
        start_filetime=$process.StartTime.ToUniversalTime().ToFileTimeUtc().ToString();
        command_sha256=(TextHash $command);exe_sha256=(Get-FileHash -LiteralPath $info.ExecutablePath).Hash}
}
function SameIdentity($before,$now) {
    foreach ($key in @('pid','role','exe','start_filetime','command_sha256','exe_sha256')) {
        if ($before.$key -ne $now.$key) { throw "Analysis identity changed: $key" }
    }
}
function ProcessHandle($snapshot,[bool]$control) {
    $rights=if ($control) {0x1800} else {0x1000}
    $handle=[R2MatchingNative]::OpenProcess($rights,$false,$snapshot.pid)
    if ($handle -eq [IntPtr]::Zero) { throw 'Cannot open the owned analysis process' }
    [long]$created=0; [long]$ended=0; [long]$kernel=0; [long]$user=0
    if (![R2MatchingNative]::GetProcessTimes($handle,[ref]$created,[ref]$ended,[ref]$kernel,[ref]$user) -or
        $created.ToString() -ne $snapshot.start_filetime) {
        [void][R2MatchingNative]::CloseHandle($handle)
        throw 'Analysis PID changed before acquiring its handle'
    }
    $handle
}
function Counts([int]$number) {
    $process=Get-Process -Id $number
    $rows=@(foreach ($thread in $process.Threads) {
        $handle=[R2MatchingNative]::OpenThread(0x40,$false,$thread.Id)
        if ($handle -eq [IntPtr]::Zero) { throw 'Cannot query an analysis thread' }
        try {
            [uint32]$count=0; [uint32]$returned=0
            $status=[R2MatchingNative]::NtQueryInformationThread($handle,35,[ref]$count,4,[ref]$returned)
            if ($status -lt 0 -or $returned -ne 4) { throw ('Thread suspend query failed: {0:X8}' -f $status) }
            [pscustomobject]@{tid=$thread.Id;count=$count}
        } finally { [void][R2MatchingNative]::CloseHandle($handle) }
    })
    if (!$rows.Count) { throw 'No analysis threads to verify' }
    $rows
}
function SaveReceipt($value) {
    ConvertTo-Json -InputObject $value -Depth 8 | Set-Content -LiteralPath $receiptPath
}
if ($Stage -eq 'Pause') {
    if (!$AnalysisRoot -or $ProcessIds.Count -lt 1 -or $ProcessIds.Count -gt 2 -or
        @($ProcessIds | Select-Object -Unique).Count -ne $ProcessIds.Count -or
        @($ProcessIds | Where-Object {$_ -le 0}).Count) { throw 'Supply one or two explicit owned matcher PIDs and analysis root' }
    if (Test-Path -LiteralPath $receiptPath) { throw 'Use a fresh pause receipt name' }
    $ar=[IO.Path]::GetFullPath($AnalysisRoot)
    $entries=@($ProcessIds | ForEach-Object { Snapshot $_ $ar })
    if (@($entries.role | Select-Object -Unique).Count -ne $entries.Count) { throw 'Duplicate matcher role' }
    New-Item -ItemType Directory -Force -Path $folder | Out-Null
    $record=[ordered]@{session=$Session;analysis_root=$ar;state='preparing';created_utc=[DateTime]::UtcNow.ToString('o');entries=$entries}
    $handles=@{}; $suspended=@(); $success=$false
    try {
        foreach ($entry in $entries) {
            $handles[$entry.pid]=ProcessHandle $entry $true
            $before=@(Counts $entry.pid)
            if (@($before | Where-Object count -ne 0).Count) { throw 'A matcher was already suspended; do not take ownership' }
            $entry | Add-Member NoteProperty before_threads $before
            $entry | Add-Member NoteProperty pause_pending $false
            $entry | Add-Member NoteProperty resume_issued $false
        }
        SaveReceipt $record
        foreach ($entry in $entries) {
            SameIdentity $entry (Snapshot $entry.pid $ar)
            $entry.pause_pending=$true
            SaveReceipt $record
            $status=[R2MatchingNative]::NtSuspendProcess($handles[$entry.pid])
            if ($status -lt 0) { throw ('Matcher suspension failed: {0:X8}' -f $status) }
            $suspended+=,$entry
            $after=@(Counts $entry.pid)
            if (@($after | Where-Object count -lt 1).Count) { throw 'Not every matcher thread is suspended' }
            $entry | Add-Member NoteProperty paused_threads $after
            SaveReceipt $record
        }
        $record.state='paused'
        SaveReceipt $record
        $success=$true
    } finally {
        if (!$success) {
            foreach ($entry in $suspended) { [void][R2MatchingNative]::NtResumeProcess($handles[$entry.pid]) }
        }
        foreach ($handle in $handles.Values) { [void][R2MatchingNative]::CloseHandle($handle) }
    }
    Write-Output "Paused owned matchers: $($ProcessIds -join ', '); every thread verified; receipt $Session"
    return
}
if (!(Test-Path -LiteralPath $receiptPath)) { throw 'Missing pause receipt' }
$record=Get-Content -Raw -LiteralPath $receiptPath | ConvertFrom-Json
if ($record.session -ne $Session) { throw 'Pause receipt session mismatch' }
if ($Stage -eq 'Resume' -and $record.state -eq 'resumed') { Write-Output 'Receipt already resumed; no native operation'; return }
if ($Stage -eq 'Verify' -and $record.state -ne 'paused') { throw 'Receipt is not a verified pause' }
foreach ($entry in @($record.entries)) {
    $now=Snapshot $entry.pid $record.analysis_root
    SameIdentity $entry $now
    $handle=ProcessHandle $entry ($Stage -eq 'Resume')
    try {
        $counts=@(Counts $entry.pid)
        if ($Stage -eq 'Verify') {
            if (!$entry.pause_pending -or $entry.resume_issued -or @($counts | Where-Object count -lt 1).Count) {
                throw 'Matcher is not completely suspended by this receipt'
            }
        } elseif ($entry.pause_pending -and !$entry.resume_issued) {
            if (@($counts | Where-Object count -gt 0).Count) {
                $status=[R2MatchingNative]::NtResumeProcess($handle)
                if ($status -lt 0) { throw ('Matcher resumption failed: {0:X8}' -f $status) }
            }
            $entry.resume_issued=$true
            SaveReceipt $record
            if (@(Counts $entry.pid | Where-Object count -ne 0).Count) { throw 'A thread remains suspended; do not resume it again blindly' }
        }
    } finally { [void][R2MatchingNative]::CloseHandle($handle) }
}
if ($Stage -eq 'Resume') {
    $record.state='resumed'
    $record | Add-Member NoteProperty resumed_utc ([DateTime]::UtcNow.ToString('o')) -Force
    SaveReceipt $record
}
Write-Output "$Stage owned matcher receipt: OK ($Session)"
