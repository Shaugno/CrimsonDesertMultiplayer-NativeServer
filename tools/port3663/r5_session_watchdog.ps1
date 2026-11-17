# R5 pinned-process deadline; startup sessions at most ten minutes. No game memory rights.
param([int]$GamePid=0,[string]$SessionRoot='',[ValidateSet('host','guest')][string]$Role='host',
      [ValidatePattern('^(host|guest)(-extra-[0-9]+)?$')][string]$RecordName='host',[switch]$Present,[switch]$SelfTest,
      [ValidateRange(60,840)][int]$MaxAgeSeconds=600)
$ErrorActionPreference='Stop'
if (!('R2SessionDeadline' -as [type])) {
    Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public static class R2SessionDeadline {
 [DllImport("kernel32.dll",SetLastError=true)] static extern IntPtr OpenProcess(uint rights,bool inherit,uint pid);
 [DllImport("kernel32.dll",SetLastError=true)] static extern bool GetProcessTimes(IntPtr h,out long birth,out long exit,out long kernel,out long user);
 [DllImport("kernel32.dll",CharSet=CharSet.Unicode,SetLastError=true)] static extern bool QueryFullProcessImageName(IntPtr h,uint flags,StringBuilder name,ref uint size);
 [DllImport("kernel32.dll",SetLastError=true)] static extern uint WaitForSingleObject(IntPtr h,uint milliseconds);
 [DllImport("kernel32.dll",SetLastError=true)] static extern bool TerminateProcess(IntPtr h,uint code);
 [DllImport("kernel32.dll",SetLastError=true)] static extern bool GetExitCodeProcess(IntPtr h,out uint code);
 [DllImport("kernel32.dll")] public static extern bool CloseHandle(IntPtr h);
 public static IntPtr OpenPinned(uint pid,long expectedBirth,string expectedImage) {
  IntPtr h=OpenProcess(0x101001,false,pid); // synchronize, query-limited, terminate; no VM rights
  if(h==IntPtr.Zero) throw new InvalidOperationException("Cannot open session process");
  try {
   long birth,exit,kernel,user;
   var image=new StringBuilder(1024);uint size=1024;
   if(!GetProcessTimes(h,out birth,out exit,out kernel,out user)||birth!=expectedBirth)
    throw new InvalidOperationException("Session creation identity mismatch");
   if(!QueryFullProcessImageName(h,0,image,ref size)||!String.Equals(image.ToString(),expectedImage,StringComparison.OrdinalIgnoreCase))
    throw new InvalidOperationException("Session executable identity mismatch");
   return h;
  } catch {CloseHandle(h);throw;}
 }
 public static bool WaitUntil(IntPtr h,long deadlineFiletime) {
  long remaining=Math.Max(0,(deadlineFiletime-DateTime.UtcNow.ToFileTimeUtc())/10000);
  uint result=WaitForSingleObject(h,(uint)Math.Min(remaining,Int32.MaxValue));
  if(result==0) return false;
  if(result!=258) throw new InvalidOperationException("Session wait failed");
  if(!TerminateProcess(h,0x3663)) throw new InvalidOperationException("Session deadline stop failed");
  if(WaitForSingleObject(h,10000)!=0) throw new InvalidOperationException("Session did not exit after deadline stop");
  return true;
 }
 public static uint ExitCode(IntPtr h) {
  uint code;if(!GetExitCodeProcess(h,out code))throw new InvalidOperationException("Cannot read exit code");return code;
 }
}
'@
}
if ($SelfTest) {
    $shell=(Get-Process -Id $PID).Path
    foreach ($mode in @('deadline','normal','mismatch')) {
        $command=if ($mode -eq 'normal') {'Start-Sleep -Milliseconds 700'} else {'Start-Sleep -Seconds 30'}
        $child=Start-Process -FilePath $shell -ArgumentList @('-NoProfile','-Command',$command) -WindowStyle Hidden -PassThru
        $handle=[IntPtr]::Zero
        try {
            $birth=$child.StartTime.ToUniversalTime().ToFileTimeUtc()
            if ($mode -eq 'mismatch') {
                $rejected=$false
                try { $handle=[R2SessionDeadline]::OpenPinned($child.Id,$birth+1,$shell) }
                catch { if ($_.Exception.Message -notmatch 'creation identity mismatch') { throw };$rejected=$true }
                if (!$rejected -or $child.HasExited) { throw 'Identity rejection failed' }
            } else {
                $handle=[R2SessionDeadline]::OpenPinned($child.Id,$birth,$shell)
                $forced=[R2SessionDeadline]::WaitUntil($handle,[DateTime]::UtcNow.AddSeconds(2).ToFileTimeUtc())
                if ($forced -ne ($mode -eq 'deadline')) { throw 'Watchdog outcome mismatch' }
            }
            Write-Output "Watchdog fixture $mode PASS; owned hidden child only"
        } finally {
            if ($handle -ne [IntPtr]::Zero) { [void][R2SessionDeadline]::CloseHandle($handle) }
            if (!$child.HasExited) { $child.Kill();$child.WaitForExit(5000) | Out-Null }
        }
    }
    return
}
if (!$Present -or $GamePid -lt 1 -or !$SessionRoot) { throw 'Explicit user presence, session root and game PID required' }
$SessionRoot=[IO.Path]::GetFullPath($SessionRoot)
if (!(Test-Path -LiteralPath (Join-Path $SessionRoot 'deployment.json'))) { throw 'Prepare and deploy this session first' }
if (!$RecordName.StartsWith($Role,[StringComparison]::Ordinal)) { throw 'Watchdog role/record mismatch' }
$launch=Get-Content -LiteralPath (Join-Path $SessionRoot ("launch-$RecordName.json")) -Raw | ConvertFrom-Json
if ($launch.game_pid -ne $GamePid -or $launch.role -ne $Role) { throw 'Launch receipt mismatch' }
$image=[IO.Path]::GetFullPath($launch.game_image)
if([IO.Path]::GetFileName($image) -ne 'CrimsonDesert.exe'){throw 'Unexpected launched executable name'}
if ((Get-FileHash -LiteralPath $image).Hash -ne '8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8') { throw 'Build drift' }
$birth=[long]$launch.game_start_filetime
$deadline=[DateTime]::FromFileTimeUtc($birth).AddSeconds($MaxAgeSeconds)
$hostReceipt=Join-Path $SessionRoot 'launch-host.json'
if(Test-Path -LiteralPath $hostReceipt){
    $hostRecord=Get-Content -LiteralPath $hostReceipt -Raw | ConvertFrom-Json
    $sessionDeadline=[DateTime]::FromFileTimeUtc([long]$hostRecord.game_start_filetime).AddSeconds([int]$hostRecord.max_age_seconds)
    if($sessionDeadline -lt $deadline){$deadline=$sessionDeadline}
}
$handle=[R2SessionDeadline]::OpenPinned($GamePid,$birth,$image)
$path=Join-Path $SessionRoot ("watchdog-$RecordName.json")
$record=[ordered]@{state='watching';game_pid=$GamePid;game_start_filetime=$birth.ToString();game_image=$image;
    controller_pid=$PID;controller_start_filetime=(Get-Process -Id $PID).StartTime.ToUniversalTime().ToFileTimeUtc().ToString();
    deadline_utc=$deadline.ToString('o');max_game_age_seconds=$MaxAgeSeconds;helper_sha256=(Get-FileHash -LiteralPath $PSCommandPath).Hash}
try {
    $record | ConvertTo-Json | Set-Content -LiteralPath $path
    Write-Output "Watchdog armed for PID $GamePid; absolute deadline $($record.deadline_utc)"
    $forced=[R2SessionDeadline]::WaitUntil($handle,$deadline.ToFileTimeUtc())
    $record.state=if ($forced) {'deadline_stopped'} else {'exited'}
    $record['game_exit_code']=[R2SessionDeadline]::ExitCode($handle)
    $record['ended_utc']=[DateTime]::UtcNow.ToString('o')
    $record | ConvertTo-Json | Set-Content -LiteralPath $path
    Write-Output "Watchdog finished: $($record.state)"
} finally { [void][R2SessionDeadline]::CloseHandle($handle) }
