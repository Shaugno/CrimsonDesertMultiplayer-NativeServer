param([Parameter(Mandatory=$true)][string]$Output)
$ErrorActionPreference='Stop'
$r5Repo=Split-Path $PSScriptRoot -Parent
$Output=[IO.Path]::GetFullPath($Output)
$r5Generated=Join-Path (Split-Path $Output -Parent) 'r5-desktop-src'
New-Item -ItemType Directory -Path $r5Generated -Force | Out-Null
$r5Sources=@(Join-Path $r5Repo 'tools\port3663\R5DesktopRunner.cs')
foreach($r5Name in @('r4_capture','r5_window_report')){
    $r5Text=[IO.File]::ReadAllText((Join-Path $r5Repo ("tools\port3663\$r5Name.ps1")))
    $r5Match=[regex]::Match($r5Text,"(?s)@'\r?\n(.*?)\r?\n'@")
    if(!$r5Match.Success){throw 'Embedded owned helper source not found'}
    $r5Source=Join-Path $r5Generated ($r5Name+'.cs')
    [IO.File]::WriteAllText($r5Source,$r5Match.Groups[1].Value,[Text.UTF8Encoding]::new($false));$r5Sources+=$r5Source
}
$r5Compiler=Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
& $r5Compiler /nologo /target:winexe /platform:x64 /r:System.Core.dll /r:System.Windows.Forms.dll /r:System.Drawing.dll /r:System.Web.Extensions.dll ("/out:"+$Output) @r5Sources
if($LASTEXITCODE){throw "Desktop runner compile failed: $LASTEXITCODE"}
