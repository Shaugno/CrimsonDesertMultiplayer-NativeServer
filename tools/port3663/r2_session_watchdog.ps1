# Independent launch-to-close deadline. Collector memory access remains read-only.
param([int]$GamePid=0,[string]$Session='',[switch]$Present,[switch]$SelfTest)
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
if (!$Present -or $GamePid -lt 1 -or $Session -notmatch '^R2-P1-[0-9]{8}-[0-9]{6}$') { throw 'Explicit user presence, session and game PID required' }
$root=Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$evidence=Join-Path $root "build\r2\live\$Session"
if (!(Test-Path -LiteralPath (Join-Path $evidence 'installed-receipt.json'))) { throw 'Prepare and deploy this session first' }
$image='D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe'
if ((Get-FileHash -LiteralPath $image).Hash -ne '8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8') { throw 'Build drift' }
$game=Get-Process -Id $GamePid
$birth=$game.StartTime.ToUniversalTime().ToFileTimeUtc()
$deadline=[DateTime]::FromFileTimeUtc($birth).AddSeconds(840)
$handle=[R2SessionDeadline]::OpenPinned($GamePid,$birth,$image)
$path=Join-Path $evidence 'watchdog.json'
$record=[ordered]@{state='watching';game_pid=$GamePid;game_start_filetime=$birth.ToString();game_image=$image;
    controller_pid=$PID;controller_start_filetime=(Get-Process -Id $PID).StartTime.ToUniversalTime().ToFileTimeUtc().ToString();
    deadline_utc=$deadline.ToString('o');max_game_age_seconds=840;helper_sha256=(Get-FileHash -LiteralPath $PSCommandPath).Hash}
try {
    $record | ConvertTo-Json | Set-Content -LiteralPath $path
    Write-Output "Watchdog armed for PID $GamePid; absolute deadline $($record.deadline_utc)"
    $forced=[R2SessionDeadline]::WaitUntil($handle,$deadline.ToFileTimeUtc())
    $record.state=if ($forced) {'deadline_stopped'} else {'exited'}
    $record['ended_utc']=[DateTime]::UtcNow.ToString('o')
    $record | ConvertTo-Json | Set-Content -LiteralPath $path
    Write-Output "Watchdog finished: $($record.state)"
} finally { [void][R2SessionDeadline]::CloseHandle($handle) }
