// Desktop-launched, narrowly scoped R5 process starter. No shell, terminal,
// arbitrary command execution, UI input or game-memory access. The deadline
// implementation follows this project's r5_session_watchdog.ps1.
using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Web.Script.Serialization;
using System.Windows.Forms;
using System.Drawing;

static class R5DesktopRunner {
 const string GameHash="8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8";
 static readonly JavaScriptSerializer Json=new JavaScriptSerializer{MaxJsonLength=8*1024*1024};
 static readonly string Self=Assembly.GetExecutingAssembly().Location;
 static readonly string Stem=Path.GetFileNameWithoutExtension(Self);
 [DllImport("kernel32.dll",SetLastError=true)]static extern IntPtr OpenProcess(uint rights,bool inherit,uint pid);
 [DllImport("kernel32.dll",SetLastError=true)]static extern bool GetProcessTimes(IntPtr h,out long birth,out long exit,out long kernel,out long user);
 [DllImport("kernel32.dll",CharSet=CharSet.Unicode,SetLastError=true)]static extern bool QueryFullProcessImageName(IntPtr h,uint flags,StringBuilder name,ref uint size);
 [DllImport("kernel32.dll")]static extern uint WaitForSingleObject(IntPtr h,uint milliseconds);
 [DllImport("kernel32.dll",SetLastError=true)]static extern bool TerminateProcess(IntPtr h,uint code);
 [DllImport("kernel32.dll")]static extern bool GetExitCodeProcess(IntPtr h,out uint code);
 [DllImport("kernel32.dll")]static extern bool CloseHandle(IntPtr h);
 static void Require(bool value,string message){if(!value)throw new InvalidOperationException(message);}
 static string Hash(string p){using(var s=File.OpenRead(p))using(var h=SHA256.Create())return BitConverter.ToString(h.ComputeHash(s)).Replace("-","");}
 static Dictionary<string,object> Read(string p){return Json.Deserialize<Dictionary<string,object>>(File.ReadAllText(p));}
 static string S(Dictionary<string,object> d,string k){return Convert.ToString(d[k],CultureInfo.InvariantCulture);}
 static long N(Dictionary<string,object> d,string k){return Convert.ToInt64(d[k],CultureInfo.InvariantCulture);}
 static bool B(Dictionary<string,object> d,string k){return Convert.ToBoolean(d[k],CultureInfo.InvariantCulture);}
 static void Write(string path,object value){Require(!File.Exists(path),"Receipt already exists");File.WriteAllText(path,Json.Serialize(value),new UTF8Encoding(false));}
 static string Under(string root,string relative){string r=Path.GetFullPath(root).TrimEnd('\\')+"\\",p=Path.GetFullPath(Path.Combine(root,relative));Require(p.StartsWith(r,StringComparison.OrdinalIgnoreCase),"Path escapes root");return p;}
 static IntPtr Pin(uint pid,long expectedBirth,string expectedImage){
  IntPtr h=OpenProcess(0x101001,false,pid);Require(h!=IntPtr.Zero,"Cannot pin process");
  try{long birth,exit,kernel,user;var b=new StringBuilder(1024);uint size=1024;
   Require(GetProcessTimes(h,out birth,out exit,out kernel,out user)&&birth==expectedBirth,"Process birth mismatch");
   Require(QueryFullProcessImageName(h,0,b,ref size)&&String.Equals(b.ToString(),expectedImage,StringComparison.OrdinalIgnoreCase),"Process image mismatch");return h;
  }catch{CloseHandle(h);throw;}
 }
 static bool Wait(IntPtr h,DateTime deadline){
  long ms=Math.Max(0,(long)(deadline-DateTime.UtcNow).TotalMilliseconds);uint r=WaitForSingleObject(h,(uint)Math.Min(ms,Int32.MaxValue));
  if(r==0)return false;Require(r==258,"Process wait failed");Require(TerminateProcess(h,0x3663),"Deadline stop failed");Require(WaitForSingleObject(h,10000)==0,"Deadline exit failed");return true;
 }
 static void Manifest(string root,string path){
  var lines=File.ReadAllLines(path);Require(lines.Length>1&&lines[0]=="CDMP-R4-SAVE-1","Manifest header");int count=0;
  foreach(var entry in Directory.GetFileSystemEntries(root,"*",SearchOption.AllDirectories))Require((File.GetAttributes(entry)&FileAttributes.ReparsePoint)==0,"Linked save entry");
  foreach(var line in lines){if(line==lines[0]||line.Length==0)continue;var parts=line.Split('\t');Require(parts.Length==3,"Manifest row");string file=Under(root,parts[2]);Require(new FileInfo(file).Length==Int64.Parse(parts[1],CultureInfo.InvariantCulture)&&Hash(file)==parts[0],"Frozen save copy changed");++count;}
  Require(Directory.GetFiles(root,"*",SearchOption.AllDirectories).Length==count,"Extra save files");
 }
 static void VisibleFixture(){
  Require(Process.GetProcessesByName("CrimsonDesert").Length==0,"Keep the game closed");
  Application.EnableVisualStyles();Application.SetCompatibleTextRenderingDefault(false);
  using(var a=new Form())using(var b=new Form()){
   a.Text="CDMP R5 fixture HOST";b.Text="CDMP R5 fixture GUEST";
   foreach(var f in new[]{a,b}){f.StartPosition=FormStartPosition.Manual;f.Size=new Size(430,220);f.ShowInTaskbar=true;var label=new Label{Dock=DockStyle.Fill,TextAlign=ContentAlignment.MiddleCenter,Text=f.Text+"\r\nGame-free desktop-launch visibility check\r\nCloses automatically in 45 seconds"};f.Controls.Add(label);}
   a.Location=new Point(100,120);b.Location=new Point(560,120);a.Show();b.Show();Application.DoEvents();
   var start=DateTime.UtcNow;Write(Path.Combine(Path.GetDirectoryName(Self),"visible-pair-start.json"),new{pid=Process.GetCurrentProcess().Id,session=Process.GetCurrentProcess().SessionId,host_hwnd=a.Handle.ToInt64(),guest_hwnd=b.Handle.ToInt64(),host_visible=a.Visible,guest_visible=b.Visible,started_utc=start.ToString("o"),game_launched=false});
   var t=Stopwatch.StartNew();while(t.Elapsed.TotalSeconds<45&&(!a.IsDisposed||!b.IsDisposed)){Application.DoEvents();Thread.Sleep(50);}
   Write(Path.Combine(Path.GetDirectoryName(Self),"visible-pair-end.json"),new{ended_utc=DateTime.UtcNow.ToString("o"),game_launched=false});
  }
 }
 static void Run(){
  string ownDir=Path.GetDirectoryName(Self),planPath=Path.Combine(ownDir,Stem+".plan.json");var p=Read(planPath);
  string role=S(p,"role"),root=Path.GetFullPath(S(p,"session_root")),image=Path.GetFullPath(S(p,"game_image")),repo=Path.GetFullPath(S(p,"repo_root"));
  Require((role=="host"||role=="guest")&&Stem=="r5_"+role+"_desktop","Role/filename mismatch");
  Require(String.Equals(root,ownDir,StringComparison.OrdinalIgnoreCase),"Plan/session root mismatch");
  Require(N(p,"version")==1&&B(p,"present"),"Presence/plan version missing");
  DateTime planned=DateTime.Parse(S(p,"planned_utc"),CultureInfo.InvariantCulture,DateTimeStyles.RoundtripKind).ToUniversalTime();
  Require(DateTime.UtcNow>=planned.AddSeconds(-5)&&DateTime.UtcNow<planned.AddMinutes(2),"Expired launch plan");
  Require(Path.GetFileName(image)=="CrimsonDesert.exe"&&Hash(image)==GameHash,"Executable mismatch");
  string arming=Path.GetFullPath(S(p,"arming_receipt"));Require(Hash(arming)==S(p,"arming_sha256"),"Arming receipt changed");var a=Read(arming);
  Require(N(a,"version")==2&&N(a,"round")==5&&N(a,"compiled_tier_mask")==15&&B(a,"full_suite_passed")&&B(a,"synthetic_v1_metadata_verified")&&S(a,"executable_sha256")==GameHash,"Unarmed build");
  Require(Hash(Self)==S(a,"desktop_runner_sha256"),"Desktop runner changed");
  foreach(Dictionary<string,object> f in (IEnumerable)a["source_manifest"])Require(Hash(Under(repo,S(f,"path")))==S(f,"sha256"),"Source seal changed");
  Require(Hash(S(a,"asi_path"))==S(a,"asi_sha256"),"ASI changed");
  var deployment=Read(Path.Combine(root,"deployment.json"));Require(S(deployment,"arming_sha256")==S(p,"arming_sha256"),"Deployment/arming mismatch");
  string gameDir=Path.GetDirectoryName(image);var allowed=new HashSet<string>{"cdmp.asi","cdmp.pdb","winmm.dll","cdmp.host.ini","cdmp.guest.ini","cdmp.r4-test-lock"};
  int installed=0;foreach(Dictionary<string,object> f in (IEnumerable)deployment["files"]){string name=S(f,"name");Require(allowed.Remove(name)&&Hash(Under(gameDir,name))==S(f,"installed_sha256"),"Installed file changed");++installed;}Require(installed==6&&allowed.Count==0,"Deployment file set");
  var session=Read(Path.Combine(root,"session.json"));Require(S(session,"root")==root&&S(session,"session_kind")==S(p,"session_kind"),"Session mismatch");
  string manifest=Path.Combine(root,"save-manifest.tsv");Require(Hash(manifest)==S(session,"manifest_sha256"),"Save manifest changed");
  string realRoot=Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),"Pearl Abyss\\CD\\save");
  Require(String.Equals(realRoot,S(session,"source"),StringComparison.OrdinalIgnoreCase),"Desktop user's real save root differs from prepared source");
  Manifest(Path.Combine(root,"host-backup"),manifest);Manifest(Path.Combine(root,"guest-backup"),manifest);Manifest(S(session,"guest_sandbox"),manifest);
  if(role=="host")Manifest(realRoot,manifest);
  string launchPath=Path.Combine(root,"launch-"+role+".json");Require(!File.Exists(launchPath),"Role already launched");
  int max=(int)N(p,"max_age_seconds");Require(max>=60&&max<=840,"Deadline bound");var games=Process.GetProcessesByName("CrimsonDesert");
  DateTime common=DateTime.MaxValue;
  if(role=="host")Require(games.Length==0,"Unexpected game inventory");
  else if(S(p,"session_kind")=="L3"){
   var host=Read(Path.Combine(root,"launch-host.json"));Require(games.Length==1&&games[0].Id==N(host,"game_pid"),"Expected exact host only");
   long birth=N(host,"game_start_filetime");IntPtr hostHandle=Pin((uint)N(host,"game_pid"),birth,S(host,"game_image"));CloseHandle(hostHandle);
   Require(DateTime.UtcNow<DateTime.FromFileTimeUtc(birth).AddMinutes(6),"Insufficient common deadline");
   string log;using(var stream=new FileStream(Path.Combine(gameDir,"cdmp.host.log"),FileMode.Open,FileAccess.Read,FileShare.ReadWrite|FileShare.Delete))using(var reader=new StreamReader(stream))log=reader.ReadToEnd();
   Require(log.Contains("(PID "+S(host,"game_pid")+")")&&log.Contains("3663 v1 host world ready"),"Host not world ready");common=DateTime.FromFileTimeUtc(birth).AddSeconds(N(host,"max_age_seconds"));
  }else{
   Require(games.Length==0,"Unexpected game inventory");
   if(role=="guest"&&S(p,"session_kind")=="L4Guest"){
    var host=Read(Path.Combine(root,"launch-host.json"));Require(S(host,"role")=="host"&&S(host,"session_kind")=="L4Host","Actual L4 host receipt required");
    common=DateTime.FromFileTimeUtc(N(host,"game_start_filetime")).AddSeconds(N(host,"max_age_seconds"));Require(common>DateTime.UtcNow.AddMinutes(3),"Insufficient L4 common deadline");
   }
  }
  var env=(Dictionary<string,object>)p["environment"];var names=new HashSet<string>{"SteamAppId","SteamGameId","CDMP_ROLE","CDMP_HOST_REAL_SAVES","CDMP_SAVE_SANDBOX","CDMP_SAVE_BACKUP","CDMP_SAVE_MANIFEST","CDMP_SAVE_MANIFEST_SHA256","CDMP_REAL_SAVE_ROOT","CDMP_SAVE_ISOLATION","CDMP_GUEST_WITH_HOST","USERPROFILE","LOCALAPPDATA"};
  foreach(string key in env.Keys)Require(names.Contains(key),"Unexpected environment key");
  Require(S(env,"SteamAppId")=="3321460"&&S(env,"SteamGameId")=="3321460"&&S(env,"CDMP_ROLE")==role,"Role/Steam environment mismatch");
  if(role=="host")Require(S(env,"CDMP_HOST_REAL_SAVES")=="1"&&!env.ContainsKey("CDMP_SAVE_SANDBOX"),"Host isolation forbidden");
  else Require(!env.ContainsKey("CDMP_HOST_REAL_SAVES")&&S(env,"CDMP_SAVE_SANDBOX")==S(session,"guest_sandbox")&&S(env,"CDMP_SAVE_BACKUP")==Path.Combine(root,"guest-backup")&&S(env,"CDMP_SAVE_MANIFEST")==manifest&&S(env,"CDMP_SAVE_MANIFEST_SHA256")==S(session,"manifest_sha256")&&S(env,"CDMP_SAVE_ISOLATION")=="profile"&&S(env,"USERPROFILE")==S(session,"guest_profile")&&S(env,"LOCALAPPDATA")==Path.Combine(S(session,"guest_profile"),"AppData\\Local")&&S(env,"CDMP_REAL_SAVE_ROOT")==S(session,"source")&&S(env,"CDMP_GUEST_WITH_HOST")==(S(p,"session_kind")=="L3"?"1":"0"),"Guest isolation environment mismatch");
  var psi=new ProcessStartInfo(image,"PlatformServiceType=Steam"){WorkingDirectory=gameDir,UseShellExecute=false,WindowStyle=ProcessWindowStyle.Normal};
  var inherited=new List<string>();foreach(string key in psi.EnvironmentVariables.Keys)if(key.StartsWith("CDMP_",StringComparison.OrdinalIgnoreCase))inherited.Add(key);foreach(string key in inherited)psi.EnvironmentVariables.Remove(key);
  foreach(var entry in env)psi.EnvironmentVariables[entry.Key]=Convert.ToString(entry.Value,CultureInfo.InvariantCulture);
  Process game=null;IntPtr pinned=IntPtr.Zero;bool completed=false;System.Threading.Timer windows=null;
  try{
   game=Process.Start(psi);long birth=game.StartTime.ToUniversalTime().ToFileTimeUtc();pinned=Pin((uint)game.Id,birth,image);
   DateTime deadline=DateTime.FromFileTimeUtc(birth).AddSeconds(max);if(common<deadline)deadline=common;
   Write(launchPath,new{role=role,game_pid=game.Id,game_start_filetime=birth.ToString(CultureInfo.InvariantCulture),game_image=image,max_age_seconds=max,isolation=role=="host"?"none":"profile",session_kind=S(p,"session_kind"),launched_utc=DateTime.UtcNow.ToString("o"),backend="desktop-app"});
   var watch=new Dictionary<string,object>{{"state","watching"},{"game_pid",game.Id},{"game_start_filetime",birth.ToString(CultureInfo.InvariantCulture)},{"game_image",image},{"controller_pid",Process.GetCurrentProcess().Id},{"controller_start_filetime",Process.GetCurrentProcess().StartTime.ToUniversalTime().ToFileTimeUtc().ToString(CultureInfo.InvariantCulture)},{"deadline_utc",deadline.ToString("o")},{"max_game_age_seconds",max},{"helper_sha256",Hash(Self)},{"backend","desktop-app"}};
   string watchPath=Path.Combine(root,"watchdog-"+role+".json");Write(watchPath,watch);
   windows=new System.Threading.Timer(delegate(object unused){try{if(WaitForSingleObject(pinned,0)==258)File.WriteAllText(Path.Combine(root,"window-"+role+".json"),Json.Serialize(new{game_pid=game.Id,game_start_filetime=birth.ToString(CultureInfo.InvariantCulture),windows=R5WindowReport.Read(new[]{game.Id}),recorded_utc=DateTime.UtcNow.ToString("o"),scope="read-only controlled process windows; no UI input"}));}catch{}},null,0,1000);
   bool forced=Wait(pinned,deadline);uint code;Require(GetExitCodeProcess(pinned,out code),"Exit code unavailable");watch["state"]=forced?"deadline_stopped":"exited";watch["game_exit_code"]=code;watch["ended_utc"]=DateTime.UtcNow.ToString("o");File.WriteAllText(watchPath,Json.Serialize(watch));completed=true;
  }finally{if(windows!=null){using(var done=new ManualResetEvent(false)){windows.Dispose(done);done.WaitOne(5000);}}if(!completed&&game!=null&&!game.HasExited){TerminateProcess(game.Handle,0x3663);game.WaitForExit(10000);}if(pinned!=IntPtr.Zero)CloseHandle(pinned);if(game!=null)game.Dispose();}
 }
 static void CapturePair(){
  string root=Path.GetDirectoryName(Self);var plan=Read(Path.Combine(root,Stem+".plan.json"));Require(B(plan,"present"),"Capture presence required");
  var ids=new int[2];var births=new long[2];int i=0;
  foreach(string role in new[]{"host","guest"}){
   var r=Read(Path.Combine(root,"launch-"+role+".json"));var w=Read(Path.Combine(root,"watchdog-"+role+".json"));Require(S(w,"state")=="watching","Game already exited");
   string image=S(r,"game_image");Require(Path.GetFileName(image)=="CrimsonDesert.exe"&&Hash(image)==GameHash,"Capture executable mismatch");
   ids[i]=(int)N(r,"game_pid");births[i]=N(r,"game_start_filetime");IntPtr h=Pin((uint)ids[i],births[i],image);CloseHandle(h);++i;
  }
  Require(ids[0]!=ids[1],"Duplicate capture roles");string path=Path.Combine(root,"l3-desktop-capture-"+DateTime.UtcNow.ToString("yyyyMMdd-HHmmss")+".avi");
  int[] seen=R4LocalCapture.Capture(path,ids,births);uint frames=R4LocalCapture.ReadFrames(path);bool complete=R4LocalCapture.Complete(frames,ids,seen);
  Write(path+".json",new{path=path,sha256=Hash(path),bytes=new FileInfo(path).Length,frames=frames,duration_seconds=10,fps=2,foreground_test_pids=seen,both_test_windows_seen=complete,scope="local-only foreground controlled games; no UI input or memory access",captured_utc=DateTime.UtcNow.ToString("o")});
  Require(complete,"Capture incomplete; metadata retained");
 }
 static void SelfTest(){
  foreach(string mode in new[]{"normal","deadline","mismatch"}){
   using(var child=Process.Start(new ProcessStartInfo(Self,"--child "+(mode=="normal"?"200":"30000")){UseShellExecute=false,WindowStyle=ProcessWindowStyle.Hidden})){
    IntPtr h=IntPtr.Zero;try{long birth=child.StartTime.ToUniversalTime().ToFileTimeUtc();
     if(mode=="mismatch"){bool refused=false;try{h=Pin((uint)child.Id,birth+1,Self);}catch(InvalidOperationException){refused=true;}Require(refused&&!child.HasExited,"Identity refusal failed");}
     else{h=Pin((uint)child.Id,birth,Self);bool forced=Wait(h,DateTime.UtcNow.AddSeconds(1));Require(forced==(mode=="deadline"),"Deadline result mismatch");}
    }finally{if(h!=IntPtr.Zero)CloseHandle(h);if(!child.HasExited){child.Kill();child.WaitForExit(5000);}}
   }
  }
  bool escaped=false;try{Under(Path.GetTempPath(),"..\\outside");}catch(InvalidOperationException){escaped=true;}Require(escaped,"Path escape accepted");
  string folder=Path.Combine(Path.GetTempPath(),"cdmp-r5-desktop-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(folder);
  try{
   string file=Path.Combine(folder,"save.bin"),manifest=Path.Combine(folder,"manifest.tsv"),tree=Path.Combine(folder,"tree");Directory.CreateDirectory(tree);file=Path.Combine(tree,"save.bin");File.WriteAllText(file,"owned fixture");
   File.WriteAllText(manifest,"CDMP-R4-SAVE-1\n"+Hash(file)+"\t"+new FileInfo(file).Length+"\tsave.bin\n");Manifest(tree,manifest);
   File.WriteAllText(file,"changed");bool refused=false;try{Manifest(tree,manifest);}catch(InvalidOperationException){refused=true;}Require(refused,"Mutated frozen save accepted");
  }finally{
   string resolved=Path.GetFullPath(folder),temp=Path.GetFullPath(Path.GetTempPath()).TrimEnd('\\')+"\\";
   Require(resolved.StartsWith(temp,StringComparison.OrdinalIgnoreCase)&&Path.GetFileName(resolved).StartsWith("cdmp-r5-desktop-",StringComparison.Ordinal),"Unsafe fixture cleanup");Directory.Delete(resolved,true);
  }
 }
 [STAThread]public static int Main(string[] args){
  try{if(args.Length==2&&args[0]=="--child"){int delay=Int32.Parse(args[1],CultureInfo.InvariantCulture);Require(delay>=0&&delay<=30000,"Fixture child duration bound");Thread.Sleep(delay);return 0;}
   if(args.Length==1&&args[0]=="--self-test"){SelfTest();return 0;}
   Require(args.Length==0,"No arbitrary command line accepted");
   if(Stem=="r5_visible_pair")VisibleFixture();else if(Stem=="r5_capture_pair")CapturePair();else Run();return 0;
  }catch(Exception e){try{File.WriteAllText(Path.Combine(Path.GetDirectoryName(Self),Stem+".error.json"),Json.Serialize(new{error=e.ToString(),utc=DateTime.UtcNow.ToString("o")}));}catch{}return 1;}
 }
}
