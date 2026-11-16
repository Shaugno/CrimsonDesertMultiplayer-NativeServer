# Synthetic host/peer loopback only. Never launches or attaches to the game.
param([int]$Seconds = 8, [int]$Port = 39763, [string]$BuildDir = '')
$ErrorActionPreference = 'Stop'
if ($Seconds -lt 6 -or $Seconds -gt 60 -or $Port -lt 1024 -or $Port -gt 65535) { throw 'Invalid smoke parameters' }
if (Get-Process CrimsonDesert -ErrorAction SilentlyContinue) { throw 'Keep the game closed' }
if (Get-NetUDPEndpoint -LocalPort $Port -ErrorAction SilentlyContinue) { throw "UDP port $Port is already used" }
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$buildRoot = if (!$BuildDir) { Join-Path $root 'build' } elseif ([IO.Path]::IsPathRooted($BuildDir)) {
    [IO.Path]::GetFullPath($BuildDir)
} else { [IO.Path]::GetFullPath((Join-Path $root $BuildDir)) }
$binaryDir = Join-Path $buildRoot 'bin\RelWithDebInfo'
$run = Join-Path $buildRoot 'e2e'
New-Item -ItemType Directory -Force -Path $run | Out-Null
foreach ($name in @('cdmp.asi','cdmp_host.exe','cdmp_peer.exe')) {
    Copy-Item -LiteralPath (Join-Path $binaryDir $name) -Destination (Join-Path $run $name) -Force
}
$ini = @"
[general]
player_name=PortSmokeSynthetic
startup_delay_ms=0
[net]
enabled=1
transport=udp
role=guest
peer_host=127.0.0.1
peer_port=$Port
send_rate_hz=20
[overlay]
enabled=0
[sniffer]
enabled=0
[coop]
enabled=0
[session2]
enabled=0
"@
[IO.File]::WriteAllText((Join-Path $run 'cdmp.ini'), $ini, [Text.UTF8Encoding]::new($false))
$env:CDMP_ROLE = ''
$peer = Start-Process -FilePath (Join-Path $run 'cdmp_peer.exe') -ArgumentList @('--port', "$Port", '--mode', 'ghost') -WorkingDirectory $run -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $run 'peer.stdout.txt') -RedirectStandardError (Join-Path $run 'peer.stderr.txt')
try {
    & (Join-Path $run 'cdmp_host.exe') (Join-Path $run 'cdmp.asi') $Seconds
    if ($LASTEXITCODE) { throw "host exit $LASTEXITCODE" }
    $log = Get-Content -LiteralPath (Join-Path $run 'cdmp.log') -Raw
    if ($log -notmatch 'Connected|connected') { throw 'No connection in host log' }
    if ($log -notmatch 'synthetic') { throw 'No synthetic player-source evidence' }
    if ($log -notmatch 'remote states [1-9][0-9]*') { throw 'No received remote states' }
    if ($log -notmatch 'Network stopped' -or $log -notmatch 'Worker exiting') { throw 'Incomplete worker shutdown' }
    Write-Output 'Synthetic loopback complete; inspect cdmp.log for received states and clean shutdown.'
} finally {
    if (-not $peer.HasExited) { Stop-Process -Id $peer.Id }
}
