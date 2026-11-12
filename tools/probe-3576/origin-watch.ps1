# Bounded read-only watch of the origin candidate while the character travels.
# Candidates only. This script does not edit offsets and does not install a hook.
# Stdout is one terminal line: ACTION_REQUIRED, DONE, or FAILED.
# Every sample goes to the log file.
#
# Usage:
#   powershell -NoProfile -File tools\probe-3576\origin-watch.ps1 -Slot exe+0x71C09A8 -ProcessId 16920 -Seconds 180

param(
    [Parameter(Mandatory = $true)][string]$Slot,
    [Parameter(Mandatory = $true)][int]$ProcessId,
    [int]$Seconds = 180,
    [string]$OriginAddr = 'exe+0x71AC2C0',
    [string]$StaticAddr = 'exe+0x71CF140'
)

$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'
$repo = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$probe = Join-Path $repo 'build\bin\RelWithDebInfo\cdmp_probe.exe'
$inv = [Globalization.CultureInfo]::InvariantCulture
$writer = $null
$terminal = $null

function Format-Vec($vals) {
    if ($null -eq $vals -or $vals -is [string] -or $vals.Count -ne 4) { return 'unreadable' }
    $parts = @()
    for ($i = 0; $i -lt 4; $i++) { $parts += [string]::Format($inv, '{0:0.0000}', [double]$vals[$i]) }
    return '(' + ($parts -join ', ') + ')'
}

function Vec-Ok($vals) {
    return ($null -ne $vals -and -not ($vals -is [string]) -and $vals.Count -eq 4)
}

function Add-Vec($a, $b) {
    if (-not (Vec-Ok $a) -or -not (Vec-Ok $b)) { return $null }
    $out = New-Object 'double[]' 4
    for ($i = 0; $i -lt 4; $i++) { $out[$i] = [double]$a[$i] + [double]$b[$i] }
    return ,$out
}

function Sub-Vec($a, $b) {
    if (-not (Vec-Ok $a) -or -not (Vec-Ok $b)) { return $null }
    $out = New-Object 'double[]' 4
    for ($i = 0; $i -lt 4; $i++) { $out[$i] = [double]$a[$i] - [double]$b[$i] }
    return ,$out
}

function Xyz-Distance($delta) {
    if ($null -eq $delta) { return 'unreadable' }
    $d = [math]::Sqrt(($delta[0] * $delta[0]) + ($delta[1] * $delta[1]) + ($delta[2] * $delta[2]))
    return [string]::Format($inv, '{0:0.0000}', $d)
}

function Origin-Changed($baseline, $sample) {
    if ($null -eq $baseline -or $null -eq $sample) { return $false }
    for ($i = 0; $i -lt 4; $i++) {
        if ([math]::Abs($baseline[$i] - $sample[$i]) -gt 0.05) { return $true }
    }
    return $false
}

function Invoke-Probe([string[]]$ArgList) {
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $probe
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.CreateNoWindow = $true
    $quoted = @('--pid', "$ProcessId") + $ArgList | ForEach-Object {
        if ($_ -match '[\s"]') { '"' + ($_ -replace '"', '\"') + '"' } else { $_ }
    }
    $psi.Arguments = $quoted -join ' '
    $proc = New-Object System.Diagnostics.Process
    $proc.StartInfo = $psi
    [void]$proc.Start()
    $outTask = $proc.StandardOutput.ReadToEndAsync()
    $errTask = $proc.StandardError.ReadToEndAsync()
    if (-not $proc.WaitForExit(8000)) {
        try { $proc.Kill() } catch {}
        return @{ Code = 124; Lines = @('timeout') }
    }
    $text = @($outTask.Result, $errTask.Result) -join "`n"
    $lines = @($text -split "`r?`n" | Where-Object { $_ -ne '' })
    return @{ Code = $proc.ExitCode; Lines = $lines }
}

function Read-Floats([string[]]$Lines) {
    $vals = New-Object System.Collections.Generic.List[double]
    foreach ($line in $Lines) {
        if ($line -match ':\s*([+-]?\d+(?:\.\d+)?)\s*$') {
            [void]$vals.Add([double]::Parse($Matches[1], $inv))
        }
    }
    if ($vals.Count -ne 4) { return $null }
    return ,$vals
}

function Read-OriginOrStatic([string]$Addr) {
    $hit = Invoke-Probe @('read', $Addr, '--type', 'f32', '--count', '4')
    if ($hit.Code -ne 0) { return $null }
    return Read-Floats $hit.Lines
}

