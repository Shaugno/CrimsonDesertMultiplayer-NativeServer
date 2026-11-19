# Read-only window diagnostics. Never moves, activates, renames or closes a
# game window. The optional visible fixture creates only its OWN two forms.
#requires -PSEdition Desktop
param([string]$SessionRoot='',[string]$Output='',[switch]$SelfTest,[switch]$VisibleFixture,[switch]$Present)
$ErrorActionPreference='Stop'
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
if(!('R5WindowReport' -as [type])){
Add-Type -ReferencedAssemblies System.Windows.Forms,System.Drawing -TypeDefinition @'
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;
using System.Drawing;
public static class R5WindowReport {
 public sealed class Row {
  public long hwnd,owner;public uint pid,thread;public string title,desktop;
  public bool visible,minimized,on_monitor,ordinary_visible_candidate;
  public uint cloaked;public int cloak_hresult,left,top,right,bottom;public long ex_style;
 }
 [StructLayout(LayoutKind.Sequential)]struct Rect {public int left,top,right,bottom;}
 delegate bool EnumProc(IntPtr w,IntPtr p);
 [DllImport("user32.dll")]static extern bool EnumWindows(EnumProc callback,IntPtr data);
 [DllImport("user32.dll")]static extern uint GetWindowThreadProcessId(IntPtr w,out uint pid);
 [DllImport("user32.dll")]static extern bool IsWindowVisible(IntPtr w);
 [DllImport("user32.dll")]static extern bool IsIconic(IntPtr w);
 [DllImport("user32.dll")]static extern bool GetWindowRect(IntPtr w,out Rect rect);
 [DllImport("user32.dll",CharSet=CharSet.Unicode)]static extern int GetWindowText(IntPtr w,StringBuilder text,int count);
 [DllImport("user32.dll",EntryPoint="GetWindowLongPtrW")]static extern IntPtr GetWindowLongPtr(IntPtr w,int index);
 [DllImport("user32.dll")]static extern IntPtr GetWindow(IntPtr w,uint command);
 [DllImport("user32.dll")]static extern IntPtr MonitorFromWindow(IntPtr w,uint flags);
 [DllImport("dwmapi.dll")]static extern int DwmGetWindowAttribute(IntPtr w,int attr,out uint value,int size);
 [DllImport("user32.dll")]static extern IntPtr GetForegroundWindow();
 [DllImport("user32.dll")]static extern IntPtr GetProcessWindowStation();
 [DllImport("user32.dll")]static extern IntPtr GetThreadDesktop(uint thread);
 [DllImport("kernel32.dll")]static extern uint GetCurrentThreadId();
 [DllImport("kernel32.dll")]public static extern uint WTSGetActiveConsoleSessionId();
 [DllImport("user32.dll",CharSet=CharSet.Unicode,SetLastError=true)]static extern bool GetUserObjectInformation(IntPtr h,int index,StringBuilder text,int bytes,out int needed);
 static string Name(IntPtr h){var b=new StringBuilder(1024);int n;return GetUserObjectInformation(h,2,b,b.Capacity*2,out n)?b.ToString():"unavailable:"+Marshal.GetLastWin32Error();}
 public static string Station(){return Name(GetProcessWindowStation());}
 public static string Desktop(){return Name(GetThreadDesktop(GetCurrentThreadId()));}
 public static uint ForegroundPid(){uint p;GetWindowThreadProcessId(GetForegroundWindow(),out p);return p;}
 public static Row[] Read(int[] ids){
  var wanted=new HashSet<int>(ids);var rows=new List<Row>();
  EnumProc callback=delegate(IntPtr w,IntPtr unused){
   uint p;uint t=GetWindowThreadProcessId(w,out p);if(!wanted.Contains((int)p))return true;
   var b=new StringBuilder(256);GetWindowText(w,b,b.Capacity);Rect r;bool rect=GetWindowRect(w,out r);
   uint cloak;int hr=DwmGetWindowAttribute(w,14,out cloak,4);
   var row=new Row{hwnd=w.ToInt64(),owner=GetWindow(w,4).ToInt64(),pid=p,thread=t,title=b.ToString(),desktop=Name(GetThreadDesktop(t)),
    visible=IsWindowVisible(w),minimized=IsIconic(w),cloaked=cloak,cloak_hresult=hr,
    ex_style=GetWindowLongPtr(w,-20).ToInt64(),on_monitor=rect && MonitorFromWindow(w,0)!=IntPtr.Zero,
    left=r.left,top=r.top,right=r.right,bottom=r.bottom};
   bool tool=(row.ex_style&0x80)!=0,app=(row.ex_style&0x40000)!=0;
   row.ordinary_visible_candidate=row.visible && !row.minimized && hr==0 && cloak==0 && row.on_monitor &&
    row.right>row.left && row.bottom>row.top && (!tool||app) && (row.owner==0||app);
   rows.Add(row);return true;
  };
  if(!EnumWindows(callback,IntPtr.Zero))throw new InvalidOperationException("Window enumeration failed");
  GC.KeepAlive(callback);return rows.ToArray();
 }
 static Form Make(string role,int x){
  var f=new Form();f.Text="CDMP R5 fixture "+role;f.StartPosition=FormStartPosition.Manual;
  f.Location=new Point(x,120);f.Size=new Size(430,220);f.ShowInTaskbar=true;
  var label=new Label();label.Dock=DockStyle.Fill;label.TextAlign=ContentAlignment.MiddleCenter;
  label.Text="Game-free "+role+" visibility fixture\r\nNo Steam or game is launched.\r\nBoth fixture windows close automatically in 45 seconds.";
  f.Controls.Add(label);return f;
 }
 public static Row[] Fixture(bool visible){
  using(var a=Make("HOST",100))using(var b=Make("GUEST",560)){
   var handles=new HashSet<long>{a.Handle.ToInt64(),b.Handle.ToInt64()};
   if(visible){a.Show();b.Show();Application.DoEvents();}
   var rows=new List<Row>();foreach(var r in Read(new int[]{Process.GetCurrentProcess().Id}))if(handles.Contains(r.hwnd))rows.Add(r);
   if(rows.Count!=2)throw new InvalidOperationException("Owned two-window fixture enumeration failed");
   foreach(var r in rows)if(r.visible!=visible || (!visible && r.ordinary_visible_candidate))throw new InvalidOperationException("Fixture visibility mismatch");
   if(visible){var timer=Stopwatch.StartNew();while(timer.Elapsed.TotalSeconds<45 && (!a.IsDisposed||!b.IsDisposed)){Application.DoEvents();System.Threading.Thread.Sleep(50);}}
   return rows.ToArray();
  }
 }
}
'@
}
function Context {
    @{tool_pid=$PID;tool_session=(Get-Process -Id $PID).SessionId;active_console_session=[R5WindowReport]::WTSGetActiveConsoleSessionId();
      window_station=[R5WindowReport]::Station();thread_desktop=[R5WindowReport]::Desktop();foreground_pid=[R5WindowReport]::ForegroundPid()}
}
if($SelfTest){
    if($VisibleFixture -or $SessionRoot){throw 'Separate fixture/report modes'}
    $rows=@([R5WindowReport]::Fixture($false));if($rows.Count -ne 2){throw 'Expected two owned hidden fixture windows'}
    Write-Output 'R5 window report: two owned hidden forms, PID filter, visibility/candidate checks PASS; no game or desktop capture'
    return
}
if($VisibleFixture){
    if(!$Present -or !$Output -or $SessionRoot){throw 'Visible fixture requires explicit presence/output and no game session'}
    if(@(Get-CimInstance Win32_Process -Filter "Name='CrimsonDesert.exe'").Count){throw 'Keep the game closed for this game-free fixture'}
    $context=Context
    $rows=@([R5WindowReport]::Fixture($true))
    $result=@{kind='owned_visible_pair';context=$context;windows=$rows;seconds=45;game_launched=$false;other_windows_modified=$false;
      limitation='OS window flags are not user confirmation of seeing both forms';recorded_utc=[DateTime]::UtcNow.ToString('o')}
}else{
    if(!$SessionRoot -or !$Output){throw 'Session root/output required'}
    $ids=@();$roles=@()
    foreach($role in @('host','guest')){
        $record=Get-Content -LiteralPath (Join-Path $SessionRoot ("launch-$role.json")) -Raw | ConvertFrom-Json
        $process=Get-Process -Id $record.game_pid
        if($process.ProcessName -ne 'CrimsonDesert' -or $process.Path -ne $record.game_image -or
            $process.StartTime.ToUniversalTime().ToFileTimeUtc().ToString() -ne $record.game_start_filetime){throw 'Controlled game process identity changed'}
        if((Get-FileHash -LiteralPath $process.Path).Hash -ne 'DFCF5C507D09AA0E09CF9766DB866FD1E7CF513B9DD7FDC72A8F121BA2D9A9D5'){throw 'Build drift'}  # 1.0.0.3732 (R10)
        $ids+=[int]$process.Id;$roles+=@{role=$role;pid=$process.Id;session=$process.SessionId;birth=$record.game_start_filetime}
    }
    if($ids[0] -eq $ids[1]){throw 'Roles must be distinct processes'}
    $rows=@([R5WindowReport]::Read([int[]]$ids));$missing=@($ids | Where-Object {$r5Id=$_;!@($rows | Where-Object {$_.pid -eq $r5Id -and $_.ordinary_visible_candidate}).Count})
    $result=@{kind='controlled_pair_read_only';context=(Context);roles=$roles;windows=$rows;both_ordinary_visible_candidates=($missing.Count -eq 0);
      no_ordinary_window_pids=$missing;other_windows_modified=$false;memory_access=$false;recorded_utc=[DateTime]::UtcNow.ToString('o');
      limitation='OS window flags do not prove user visibility, foreground gameplay or an Alt+Tab entry'}
}
if(Test-Path -LiteralPath $Output){throw 'Do not overwrite window evidence'}
$result | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $Output -Encoding UTF8
$result | ConvertTo-Json -Depth 8
