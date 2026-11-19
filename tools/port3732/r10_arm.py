"""Seal the R10 build for scripts/launch-2inst-3663.ps1. Never deploys or launches the game.

Writes the arming receipt the launcher checks (its contract: version 3, round 7
fields, 3732 executable hash) only when every precondition holds right now:
  - no CrimsonDesert process; the installed executable is exactly 1.0.0.3732;
  - none of the controlled files are installed in the game folder;
  - build/r10/arm-suite.txt shows a full passing suite (>= 121 tests);
  - build/r10/arm-smoke.txt shows the v1 metadata line and a clean unload, and
    the smoke log shows remote states, zero rejects and worker shutdown;
  - the smoke ASI equals the build ASI;
  - the desktop runner pins the 3732 hash;
  - the launcher enables the milestone for the host and the remote-actor guard.
usage: r10_arm.py --build-dir build --prefix r10-l3"""
import argparse, hashlib, json, re, subprocess
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
GAME_DIR = Path(r"D:\SteamLibrary\steamapps\common\Crimson Desert\bin64")
GAME_SHA = "DFCF5C507D09AA0E09CF9766DB866FD1E7CF513B9DD7FDC72A8F121BA2D9A9D5"  # 1.0.0.3732
CONTROLLED = ("cdmp.asi", "cdmp.pdb", "winmm.dll", "cdmp.host.ini", "cdmp.guest.ini", "cdmp.r4-test-lock")


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest().upper()


def require(ok, why):
    if not ok:
        raise SystemExit(f"refused: {why}")


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--build-dir", type=Path, required=True)
    p.add_argument("--prefix", required=True)
    a = p.parse_args()
    require(re.fullmatch(r"[a-zA-Z0-9_-]+", a.prefix) is not None, "prefix")
    local = ROOT / "build/r10"

    tasks = subprocess.run(["tasklist", "/FI", "IMAGENAME eq CrimsonDesert.exe", "/NH"],
                           capture_output=True, text=True).stdout
    require("CrimsonDesert.exe" not in tasks, "the game is running")
    require(sha(GAME_DIR / "CrimsonDesert.exe") == GAME_SHA, "installed executable is not 1.0.0.3732")
    installed = [n for n in CONTROLLED if (GAME_DIR / n).exists()]
    require(not installed, f"controlled files installed: {installed}")

    suite = (local / "arm-suite.txt").read_text(encoding="utf-8-sig")
    total = re.findall(r"100% tests passed, 0 tests failed out of (\d+)", suite)
    seconds = re.findall(r"Total Test time \(real\) =\s*([\d.]+) sec", suite)
    require(total and int(total[-1]) >= 121 and seconds, "full suite evidence")
    smoke = (local / "arm-smoke.txt").read_text(encoding="utf-8-sig")
    require("3663 compiled tier mask 0xF (metadata only)" in smoke and "Mod stopped and unloaded cleanly" in smoke,
            "v1 smoke evidence")
    log = (local / "arm-smoke-run/cdmp.log").read_text(encoding="utf-8-sig")
    states = re.findall(r"remote states (\d+)", log)
    require(states and int(states[-1]) > 0 and "rejected 0" in log and "Network stopped" in log
            and "Worker exiting" in log, "smoke log")

    binary = a.build_dir.resolve() / "bin/RelWithDebInfo"
    asi = binary / "cdmp.asi"
    require(sha(asi) == sha(local / "arm-smoke-run/cdmp.asi"), "smoke ASI differs from the build ASI")
    require(b"3663 milestone active" in asi.read_bytes() and b"null_playback_controller" in asi.read_bytes(),
            "ASI lacks the R10 milestone and controller evidence")
    runner = binary / "r5_desktop_runner.exe"
    require(GAME_SHA.encode("utf-16-le") in runner.read_bytes(), "desktop runner does not pin 1.0.0.3732")
    launcher = (ROOT / "scripts/launch-2inst-3663.ps1").read_text(encoding="utf-8-sig")
    require("$milestone=if($role -eq 'host'){2}else{0}" in launcher and "core_milestone=$milestone" in launcher,
            "launcher does not enable the host milestone")
    require("$remoteActorGuard=1" in launcher and "remote_actor_guard=$remoteActorGuard" in launcher,
            "launcher does not enable the remote-actor guard")

    paths = subprocess.check_output(["git", "ls-files", "-z", "--cached", "--others", "--exclude-standard"],
                                    cwd=ROOT).decode().split("\0")
    sources = sorted(set(f for f in paths if f and (f.startswith((
        "src/", "tests/", "scripts/", "tools/", "cmake/", "third_party/safetyhook-patch/")) or f == "CMakeLists.txt")
        and (ROOT / f).is_file()))
    manifest = [dict(path=f, sha256=sha(ROOT / f)) for f in sources]

    receipt = dict(
        # Launcher contract (scripts/launch-2inst-3663.ps1); the round/date fields are its fixed format.
        version=3, round=7, not_before_utc="2026-11-18T15:00:00Z",
        compiled_tier_mask=15, executable_sha256=GAME_SHA,
        full_suite_passed=True, synthetic_v1_metadata_verified=True,
        isolation_method="nt", host_isolation="none",
        asi_path=str(asi), asi_sha256=sha(asi), pdb_path=str(binary / "cdmp.pdb"),
        watchdog_sha256=sha(ROOT / "tools/port3663/r5_session_watchdog.ps1"),
        desktop_runner_path=str(runner), desktop_runner_sha256=sha(runner),
        source_manifest=manifest,
        # R10 facts.
        r10=dict(game_version="1.0.0.3732", host_core_milestone=2, remote_actor_guard=1,
                 sealed_utc=datetime.now(timezone.utc).isoformat()),
    )
    arming = local / f"{a.prefix}-arming-receipt.json"
    result = ROOT / f"docs/probe-3663/port-evidence/r10/{a.prefix}-validation.json"
    require(not arming.exists() and not result.exists(), "use a fresh prefix")
    with arming.open("x", encoding="utf-8") as f:
        json.dump(receipt, f, indent=2)
        f.write("\n")
    summary = dict(
        round=10, recorded_utc=receipt["r10"]["sealed_utc"],
        game="1.0.0.3732", game_executable_sha256=GAME_SHA,
        suite=f"{total[-1]}/{total[-1]} passed", suite_seconds=float(seconds[-1]),
        smoke=f"v1 metadata mask 0xF; {states[-1]} remote states, 0 rejects, clean unload",
        asi_sha256=receipt["asi_sha256"], desktop_runner_sha256=receipt["desktop_runner_sha256"],
        watchdog_sha256=receipt["watchdog_sha256"],
        host_core_milestone=2, remote_actor_guard=1, controlled_files_installed=False,
        arming_receipt=dict(path=str(arming.resolve()), sha256=sha(arming)),
        source_manifest_sha256=hashlib.sha256(json.dumps(manifest, sort_keys=True).encode()).hexdigest().upper(),
        limits=["Game-free validation only; no 3732 runtime behaviour has been observed.",
                "The arming receipt is valid only while every manifest source, the ASI, runner and watchdog "
                "are unchanged."],
    )
    with result.open("x", encoding="utf-8") as f:
        json.dump(summary, f, indent=2)
        f.write("\n")
    print(json.dumps({k: summary[k] for k in ("suite", "smoke", "asi_sha256", "desktop_runner_sha256")}, indent=1))
    print(f"arming receipt: {arming.resolve()}")


if __name__ == "__main__":
    main()