function Read-Local {
    $chain = Invoke-Probe @('chain', $Slot, '0x50', '0x68', '0x40', '0x140', '0x2C0', '0x90', '--type', 'vec3')
    $field = $null
    $hops = New-Object System.Collections.Generic.List[string]
    foreach ($line in $chain.Lines) {
        [void]$hops.Add($line)
        if ($line -match '^=>\s+(0x[0-9A-Fa-f]+)\s+:') { $field = $Matches[1] }
    }
    if ($chain.Code -ne 0 -or -not $field) {
        return @{ Field = $null; Values = $null; Hops = $hops; Broken = $true }
    }
    $read = Invoke-Probe @('read', $field, '--type', 'f32', '--count', '4')
    $values = $null
    if ($read.Code -eq 0) { $values = Read-Floats $read.Lines }
    return @{ Field = $field; Values = $values; Hops = $hops; Broken = ($null -eq $values) }
}

try {
    if (-not (Test-Path -LiteralPath $probe)) { throw "probe not found: $probe" }
    if ($Seconds -lt 1 -or $Seconds -gt 600) { throw "seconds out of range: $Seconds" }
    $procInfo = Get-Process -Id $ProcessId -ErrorAction SilentlyContinue
    if (-not $procInfo -or $procInfo.ProcessName -ne 'CrimsonDesert') {
        throw "pid $ProcessId is not CrimsonDesert"
    }

    $logPath = Join-Path $repo ("docs\probe-3576\origin-watch-{0}-{1}.log" -f $ProcessId, (Get-Date -Format 'HHmmss'))
    $writer = New-Object System.IO.StreamWriter($logPath, $false, [Text.UTF8Encoding]::new($false))
    $writer.AutoFlush = $true
    $writer.WriteLine('OBSERVATION')
    $writer.WriteLine('Candidates only. Residual = static - (local + origin). No component was adjusted.')
    $writer.WriteLine(('window-seconds {0}' -f $Seconds))
    $writer.WriteLine(('change-threshold 0.05 on any origin component versus the first sample'))
    $writer.WriteLine(('slot {0}' -f $Slot))
    $writer.WriteLine(('origin-candidate {0}' -f $OriginAddr))
    $writer.WriteLine(('static-candidate {0}' -f $StaticAddr))
    $writer.WriteLine(('pid {0} start {1}' -f $ProcessId, $procInfo.StartTime.ToString('yyyy-MM-dd HH:mm:ss', $inv)))

    $info = Invoke-Probe @('info')
    foreach ($line in $info.Lines) { $writer.WriteLine($line) }
    $writer.WriteLine(('info-exit {0}' -f $info.Code))
    $infoText = $info.Lines -join "`n"
    if ($info.Code -ne 0) { throw 'probe info failed' }
    if ($infoText -notmatch 'PE timestamp 0x6AED2E85') { throw 'timestamp is not 0x6AED2E85' }
    if ($infoText -notmatch ('PID\s+' + $ProcessId)) { throw 'pid line does not match -ProcessId' }

    $deadline = (Get-Date).AddSeconds($Seconds)
    $writer.WriteLine(('deadline {0}' -f $deadline.ToString('yyyy-MM-dd HH:mm:ss.fff', $inv)))
    $baseline = $null
    $previous = $null
    $lastField = $null
    $samples = 0
    $originFails = 0
    $changed = $false
    $tail = 0

    while ($true) {
        $past = (Get-Date) -gt $deadline
        if ($past -and -not $changed) { break }
        if ($changed -and $tail -ge 2) { break }
        $alreadyChanged = $changed

        $stamp = Get-Date
        $originA = Read-OriginOrStatic $OriginAddr
        $local = Read-Local
        $staticVec = Read-OriginOrStatic $StaticAddr
        $originB = Read-OriginOrStatic $OriginAddr
        $samples++

        if ($null -eq $originA -and $null -eq $originB) {
            $originFails++
            $writer.WriteLine(('{0} SAMPLE origin-unreadable consecutive-fails {1}' -f $stamp.ToString('yyyy-MM-dd HH:mm:ss.fff', $inv), $originFails))
            if ($originFails -ge 3) { throw 'origin candidate unreadable three times' }
            continue
        }
        $originFails = 0
        $originNow = $originB
        if ($null -eq $originNow) { $originNow = $originA }
        $mid = ($null -ne $originA -and $null -ne $originB -and (Origin-Changed $originA $originB))
        $world = Add-Vec $local.Values $originNow
        $residual = Sub-Vec $staticVec $world

        if ($local.Field -and $local.Field -ne $lastField) {
            $writer.WriteLine(('CHAIN {0} local-field {1}' -f $stamp.ToString('yyyy-MM-dd HH:mm:ss.fff', $inv), $local.Field))
            foreach ($hop in $local.Hops) { $writer.WriteLine($hop) }
            $lastField = $local.Field
        } elseif ($local.Broken) {
            $writer.WriteLine(('CHAIN {0} broken' -f $stamp.ToString('yyyy-MM-dd HH:mm:ss.fff', $inv)))
            foreach ($hop in $local.Hops) { $writer.WriteLine($hop) }
        }

        $writer.WriteLine((
            '{0} SAMPLE local {1} origin-a {2} origin-b {3} static {4} world {5} residual {6} local-field {7}{8}' -f `
                $stamp.ToString('yyyy-MM-dd HH:mm:ss.fff', $inv),
                (Format-Vec $local.Values),
                (Format-Vec $originA),
                (Format-Vec $originB),
                (Format-Vec $staticVec),
                (Format-Vec $world),
                (Format-Vec $residual),
                $(if ($local.Field) { $local.Field } else { 'unreadable' }),
                $(if ($mid) { ' origin-changed-within-sample' } else { '' })
        ))

        $sample = @{
            Origin = $originNow
            Local = $local.Values
            World = $world
            Residual = $residual
            Static = $staticVec
            Field = $local.Field
        }

        $thisChanged = $false
        if ($null -eq $baseline) {
            $baseline = $originNow
        } elseif ((Origin-Changed $baseline $originNow) -or $mid) {
            $thisChanged = $true
        }

        if ($thisChanged -and -not $changed) {
            $changed = $true
            $originDelta = Sub-Vec $originNow $baseline
            $worldDelta = $null
            $localDelta = $null
            if ($previous) {
                $worldDelta = Sub-Vec $world $previous.World
                $localDelta = Sub-Vec $local.Values $previous.Local
            }
            $writer.WriteLine('COMPUTED first sample whose origin differs from the first sample. No component was adjusted.')
            $writer.WriteLine(('before-origin {0}' -f (Format-Vec $baseline)))
            $writer.WriteLine(('after-origin {0}' -f (Format-Vec $originNow)))
            $writer.WriteLine(('origin-delta {0}' -f (Format-Vec $originDelta)))
            $writer.WriteLine(('before-world {0}' -f $(if ($previous) { Format-Vec $previous.World } else { 'none' })))
            $writer.WriteLine(('after-world {0}' -f (Format-Vec $world)))
            $writer.WriteLine(('world-delta {0} distance {1}' -f (Format-Vec $worldDelta), (Xyz-Distance $worldDelta)))
            $writer.WriteLine(('local-delta {0} distance {1}' -f (Format-Vec $localDelta), (Xyz-Distance $localDelta)))
            $writer.WriteLine(('residual-after {0}' -f (Format-Vec $residual)))
            $script:changeSummary = @{
                BeforeOrigin = Format-Vec $baseline
                AfterOrigin = Format-Vec $originNow
                OriginDelta = Format-Vec $originDelta
                WorldDelta = Format-Vec $worldDelta
                WorldDistance = Xyz-Distance $worldDelta
                Residual = Format-Vec $residual
            }
        }

        $previous = $sample
        if ($alreadyChanged) { $tail++ }
        $elapsedMs = ((Get-Date) - $stamp).TotalMilliseconds
        if ($elapsedMs -lt 1000) { Start-Sleep -Milliseconds ([int](1000 - $elapsedMs)) }
    }

    if ($changed) {
        $terminal = ('ACTION_REQUIRED: Stop and stand still. Origin candidate changed. before-origin={0} after-origin={1} origin-delta={2} world-delta={3} world-distance={4} residual-after={5} samples={6} log={7}' -f `
            $script:changeSummary.BeforeOrigin,
            $script:changeSummary.AfterOrigin,
            $script:changeSummary.OriginDelta,
            $script:changeSummary.WorldDelta,
            $script:changeSummary.WorldDistance,
            $script:changeSummary.Residual,
            $samples,
            $logPath)
    } else {
        $terminal = ('DONE: no origin change within {0}s. samples={1} origin={2} log={3}' -f `
            $Seconds, $samples, (Format-Vec $baseline), $logPath)
        $writer.WriteLine('COMPUTED window ended with the origin candidate still at the first sample. No component was adjusted.')
        $writer.WriteLine(('origin {0}' -f (Format-Vec $baseline)))
        $writer.WriteLine(('samples {0}' -f $samples))
    }
} catch {
    $terminal = ('FAILED: {0}' -f $_.Exception.Message)
    if ($writer) { $writer.WriteLine($terminal) }
} finally {
    if ($writer) { $writer.Dispose() }
    if ($terminal) { Write-Output $terminal }
}
