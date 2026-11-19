# SourceMod-Incs

Shared SourcePawn includes, one self-contained `.inc` per feature. This repo is the source of truth. Plugin repos
use it as a git submodule at `incs/`, pinned to a commit, and compile with `-iincs/scripting/include`.

## Layout

- `scripting/include/<name>.inc` — the includes
- `docs/<name>.md` — usage and history per include; list every include in the README table
- `tests/<name>_test.sp` — one plugin per include that calls its whole public API
- `build.ps1` — compiles every `tests/*.sp` (`-Deploy` copies to the test server); must pass before committing

## Rules

- Every include: `_<name>_included` guard, `#pragma semicolon 1`, `#pragma newdecls required`, only stock
  SourceMod includes as dependencies, a header comment with usage and the line
  `SPDX-License-Identifier: GPL-3.0-only` (the repo is GPLv3, as SourceMod requires).
- Public functions share a short prefix (`InstCfg_` for instance_config); helpers and globals are `static`.
- Never move or rename `scripting/include/` or an existing `.inc`: every consumer's build points at that path.
  Removing or renaming a public function breaks consumers when they bump the submodule; prefer adding.
- Changing a public function's behaviour or signature: note it under History in `docs/<name>.md` with the commit,
  and list the consumer repos (docs "Used by"). Each consumer bumps its `incs` pointer and runs its own tests;
  this repo does not edit consumer repos.
- Test server and spcomp: `C:\Users\user\cssds\cstrike\addons\sourcemod` (or `$env:SOURCEMOD_DIR`). For running
  srcds see the `srcds-testing` skill.
- SourcePawn style: see SourceMod-AFKbomb's CLAUDE.md "SourcePawn notes".
