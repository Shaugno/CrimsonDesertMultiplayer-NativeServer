# P1 controller. Never launches the game; the assistant requests presence first.
param(
    [Parameter(Mandatory=$true)][ValidateSet('Prepare','Deploy','Probe','Finish')][string]$Stage,
    [Parameter(Mandatory=$true)][ValidatePattern('^R2-P1-[0-9]{8}-[0-9]{6}$')][string]$Session,
    [int]$GamePid = 0,
    [ValidateSet('hp','loading')][string]$Profile = 'hp',
    [switch]$Present,
    [string]$BuildDir = '',
    [ValidatePattern('^$|^R2-MATCH-PAUSE-[0-9]{8}-[0-9]{6}$')][string]$MatchingPauseSession = ''
)
$ErrorActionPreference='Stop'
$root=Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$buildRoot = if (!$BuildDir) { Join-Path $root 'build' } elseif ([IO.Path]::IsPathRooted($BuildDir)) {
    [IO.Path]::GetFullPath($BuildDir)
} else { [IO.Path]::GetFullPath((Join-Path $root $BuildDir)) }
$gameDir='D:\SteamLibrary\steamapps\common\Crimson Desert\bin64'
$saveRoot=Join-Path $env:LOCALAPPDATA 'Pearl Abyss\CD\save'
$backupRoot=Join-Path $root "backups\$Session"
$evidence=Join-Path $root "build\r2\live\$Session"
$py='C:\Users\shaug\source\repos\beamng-earth-s3-review\python312\python.exe'
$expected='8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8'
if ((Get-FileHash -LiteralPath (Join-Path $gameDir 'CrimsonDesert.exe')).Hash -ne $expected) { throw 'Build drift; rematch first' }
if ($Stage -ne 'Probe' -and (Get-Process CrimsonDesert -ErrorAction SilentlyContinue)) { throw 'Game must be closed' }
if ($Stage -in @('Deploy','Probe') -and !$Present) { throw 'The session requires the user at the PC' }
$sessionQuestion = if ($Profile -eq 'hp') {
    "Which native HP definition key selects the local actor's status entry on the exact 3663 executable?"
} else {
    'Does the candidate local-player chain expose a distinct missing-chain or parked-position transition during loading and fast travel on exact 3663?'
}
$expectedObservation = if ($Profile -eq 'hp') {
    "The native definition key selects one status entry whose current/max integer values, divided by the proved unit scale 1000, agree with the character's HP display. Both definition registries are logged separately; no ID is guessed."
} else {
    'The guarded candidate position and rotation follow the controllable character, then loading produces a missing chain or a distinctive park interval before placement. The old (0,1000,0) park is only a logged hypothesis, not an adopted readiness rule.'
}
$sessionSteps = if ($Profile -eq 'hp') {
    'Select Continue and wait for control; show the character/status HP display and note current/max HP; let ordinary regeneration or a healing action change HP if available; quit to desktop at the stop signal.'
} else {
    'Start collection at the title screen, select Continue, wait for control, walk a few steps and turn, then use one ordinary fast travel; wait for placement and control, walk and turn again, and quit to desktop at the stop signal.'
}
function Manifest([string]$path) {
    if (!(Test-Path -LiteralPath $path)) { throw "Missing source $path" }
    $links=@(Get-ChildItem -LiteralPath $path -Recurse -Force | Where-Object { $_.Attributes -band [IO.FileAttributes]::ReparsePoint })
    if ($links.Count) { throw 'Reparse points in saves; refuse an ambiguous backup' }
    $files=@(Get-ChildItem -LiteralPath $path -Recurse -File -Force | Sort-Object FullName)
    @($files | ForEach-Object { [pscustomobject]@{path=$_.FullName.Substring($path.Length).TrimStart('\');bytes=$_.Length;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash} })
}
function Canonical($value) { ConvertTo-Json -InputObject @($value) -Depth 8 -Compress }
function AssertNoMatching {
    $jobs=@(Get-CimInstance Win32_Process -Filter "Name='java.exe' OR Name='bindiff.exe'" | Where-Object { $_.CommandLine -match 'R2(Match|Target|Graph|Correlation|Direct|Native)|PortTarget3663|Port3663|r2-(direct|graph|native)' })
    if (!$jobs.Count) { return }
    if (!$MatchingPauseSession) { throw 'Offline matching jobs must finish or be verifiably paused before the live session' }
    & (Join-Path $PSScriptRoot 'r2_matching_pause.ps1') -Stage Verify -Session $MatchingPauseSession
    $path=Join-Path $root "build\r2\matching-pause\$MatchingPauseSession.json"
    $receipt=Get-Content -Raw -LiteralPath $path | ConvertFrom-Json
    $allowed=@($receipt.entries | ForEach-Object { $_.pid })
    if (@($jobs | Where-Object { $_.ProcessId -notin $allowed }).Count) { throw 'An unlisted offline matcher is active' }
    @{session=$MatchingPauseSession;receipt_sha256=(Get-FileHash -LiteralPath $path).Hash;
      state='every owned matcher thread verified suspended';pids=$allowed} |
      ConvertTo-Json | Set-Content -LiteralPath (Join-Path $evidence 'paused-matching.json')
}
if ($Stage -eq 'Prepare') {
    if (Test-Path -LiteralPath $backupRoot) { throw 'Session backup already exists; use a new session ID' }
    New-Item -ItemType Directory -Force -Path $backupRoot,$evidence | Out-Null
    $before=@(Manifest $saveRoot)
    if (!$before.Count) { throw 'No save exists to load for this session' }
    foreach ($file in $before) {
        $to=Join-Path $backupRoot ('save\'+$file.path)
        New-Item -ItemType Directory -Force -Path (Split-Path $to -Parent) | Out-Null
        Copy-Item -LiteralPath (Join-Path $saveRoot $file.path) -Destination $to
    }
    if ((Canonical $before) -ne (Canonical @(Manifest $saveRoot))) { throw 'Saves changed during backup' }
    if ((Canonical $before) -ne (Canonical @(Manifest (Join-Path $backupRoot 'save')))) { throw 'Backup verification failed' }
    ConvertTo-Json -InputObject $before -Depth 8 | Set-Content (Join-Path $backupRoot 'save-manifest.json')
    $installed=@()
    foreach ($name in @('cdmp.asi','cdmp.pdb','winmm.dll','cdmp.ini')) {
        $from=Join-Path $gameDir $name
        if (Test-Path -LiteralPath $from) {
            New-Item -ItemType Directory -Force -Path (Join-Path $backupRoot 'install') | Out-Null
            Copy-Item -LiteralPath $from -Destination (Join-Path $backupRoot ('install\'+$name))
            $installed += [pscustomobject]@{name=$name;sha256=(Get-FileHash -LiteralPath $from).Hash}
        }
    }
    ConvertTo-Json -InputObject $installed -Depth 8 | Set-Content (Join-Path $backupRoot 'install-manifest.json')
    @"
# $Session — passive P1, prepared before launch

Question: $sessionQuestion

Expected observation: $expectedObservation

Duration: ten minutes from collector start, at most fifteen minutes for the sitting.

Steps: after Shaugn confirms presence, deploy the closed-guard ASI; launch the game visibly. $sessionSteps

Stop: any crash, hang, identity drift, user request, or time limit. Never retry a crash blindly. Collect cdmp.log and crash context, fix offline and rerun the suite before another session.

Collector: external PROCESS_VM_READ | PROCESS_QUERY_INFORMATION only. Every copy checks VirtualQueryEx and ReadProcessMemory. No hooks, game writes, native calls, packet replay or network feed. This is not L1 and does not arm Tier A.

Supervisor: before collection, arm r2_session_watchdog.ps1 for the exact game PID. It pins the process handle and closes that test process at fourteen minutes from launch if normal quit has not completed. This separate deadline control does not grant the collector memory-write rights.

Save policy: all saves backed up with a local hash manifest and verified before launch. Afterward verify the backup and compare the live save folder; report normal game changes without overwriting player progress. Never commit saves or the private manifest.

Installed executable SHA256: $expected
Backup files: $($before.Count)
Backup manifest SHA256: $((Get-FileHash -LiteralPath (Join-Path $backupRoot 'save-manifest.json')).Hash)
"@ | Set-Content (Join-Path $evidence 'question.md')
    @{profile=$Profile;question=$sessionQuestion;build_root=$buildRoot} | ConvertTo-Json | Set-Content (Join-Path $evidence 'session-profile.json')
    Write-Output "Prepared $Session; files=$($before.Count); backup verified; no deployment or game launch."
    exit
}
if (!(Test-Path -LiteralPath (Join-Path $evidence 'question.md'))) { throw 'Prepare this session first' }
$profileFile=Join-Path $evidence 'session-profile.json'
$prepared=if (Test-Path -LiteralPath $profileFile) { Get-Content -Raw -LiteralPath $profileFile | ConvertFrom-Json } else { @{profile='hp'} }
$preparedProfile=$prepared.profile
if ($Profile -ne $preparedProfile) { throw 'Profile differs from the written question; prepare a fresh session' }
if ($prepared.build_root -and $prepared.build_root -ne $buildRoot) { throw 'Build directory differs from preparation' }
$before=@(Get-Content -Raw -LiteralPath (Join-Path $backupRoot 'save-manifest.json') | ConvertFrom-Json)
$installed=@(Get-Content -Raw -LiteralPath (Join-Path $backupRoot 'install-manifest.json') | ConvertFrom-Json)
if ($Stage -eq 'Deploy') {
    AssertNoMatching
    $header=Get-Content -Raw -LiteralPath (Join-Path $root 'src\mod\game\build_3663.h')
    foreach ($tier in 'A','B','C','D') {
        if ($header -notmatch "kTier${tier}Complete = false;") { throw 'P1 requires every production tier closed' }
    }
    foreach ($name in @('cdmp.asi','cdmp.pdb','winmm.dll','cdmp.ini')) {
        $path=Join-Path $gameDir $name
        $prior=@($installed | Where-Object name -eq $name)
        if ($prior.Count) {
            if (!(Test-Path -LiteralPath $path) -or (Get-FileHash -LiteralPath $path).Hash -ne $prior[0].sha256) { throw 'Installation changed since preparation' }
        } elseif (Test-Path -LiteralPath $path) { throw 'An installation file appeared after preparation' }
    }
    $loader=Join-Path $root 'third_party\ultimate-asi-loader\v9.7.4\dinput8.dll'
    $existingLoader=Join-Path $gameDir 'winmm.dll'
    if ((Test-Path -LiteralPath $existingLoader) -and (Get-FileHash -LiteralPath $existingLoader).Hash -ne (Get-FileHash -LiteralPath $loader).Hash) { throw 'Different loader installed; preserve it and block this controlled deployment' }
    if ((Canonical $before) -ne (Canonical @(Manifest $saveRoot))) { throw 'Saves changed since preparation; prepare a fresh session' }
    & $py (Join-Path $PSScriptRoot 'reader_probe.py') --self-test
    if ($LASTEXITCODE) { throw 'Collector logic test failed' }
    & $py (Join-Path $PSScriptRoot 'reader_probe.py') --api-self-test
    if ($LASTEXITCODE) { throw 'Collector API test failed' }
    & (Join-Path $root 'scripts\deploy.ps1') -BuildDir $buildRoot
    $ini="[general]`nplayer_name=R2PassiveP1`nstartup_delay_ms=0`n[net]`nenabled=0`n[overlay]`nenabled=0`n[sniffer]`nenabled=0`n[coop]`nenabled=0`n[session2]`nenabled=0`n"
    [IO.File]::WriteAllText((Join-Path $gameDir 'cdmp.ini'),$ini,[Text.UTF8Encoding]::new($false))
    Get-FileHash -LiteralPath (Join-Path $gameDir 'cdmp.asi'),(Join-Path $PSScriptRoot 'reader_probe.py'),(Join-Path $root 'src\mod\game\build_3663.h') | ConvertTo-Json | Set-Content (Join-Path $evidence 'deployed-hashes.json')
    $receipt=foreach ($name in @('cdmp.asi','cdmp.pdb','winmm.dll','cdmp.ini')) {
        $path=Join-Path $gameDir $name
        if (Test-Path -LiteralPath $path) { [pscustomobject]@{name=$name;sha256=(Get-FileHash -LiteralPath $path).Hash} }
    }
    ConvertTo-Json -InputObject @($receipt) | Set-Content (Join-Path $evidence 'installed-receipt.json')
    Write-Output 'Deployed closed-guard passive configuration. Launch only with user present.'
} elseif ($Stage -eq 'Probe') {
    AssertNoMatching
    if ($GamePid -lt 1) { throw 'An explicit verified game PID is required' }
    $watchPath=Join-Path $evidence 'watchdog.json'
    if (!(Test-Path -LiteralPath $watchPath)) { throw 'Arm the independent session watchdog first' }
    $watch=Get-Content -Raw -LiteralPath $watchPath | ConvertFrom-Json
    $watchDeadline = if ($watch.deadline_utc -is [datetime]) { $watch.deadline_utc.ToUniversalTime() } else { [datetime]::Parse($watch.deadline_utc, [Globalization.CultureInfo]::InvariantCulture, [Globalization.DateTimeStyles]::RoundtripKind).ToUniversalTime() }
    $game=Get-Process -Id $GamePid
    $controller=Get-Process -Id $watch.controller_pid
    $controllerInfo=Get-CimInstance Win32_Process -Filter "ProcessId=$($watch.controller_pid)"
    if ($watch.state -ne 'watching' -or $watch.game_pid -ne $GamePid -or
        $watch.game_start_filetime -ne $game.StartTime.ToUniversalTime().ToFileTimeUtc().ToString() -or
        $watch.controller_start_filetime -ne $controller.StartTime.ToUniversalTime().ToFileTimeUtc().ToString() -or
        $controllerInfo.CommandLine -notlike '*r2_session_watchdog.ps1*' -or
        $controllerInfo.CommandLine -notlike "*$Session*" -or
        $watch.max_game_age_seconds -ne 840 -or
        $watchDeadline -ne $game.StartTime.ToUniversalTime().AddSeconds(840) -or
        $watchDeadline -le [DateTime]::UtcNow -or
        $watch.helper_sha256 -ne (Get-FileHash -LiteralPath (Join-Path $PSScriptRoot 'r2_session_watchdog.ps1')).Hash) {
        throw 'Session watchdog identity/deadline is not valid'
    }
    & $py (Join-Path $PSScriptRoot 'reader_probe.py') --pid $GamePid --seconds 600 --mode $Profile *> (Join-Path $evidence 'trace.log')
    if ($LASTEXITCODE) { throw 'Probe failed; inspect trace; do not retry blindly' }
} else {
    $receiptPath=Join-Path $evidence 'installed-receipt.json'
    if (!(Test-Path -LiteralPath $receiptPath)) { throw 'No completed deployment receipt; preserve the installation for inspection' }
    foreach ($item in @(Get-Content -Raw -LiteralPath $receiptPath | ConvertFrom-Json)) {
        $path=Join-Path $gameDir $item.name
        if (!(Test-Path -LiteralPath $path) -or (Get-FileHash -LiteralPath $path).Hash -ne $item.sha256) { throw 'Installed file changed during session; preserve it for inspection' }
    }
    if ((Canonical $before) -ne (Canonical @(Manifest (Join-Path $backupRoot 'save')))) { throw 'Backup changed; stop' }
    $after=@(Manifest $saveRoot)
    ConvertTo-Json -InputObject $after -Depth 8 | Set-Content (Join-Path $backupRoot 'save-after.json')
    $summary=[pscustomobject]@{session=$Session;exe_sha256=$expected;backup_verified=$true;live_saves_unchanged=((Canonical $before) -eq (Canonical $after));before_files=$before.Count;after_files=$after.Count}
    $summary | ConvertTo-Json | Set-Content (Join-Path $evidence 'save-verification.json')
    if (Test-Path -LiteralPath (Join-Path $gameDir 'cdmp.log')) { Copy-Item -LiteralPath (Join-Path $gameDir 'cdmp.log') -Destination (Join-Path $evidence 'cdmp.log') -Force }
    & (Join-Path $root 'scripts\deploy.ps1') -Remove
    foreach ($item in $installed) {
        Copy-Item -LiteralPath (Join-Path $backupRoot ('install\'+$item.name)) -Destination (Join-Path $gameDir $item.name) -Force
        if ((Get-FileHash -LiteralPath (Join-Path $gameDir $item.name)).Hash -ne $item.sha256) { throw 'Installation restore verification failed' }
    }
    if (!($installed | Where-Object name -eq 'cdmp.ini')) { Remove-Item -LiteralPath (Join-Path $gameDir 'cdmp.ini') -ErrorAction SilentlyContinue }
    Get-ChildItem -LiteralPath $evidence -File | Get-FileHash | ConvertTo-Json | Set-Content (Join-Path $evidence 'evidence-hashes.json')
    Write-Output "Finished $Session; backup verified, installation restored; live saves unchanged=$($summary.live_saves_unchanged)"
}
