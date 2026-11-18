# SourceMod-Incs

Shared SourcePawn includes for SourceMod plugins. Each include is a single self-contained `.inc` file that only
needs the stock SourceMod includes.

## Includes

| Include | What it does | Docs |
| --- | --- | --- |
| [`instance_config`](scripting/include/instance_config.inc) | Plugin settings in `<SourceMod path>/configs/<plugin>.cfg` instead of the shared game `cfg` folder, so servers that share one game install keep separate settings | [docs/instance_config.md](docs/instance_config.md) |

## Using the includes in a plugin repo

Add this repo as a git submodule at `incs/`, pinned to a commit:

```bash
git submodule add https://github.com/Ballganda/SourceMod-Incs.git incs
```

- Compile with `-iincs/scripting/include` (next to `-i<SourceMod>/scripting/include`), and `#include <name>` in
  the plugin.
- Clone the plugin repo with `git clone --recurse-submodules`, or run `git submodule update --init` in an
  existing clone. A build script should stop with that command in its error when `incs/scripting/include` is
  missing.
- Don't edit the includes inside `incs/`. Change them here, then move the plugin to the new commit:

  ```bash
  git -C incs pull origin main
  ```

  then build, test and commit the new `incs` pointer in the plugin repo.
- Release zips: copy only the includes the plugin uses from `incs/scripting/include/` to
  `addons/sourcemod/scripting/include/` in the zip, so the shipped source recompiles with just the stock includes.

Without git (one-off server builds), copy the `.inc` into `addons/sourcemod/scripting/include/` on the server.

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

1. `scripting/include/<name>.inc` with an `_<name>_included` guard and a header comment showing usage. Keep
   the path: consumer builds point `-i` at `incs/scripting/include`.
2. `docs/<name>.md` and a row in the table above.
3. `tests/<name>_test.sp` that calls every public function; `.\build.ps1` must pass.

## License

CC0 1.0 — see [LICENSE](LICENSE).
