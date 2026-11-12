# Compile and run the read-only 3576 client-packet scanner.
# Does not attach to the game, edit offsets, or install a hook.
# Usage: powershell -NoProfile -File tools\probe-3576\run_packet_scan.ps1

$ErrorActionPreference = 'Stop'
Import-Module Microsoft.PowerShell.Utility
$repo = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$src = Join-Path $PSScriptRoot 'scan_packets.cpp'
$exe = 'D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe'
$expected = 'C94AD6C3EA54970A6D2DA7A2719753210A7728A91698ED3405DD9125470ED98F'
$outDir = Join-Path $env:TEMP 'cdmp-3576-scan'
$outTxt = Join-Path $repo 'docs\probe-3576\packet-scan-3576.txt'
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

if (-not (Test-Path -LiteralPath $exe)) { throw "game exe missing: $exe" }
$hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $exe).Hash
if ($hash -ne $expected) { throw "sha256 refused: $hash" }

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$install = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $install) { throw 'Visual Studio C++ tools not found' }
$vcvars = Join-Path $install 'VC\Auxiliary\Build\vcvars64.bat'
$obj = Join-Path $outDir 'scan_packets.obj'
$bin = Join-Path $outDir 'scan_packets.exe'

$cmd = "call `"$vcvars`" >nul && cl /nologo /O2 /std:c++20 /EHsc /Fo`"$obj`" /Fe`"$bin`" `"$src`" && `"$bin`" `"$exe`""
$lines = cmd.exe /c $cmd
$lines | Write-Output
if ($LASTEXITCODE -ne 0) { throw "scan failed with exit $LASTEXITCODE" }
$utf8 = New-Object System.Text.UTF8Encoding $false
[System.IO.File]::WriteAllLines($outTxt, @($lines + "sha256=$hash"), $utf8)
Write-Output "wrote $outTxt"
