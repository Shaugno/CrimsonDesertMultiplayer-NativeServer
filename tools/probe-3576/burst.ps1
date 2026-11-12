# One read-only local/origin/static burst.
# Candidates only. This script does not edit offsets and does not install a hook.
# Usage:
#   powershell -NoProfile -File tools\probe-3576\burst.ps1 -Slot exe+0x71C09A8 -ProcessId 16920
# The slot is a temporary sampler. Pass the slot classified for this process.

param(
    [Parameter(Mandatory = $true)][string]$Slot,
    [Parameter(Mandatory = $true)][int]$ProcessId,
    [string]$Origin = 'exe+0x71AC2C0',
    [string]$Static = 'exe+0x71CF140'
)

$ErrorActionPreference = 'Stop'
$repo = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$probe = Join-Path $repo 'build\bin\RelWithDebInfo\cdmp_probe.exe'
if (-not (Test-Path -LiteralPath $probe)) { throw "probe not found: $probe" }

$inv = [Globalization.CultureInfo]::InvariantCulture
$log = New-Object System.Collections.Generic.List[string]

function Add-Line([string]$text) { $log.Add($text) }

function Write-Stamp([string]$name) {
    Add-Line ('--- {0} {1} ---' -f (Get-Date -Format 'yyyy-MM-dd HH:mm:ss.fff'), $name)
}

function Invoke-Probe([string[]]$ArgList) {
    $lines = @(& $probe --pid $ProcessId @ArgList | ForEach-Object { "$_" })
    $code = $LASTEXITCODE
    foreach ($line in $lines) { Add-Line $line }
    Add-Line ('exit {0}' -f $code)
    if ($code -ne 0) { throw "probe failed ($code): $($ArgList -join ' ')" }
    return ,$lines
}

function Read-Floats([string[]]$Lines) {
    $vals = New-Object System.Collections.Generic.List[double]
    foreach ($line in $Lines) {
        if ($line -match ':\s*([+-]?\d+(?:\.\d+)?)\s*$') {
            $vals.Add([double]::Parse($Matches[1], $inv))
        }
    }
    if ($vals.Count -ne 4) { throw "expected 4 floats, saw $($vals.Count)" }
    return $vals
}

function Format-Vec($vals) {
    $parts = @()
    foreach ($v in $vals) { $parts += $v.ToString('0.0000', $inv) }
    return '(' + ($parts -join ', ') + ')'
}

try {
    Add-Line 'OBSERVATION'
    Write-Stamp 'info'
    $info = Invoke-Probe @('info')
    $infoText = $info -join "`n"
    if ($infoText -notmatch 'PE timestamp 0x6AED2E85') { throw 'timestamp is not 0x6AED2E85' }
    if ($infoText -notmatch ('PID\s+' + $ProcessId)) { throw 'pid line does not match -ProcessId' }

    Write-Stamp 'chain'
    $chain = Invoke-Probe @('chain', $Slot, '0x50', '0x68', '0x40', '0x140', '0x2C0', '0x90', '--type', 'vec3')
    $local = $null
    foreach ($line in $chain) {
        if ($line -match '^=>\s+(0x[0-9A-Fa-f]+)\s+:') { $local = $Matches[1] }
    }
    if (-not $local) { throw 'chain did not print a final address' }

    Write-Stamp 'local-f32'
    $localA = Read-Floats (Invoke-Probe @('read', $local, '--type', 'f32', '--count', '4'))
    Write-Stamp 'origin-f32'
    $originV = Read-Floats (Invoke-Probe @('read', $Origin, '--type', 'f32', '--count', '4'))
    Write-Stamp 'static-f32'
    $staticV = Read-Floats (Invoke-Probe @('read', $Static, '--type', 'f32', '--count', '4'))
    Write-Stamp 'local-f32-again'
    $localB = Read-Floats (Invoke-Probe @('read', $local, '--type', 'f32', '--count', '4'))

    $sum = @()
    $residual = @()
    $bookend = @()
    for ($i = 0; $i -lt 4; $i++) {
        $sum += ($localA[$i] + $originV[$i])
        $residual += ($staticV[$i] - ($localA[$i] + $originV[$i]))
        $bookend += ($localB[$i] - $localA[$i])
    }

    Add-Line 'COMPUTED'
    Add-Line 'residual = static - (local + origin), using the first local read. No component was adjusted.'
    Add-Line ('slot {0}' -f $Slot)
    Add-Line ('local-field {0}' -f $local)
    Add-Line ('origin-candidate {0}' -f $Origin)
    Add-Line ('static-candidate {0}' -f $Static)
    Add-Line ('local {0}' -f (Format-Vec $localA))
    Add-Line ('origin {0}' -f (Format-Vec $originV))
    Add-Line ('static {0}' -f (Format-Vec $staticV))
    Add-Line ('local-plus-origin {0}' -f (Format-Vec $sum))
    Add-Line ('residual {0}' -f (Format-Vec $residual))
    Add-Line ('bookend-delta {0}' -f (Format-Vec $bookend))
    $still = $true
    for ($i = 0; $i -lt 3; $i++) {
        if ([math]::Abs($bookend[$i]) -gt 0.00005) { $still = $false }
    }
    if ($still) {
        Add-Line 'bookend: first and last local X Y Z match at the printed resolution'
    } else {
        Add-Line 'bookend: local changed during this burst; it is not a stationary sample'
    }
} finally {
    foreach ($line in $log) { Write-Output $line }
}
