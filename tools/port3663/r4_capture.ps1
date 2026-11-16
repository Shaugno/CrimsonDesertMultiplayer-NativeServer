# Short local-only capture of the foreground TEST GAME window. No UI control.
# Uncompressed AVI avoids installing another runtime or uploading screenshots.
#requires -PSEdition Desktop
param([string]$SessionRoot='',[switch]$Present,[switch]$SelfTest)
$ErrorActionPreference='Stop'
Add-Type -AssemblyName System.Drawing
if(!('R4LocalCapture' -as [type])){
Add-Type -ReferencedAssemblies System.Drawing -TypeDefinition @'
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
public sealed class R4LocalCapture : IDisposable {
 const int W=1280,H=720,Bytes=W*H*3;
 readonly BinaryWriter writer;
 readonly List<uint> offsets=new List<uint>();
 readonly long totalAt,lengthAt,moviAt;
 public int Frames {get {return offsets.Count;}}
 [StructLayout(LayoutKind.Sequential)] struct Rect {public int Left,Top,Right,Bottom;}
 [DllImport("user32.dll")] static extern IntPtr GetForegroundWindow();
 [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr h,out uint pid);
 [DllImport("user32.dll")] static extern bool GetWindowRect(IntPtr h,out Rect r);
 void Id(string value){writer.Write(Encoding.ASCII.GetBytes(value));}
 void Patch(long at,uint value){long end=writer.BaseStream.Position;writer.BaseStream.Position=at;writer.Write(value);writer.BaseStream.Position=end;}
 public R4LocalCapture(string path){
  writer=new BinaryWriter(new FileStream(path,FileMode.CreateNew,FileAccess.ReadWrite,FileShare.Read));
  Id("RIFF");writer.Write(0u);Id("AVI ");
  long hdrl=writer.BaseStream.Position;Id("LIST");writer.Write(0u);Id("hdrl");
  Id("avih");writer.Write(56u);writer.Write(500000u);writer.Write((uint)(Bytes*2));writer.Write(0u);writer.Write(16u);
  totalAt=writer.BaseStream.Position;writer.Write(0u);writer.Write(0u);writer.Write(1u);writer.Write((uint)Bytes);writer.Write(W);writer.Write(H);
  for(int i=0;i<4;++i)writer.Write(0u);
  long strl=writer.BaseStream.Position;Id("LIST");writer.Write(0u);Id("strl");
  Id("strh");writer.Write(56u);Id("vids");Id("DIB ");writer.Write(0u);writer.Write((ushort)0);writer.Write((ushort)0);
  writer.Write(0u);writer.Write(1u);writer.Write(2u);writer.Write(0u);lengthAt=writer.BaseStream.Position;writer.Write(0u);
  writer.Write((uint)Bytes);writer.Write(UInt32.MaxValue);writer.Write(0u);writer.Write((short)0);writer.Write((short)0);writer.Write((short)W);writer.Write((short)H);
  Id("strf");writer.Write(40u);writer.Write(40u);writer.Write(W);writer.Write(H);writer.Write((ushort)1);writer.Write((ushort)24);
  writer.Write(0u);writer.Write((uint)Bytes);for(int i=0;i<4;++i)writer.Write(0u);
  Patch(strl+4,(uint)(writer.BaseStream.Position-strl-8));Patch(hdrl+4,(uint)(writer.BaseStream.Position-hdrl-8));
  moviAt=writer.BaseStream.Position;Id("LIST");writer.Write(0u);Id("movi");
 }
 public void Frame(Bitmap image){
  offsets.Add((uint)(writer.BaseStream.Position-(moviAt+8)));Id("00db");writer.Write((uint)Bytes);
  BitmapData data=image.LockBits(new Rectangle(0,0,W,H),ImageLockMode.ReadOnly,PixelFormat.Format24bppRgb);
  try{byte[] row=new byte[W*3];for(int y=H-1;y>=0;--y){Marshal.Copy(IntPtr.Add(data.Scan0,y*data.Stride),row,0,row.Length);writer.Write(row);}}
  finally{image.UnlockBits(data);}
 }
 public void Dispose(){
  Patch(moviAt+4,(uint)(writer.BaseStream.Position-moviAt-8));Id("idx1");writer.Write((uint)(offsets.Count*16));
  foreach(uint offset in offsets){Id("00db");writer.Write(16u);writer.Write(offset);writer.Write((uint)Bytes);}
  Patch(4,(uint)(writer.BaseStream.Length-8));Patch(totalAt,(uint)offsets.Count);Patch(lengthAt,(uint)offsets.Count);writer.Dispose();
 }
 public static int[] Capture(string path,int[] pids,long[] births){
  var seen=new HashSet<int>();var timer=Stopwatch.StartNew();
  using(var output=new R4LocalCapture(path)){
   while(timer.ElapsedMilliseconds<10000){
    long started=timer.ElapsedMilliseconds;IntPtr window=GetForegroundWindow();uint pid;GetWindowThreadProcessId(window,out pid);
    int index=Array.IndexOf(pids,(int)pid);Rect rect;
    bool valid=false;
    if(index>=0)try{using(Process process=Process.GetProcessById((int)pid)){valid=process.StartTime.ToUniversalTime().ToFileTimeUtc()==births[index];}}catch{valid=false;}
    if(valid && GetWindowRect(window,out rect)){
     int width=rect.Right-rect.Left,height=rect.Bottom-rect.Top;
     if(width>=640 && height>=360 && width<=8192 && height<=8192){
      using(var screen=new Bitmap(width,height,PixelFormat.Format24bppRgb))
      using(var scaled=new Bitmap(W,H,PixelFormat.Format24bppRgb)){
       using(Graphics g=Graphics.FromImage(screen)){g.CopyFromScreen(rect.Left,rect.Top,0,0,screen.Size,CopyPixelOperation.SourceCopy);}
       using(Graphics g=Graphics.FromImage(scaled)){
        g.Clear(Color.Black);double scale=Math.Min((double)W/width,(double)H/height);int w=(int)(width*scale),h=(int)(height*scale);
        g.DrawImage(screen,new Rectangle((W-w)/2,(H-h)/2,w,h));
       }
       output.Frame(scaled);seen.Add((int)pid);
      }
     }
    }
    int left=(int)(500-(timer.ElapsedMilliseconds-started));if(left>0)Thread.Sleep(left);
   }
  }
  int[] result=new int[seen.Count];seen.CopyTo(result);return result;
 }
 public static void Fixture(string path){
  using(var output=new R4LocalCapture(path))using(var image=new Bitmap(W,H,PixelFormat.Format24bppRgb)){
   using(Graphics g=Graphics.FromImage(image)){g.Clear(Color.Blue);g.FillRectangle(Brushes.Red,0,0,W,H/2);}
   output.Frame(image);output.Frame(image);
  }
  using(var r=new BinaryReader(File.OpenRead(path))){
   if(Encoding.ASCII.GetString(r.ReadBytes(4))!="RIFF" || r.ReadUInt32()!=r.BaseStream.Length-8)throw new Exception("AVI RIFF size");
   r.BaseStream.Position=48;if(r.ReadUInt32()!=2)throw new Exception("AVI frame count");
   r.BaseStream.Position=r.BaseStream.Length-40;
   if(Encoding.ASCII.GetString(r.ReadBytes(4))!="idx1" || r.ReadUInt32()!=32)throw new Exception("AVI index");
  }
 }
}
'@
}
if($SelfTest){
    $fixture=Join-Path ([IO.Path]::GetTempPath()) ('cdmp-r4-capture-'+[guid]::NewGuid().ToString('N')+'.avi')
    try{[R4LocalCapture]::Fixture($fixture);Write-Output 'R4 capture fixture: two owned synthetic frames, AVI sizes/count/index PASS; no desktop read'}
    finally{if(Test-Path -LiteralPath $fixture){Remove-Item -LiteralPath $fixture -Force}}
    return
}
if(!$Present -or !$SessionRoot){throw 'Explicit live presence and controlled session required'}
$SessionRoot=[IO.Path]::GetFullPath($SessionRoot)
$ids=@();$births=@()
foreach($role in 'host','guest'){
    $record=Get-Content -LiteralPath (Join-Path $SessionRoot ("launch-$role.json")) -Raw | ConvertFrom-Json
    $process=Get-Process -Id $record.game_pid
    if($process.Path -ne $record.game_image -or $process.ProcessName -ne 'CrimsonDesert' -or
        $process.StartTime.ToUniversalTime().ToFileTimeUtc().ToString() -ne $record.game_start_filetime){throw 'Capture process identity mismatch'}
    $ids+=[int]$record.game_pid;$births+=[long]$record.game_start_filetime
}
$path=Join-Path $SessionRoot ('l3-capture-'+[DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss')+'.avi')
$seen=@([R4LocalCapture]::Capture($path,[int[]]$ids,[long[]]$births))
$result=@{path=$path;sha256=(Get-FileHash -LiteralPath $path).Hash;bytes=(Get-Item -LiteralPath $path).Length;
    duration_seconds=10;fps=2;foreground_test_pids=$seen;both_test_windows_seen=($ids.Count -eq 2 -and @($ids | Where-Object {$_ -notin $seen}).Count -eq 0);
    scope='local-only foreground game windows; no UI control; no native memory access';captured_utc=[DateTime]::UtcNow.ToString('o')}
$result | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath ($path+'.json') -Encoding UTF8
$result | ConvertTo-Json -Depth 4
