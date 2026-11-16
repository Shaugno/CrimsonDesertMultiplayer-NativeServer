# Exercise the production receipt conversion against PowerShell's JSON date modes.
$ErrorActionPreference='Stop'
$tokens=$null; $errors=$null
$ast=[Management.Automation.Language.Parser]::ParseFile((Join-Path $PSScriptRoot 'r2_p1_session.ps1'),[ref]$tokens,[ref]$errors)
if ($errors.Count) { throw 'Session controller parse failure' }
$assignment=$ast.Find({param($node)
    $node -is [Management.Automation.Language.AssignmentStatementAst] -and
    $node.Left.Extent.Text -eq '$watchDeadline'
},$true)
if (!$assignment) { throw 'Missing production deadline conversion' }
$convert=[scriptblock]::Create($assignment.Right.Extent.Text)
$expected=[datetime]::Parse('2026-11-16T13:16:08.7072654Z',[Globalization.CultureInfo]::InvariantCulture,[Globalization.DateTimeStyles]::RoundtripKind)
$values=@('2026-11-16T13:16:08.7072654Z','2026-11-16T08:16:08.7072654-05:00',$expected,$expected.ToLocalTime())
foreach ($value in $values) {
    $watch=@{deadline_utc=$value}
    $actual=& $convert
    if ($actual.Kind -ne [DateTimeKind]::Utc -or $actual.Ticks -ne $expected.Ticks) {
        throw "Deadline precision/timezone mismatch for $($value.GetType().FullName)"
    }
}
$watch=('{"deadline_utc":"2026-11-16T13:16:08.7072654Z"}' | ConvertFrom-Json)
if ((& $convert).Ticks -ne $expected.Ticks) { throw 'Native JSON roundtrip failed' }
$watch=@{deadline_utc='invalid'}
$rejected=$false
try { $null=& $convert } catch { $rejected=$true }
if (!$rejected) { throw 'Invalid timestamp accepted' }
'Session receipt conversion PASS: string Z/offset, UTC/local DateTime, native JSON, precision and invalid-input rejection; no game opened.'
