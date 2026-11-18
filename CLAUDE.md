# SourceMod-Incs

Shared SourcePawn includes, one self-contained `.inc` per feature. Plugins copy the file they need into their own
`scripting/include/`; this repo is the source of truth.

## Layout

- `scripting/include/<name>.inc` — the includes
- `docs/<name>.md` — usage and history per include; list every include in the README table
- `tests/<name>_test.sp` — one plugin per include that calls its whole public API
- `build.ps1` — compiles every `tests/*.sp` (`-Deploy` copies to the test server); must pass before committing

## Rules

- Every include: `_<name>_included` guard, `#pragma semicolon 1`, `#pragma newdecls required`, only stock
  SourceMod includes as dependencies, a header comment with usage.
- Public functions share a short prefix (`InstCfg_` for instance_config); helpers and globals are `static`.
- Changing a public function's behaviour or signature: note it under History in `docs/<name>.md`, and list the
  consumer repos (docs "Used by") that need the new copy.
- Test server and spcomp: `C:\Users\user\cssds\cstrike\addons\sourcemod` (or `$env:SOURCEMOD_DIR`). For running
  srcds see the `srcds-testing` skill.
- SourcePawn style: see SourceMod-AFKbomb's CLAUDE.md "SourcePawn notes".
