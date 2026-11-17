# GPT task: prepare a Proxmox VM as the L4 "second PC" (guest)

Written by Claude, 2026-11-17. This is preparation for R7 Step 4 (L4). It does
not replace the R7 plan, and it must not launch Crimson Desert on the Razer
before the R7 time gate (2026-11-18 10:00 AM Eastern).

## Goal

A Windows VM on Shaugn's Proxmox cluster that can run Crimson Desert 1.0.0.3663
as the **guest** for L4, signed into a **second Steam account**, Steam friends
with the host account. The host stays on the Razer.

## Hard constraints

- **Ask Shaugn before any Proxmox host change that needs a reboot** (IOMMU,
  kernel parameters, vfio binding, BIOS settings). The Proxmox hosts also run
  Pi-hole (home DNS), AMP game servers, the Cloudflare tunnel and Uptime Kuma;
  a reboot interrupts them. Schedule it with him and say which services go
  down and for how long.
- Do not change other VMs/containers, storage pools, backups or network
  bridges beyond what this VM needs. Take a Proxmox backup/snapshot plan into
  account before changing host config.
- Never enter passwords, Steam credentials or Steam Guard codes yourself.
  Shaugn signs in.
- **Copy protection:** Crimson Desert uses anti-tamper with activation limits
  (it caused a 24-hour lockout on 2026-11-17). Launch the game in the VM as
  few times as possible. Do not copy, delete or edit any anti-tamper or
  activation file, and never patch the executable. If the game refuses to run
  in the VM (virtual machine or hypervisor detection, an activation error),
  stop and report it; do not try to hide the VM.

## Step 0 — Feasibility check (no changes; report before continuing)

Report each item, then stop and give Shaugn a go/no-go recommendation:

1. **GPU:** which Proxmox host has a discrete GPU that can be passed through to
   a VM (model, VRAM, IOMMU group, whether it is in use by anything else).
   Crimson Desert needs a DX12 GPU; a VM without GPU passthrough (or vGPU) will
   not run it. If no suitable GPU exists, recommend the main desktop (RTX 4090)
   instead and stop.
2. **Host support:** CPU/IOMMU (VT-d/AMD-Vi) enabled, current kernel command
   line, whether passthrough is already configured anywhere.
3. **Resources:** free RAM (target 16 GB+ for the VM), CPU cores (6–8 vCPU),
   storage for Windows plus the game (about 150 GB+, preferably SSD/NVMe).
4. **Second Steam account:** confirm with Shaugn that it exists and **owns its
   own copy** of Crimson Desert. Family Sharing usually cannot run the same
   game at the same time as the owner, and protected games are often excluded.
5. **Windows licence/ISO** available for the VM.
6. **Display/input:** how Shaugn will see and control the VM with a GPU passed
   through (Parsec, Sunshine/Moonlight, or a monitor on that GPU). Proxmox's
   noVNC console will not show the passed-through GPU output.

## Step 1 — Create the VM (after go-ahead)

- Windows 11, q35, OVMF/UEFI, TPM 2.0 (vTPM), VirtIO SCSI disk, VirtIO NIC on
  the normal LAN bridge, CPU type `host`, ballooning off, 16 GB+ RAM.
- GPU passthrough (primary GPU + its audio function, `pcie=1`), only after the
  host is prepared with Shaugn's approval.
- Install VirtIO drivers, the GPU driver, Windows updates.
- Install a remote-play tool (Parsec or Sunshine) so the VM's GPU output can
  be viewed from the Razer or another device.

## Step 2 — Steam and game

- Install Steam. **Shaugn signs into the second account.**
- Install Crimson Desert and confirm the executable is **exactly 1.0.0.3663**,
  SHA-256 `8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8`.
  If Steam updated it, stop: the mod supports only that build.
- Make the second account and the host account **Steam friends**.
- Launch the game **once** to set windowed/borderless mode at 1280×720 and to
  confirm it runs in the VM, then quit. If anti-tamper or VM detection blocks
  it, stop and report (see constraints).

## Step 3 — Guest kit (only after L3 passes on the Razer)

- Build the guest kit from the same commit and binary as the passing L3
  (`docs/PRUEBA-DOS-PC.md`, R7 guest isolation: save-directory redirect only,
  no profile override). Copy it to the VM; never copy game files.
- Back up the VM's save folder (`%LOCALAPPDATA%\Pearl Abyss\CD\save`) with a
  hash manifest before the first co-op session.
- Do not run L4 until R7 says L3 passed.

## Report

Write `docs/probe-3663/L4-SECOND-PC-SETUP.md` on `gpt/3663-port`: the
feasibility findings, every host/VM change with Shaugn's approval noted, the
VM spec, the game build hash, the one test launch result, and anything left.
Commit and push.

## Prompt to give GPT

> Read docs/GPT-L4-SECOND-PC-PROXMOX.md (attached; save it to docs/ on
> gpt/3663-port and commit it). Do Step 0 now and report the go/no-go to me
> before changing anything. Do not reboot or reconfigure any Proxmox host
> without asking me first. Do not launch Crimson Desert on the Razer before
> 2026-11-18 10:00 AM Eastern.
