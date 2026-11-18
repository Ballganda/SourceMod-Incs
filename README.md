# SourceMod-Incs

Shared SourcePawn includes for SourceMod plugins. Each include is a single self-contained `.inc` file that only
needs the stock SourceMod includes.

## Includes

| Include | What it does | Docs |
| --- | --- | --- |
| [`instance_config`](scripting/include/instance_config.inc) | Plugin settings in `<SourceMod path>/configs/<plugin>.cfg` instead of the shared game `cfg` folder, so servers that share one game install keep separate settings | [docs/instance_config.md](docs/instance_config.md) |

## Using an include in a plugin

1. Copy the `.inc` file into the plugin's own `scripting/include/` folder (or into the server's
   `addons/sourcemod/scripting/include/`).
2. `#include <name>` in the plugin and compile with `-i<that folder>`.

Copying keeps each plugin buildable and its release zip complete on its own. When an include changes here, copy
the new version into each plugin that uses it.

## Layout

- `scripting/include/*.inc` — the includes; same folder layout as SourceMod, so the folder can be passed to
  `spcomp -i` directly
- `docs/<name>.md` — per-include usage notes and change history
- `tests/<name>_test.sp` — a plugin per include that uses its public API; `build.ps1` compiles them all
- `plugins/` — build output (git-ignored)

## Building

```powershell
.\build.ps1           # compile every tests\*.sp
.\build.ps1 -Deploy   # and copy the .smx files to the server
```

`build.ps1` uses `spcomp.exe` from `$env:SOURCEMOD_DIR`, or `C:\Users\user\cssds\cstrike\addons\sourcemod`.

## Adding an include

1. `scripting/include/<name>.inc` with an `_<name>_included` guard and a header comment showing usage.
2. `docs/<name>.md` and a row in the table above.
3. `tests/<name>_test.sp` that calls every public function; `.\build.ps1` must pass.

## License

CC0 1.0 — see [LICENSE](LICENSE).
