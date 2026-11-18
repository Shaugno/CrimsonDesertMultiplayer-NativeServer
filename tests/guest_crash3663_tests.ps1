param([Parameter(Mandatory=$true)][string]$FixtureExe)
$ErrorActionPreference = 'Stop'
$temp = [IO.Path]::GetFullPath([IO.Path]::GetTempPath()).TrimEnd('\')
$root = [IO.Path]::GetFullPath((Join-Path $temp ('cdmp_guest_crash_' + [guid]::NewGuid().ToString('N'))))
if (!(Split-Path $root -Parent).Equals($temp,[StringComparison]::OrdinalIgnoreCase)) { throw 'Invalid fixture root' }
New-Item -ItemType Directory -Path $root | Out-Null
function Test-CrashCapture([string]$role) {
    $work = Join-Path $root $role
    New-Item -ItemType Directory -Path $work | Out-Null
    $evidence = Join-Path $work 'evidence'
    if ($role -eq 'host') { $env:CDMP_HOST_CRASH_DIR = $evidence }
    else { $env:CDMP_SAVE_BACKUP = Join-Path $work 'guest-backup' }
    $process = Start-Process -FilePath $FixtureExe -ArgumentList $role -WorkingDirectory $work -WindowStyle Hidden -PassThru
    if (!$process.WaitForExit(15000)) {
        $process.Kill()
        $process.WaitForExit()
        throw 'Fixture exceeded timeout'
    }
    if ($process.ExitCode -ne -1073741819) { throw "Expected native access violation, got $($process.ExitCode)" }
    $crash = Join-Path $evidence "$role-crash-$($process.Id)"
    $note = [IO.File]::ReadAllText($crash + '.log').TrimEnd([char]0)
    if (@([regex]::Matches($note,'(?m)^first_chance ')).Count -ne 2) { throw 'Wrong first-chance count' }
    $peer = if ($role -eq 'host') { 'actor=0xA01001BE class=0xA identity=guest' }
            else { 'actor=0xA0100001 class=0xA identity=host' }
    foreach ($pattern in "3663 $role diagnostics installed", 'unhandled .+code=0xC0000005 at=cdmp_tests_guest_crash3663\.exe\+0x',
                         'access=1 address=0x','registers .+ rsp=0x','stack .+ <- ','minidump=written',
                         "null_playback .+ $peer",
                         'null_playback_skip .+ continuation=CrimsonDesert.exe\+0x258C99A') {
        if ($note -notmatch $pattern) { throw "Missing crash evidence pattern $pattern" }
    }
    if ((Get-Content -Raw -LiteralPath (Join-Path $work 'prior-filter.txt')) -notmatch 'previous filter called') {
        throw 'Previous filter did not run'
    }
    $dump = [IO.File]::ReadAllBytes($crash + '.dmp')
    if ([Text.Encoding]::ASCII.GetString($dump,0,4) -ne 'MDMP') { throw 'Invalid minidump signature' }
    $hasException = $false
    $directory = [BitConverter]::ToInt32($dump,12)
    for ($i = 0; $i -lt [BitConverter]::ToInt32($dump,8); ++$i) {
        $entry = $directory + 12*$i
        if ([BitConverter]::ToUInt32($dump,$entry) -ne 6) { continue }
        $start = [BitConverter]::ToInt32($dump,$entry+8)
        $hasException = [BitConverter]::ToInt32($dump,$start+8) -eq -1073741819
    }
    if (!$hasException) { throw 'Missing access violation exception stream' }
    $trace = Join-Path $evidence "guest-packets-$($process.Id).bin"
    if ($role -eq 'host') {
        if (Test-Path -LiteralPath $trace) { throw 'Host installed a guest packet trace' }
        return
    }
    $packets = [IO.File]::ReadAllBytes($trace)
    if ([Text.Encoding]::ASCII.GetString($packets,0,8) -ne 'CDMPPKT1' -or
        [BitConverter]::ToUInt32($packets,12) -ne 3) { throw 'Invalid packet trace' }
    $expected = @(@(3081,2,1),@(2364,1,1),@(2005,1,0))
    for ($i = 0; $i -lt $expected.Count; ++$i) {
        $row = 24 + 16*$i
        if ([BitConverter]::ToUInt16($packets,$row+8) -ne $expected[$i][0] -or
            $packets[$row+12] -ne $expected[$i][1] -or $packets[$row+13] -ne $expected[$i][2]) {
            throw "Wrong packet row $i"
        }
    }
}
try {
    Test-CrashCapture 'host'
    Test-CrashCapture 'guest'
    Write-Output 'Host and guest crash fixtures PASS: armed-only first chance, native exit, prior filter, stack, minidump and guest pending packet'
} finally {
    Remove-Item -LiteralPath $root -Recurse -Force
}
