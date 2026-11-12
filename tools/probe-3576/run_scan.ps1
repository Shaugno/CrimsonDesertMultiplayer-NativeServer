# Compile and run the read-only 3576 local-to-world scanner.
# Does not attach to the game, edit offsets, or install a hook.
# Usage: powershell -NoProfile -File tools\probe-3576\run_scan.ps1

$ErrorActionPreference = 'Stop'
$repo = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$src = Join-Path $PSScriptRoot 'scan_local_to_world.cpp'
$exe = 'D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe'
$outDir = Join-Path $env:TEMP 'cdmp-3576-scan'
$outTxt = Join-Path $repo 'docs\probe-3576\local-to-world-scan.txt'
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$install = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $install) { throw 'Visual Studio C++ tools not found' }
$vcvars = Join-Path $install 'VC\Auxiliary\Build\vcvars64.bat'
$obj = Join-Path $outDir 'scan_local_to_world.obj'
$bin = Join-Path $outDir 'scan_local_to_world.exe'

$cmd = "call `"$vcvars`" >nul && cl /nologo /O2 /std:c++20 /EHsc /Fo`"$obj`" /Fe`"$bin`" `"$src`" && `"$bin`" `"$exe`""
cmd.exe /c $cmd | Tee-Object -FilePath $outTxt
if ($LASTEXITCODE -ne 0) { throw "scan failed with exit $LASTEXITCODE" }
Write-Output "wrote $outTxt"
