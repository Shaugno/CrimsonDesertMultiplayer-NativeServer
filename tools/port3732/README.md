# File-only 3663 → 3732 port tools

Steam updated Crimson Desert from 1.0.0.3663 to 1.0.0.3732 on 2026-11-19.
These scripts move the 3663 adoptions to 3732 by comparing the two
executables as files. They never execute or attach to either image.

Reference images (outside the repository, read-only):

| Build | PE timestamp | SizeOfImage | SHA-256 |
| --- | --- | --- | --- |
| 1.0.0.3663 | `0x6AF5D198` | `0x144E9000` | `8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8` |
| 1.0.0.3732 | `0x6AFDC0CF` | `0x13C7E000` | `DFCF5C507D09AA0E09CF9766DB866FD1E7CF513B9DD7FDC72A8F121BA2D9A9D5` |

3663 came from depot 3321461 manifest 532620525636668883 via DepotDownloader
(only `bin64\CrimsonDesert.exe`); 3732 was copied from the installed game.
Python 3.12.10 with `capstone` and `pefile`; run with `python -I`.

- `sections.py <exe>...` — section table with entropy. The protector renames
  sections every build; the first executable section is plain code.
- `dis.py <exe> <rva> [count]` — capstone disassembly at an RVA.
- `inventory.py <3663.exe> src <out.tsv>` — every RVA-like literal in the 3663
  source files (comments stripped), classified by 3663 section.
- `match.py <3663.exe> <3732.exe> <rva>...` — per RVA: masked byte signature
  (rel32 branch targets and RIP displacements masked) near the old address;
  ambiguous hits resolved only when both neighbouring unique anchors agree on
  the local delta; jmp/call thunks resolved through their target. Output
  `old  new  method  detail`; `-` means unresolved and needs manual review.

A match is a candidate. Each adopted value still needs its site's byte check
and the game-free suite before any live use.
