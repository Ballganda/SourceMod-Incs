# instance_config

Per-instance plugin config. SourceMod's `AutoExecConfig` always writes to, and runs from, the shared game `cfg`
folder, so servers that share one game install but have separate SourceMod folders overwrite each other's
settings. This include keeps the file at `<SourceMod path>/configs/<subfolder>/<plugin file name>.cfg`
(`BuildPath(Path_SM, ...)`) and never touches the game `cfg` folder. `<subfolder>` is the plugin's folder under
`plugins/`, so two plugins with the same file name in different folders get different files. Missing folders
are created.

| Plugin | Config |
| --- | --- |
| `plugins/afkbomb.smx` | `configs/afkbomb.cfg` |
| `plugins/test/afkbomb.smx` | `configs/test/afkbomb.cfg` |

## API

| Function | When | Does |
| --- | --- | --- |
| `InstCfg_Add(cvar, deprecated = false)` | `OnPluginStart`, after `CreateConVar` | Registers a cvar for the file. `deprecated` adds a comment saying it has no effect. Leave the version cvar out. |
| `InstCfg_Ensure()` | `OnPluginStart`, after all `InstCfg_Add` | Creates the file, or appends missing entries. Existing lines are never changed. |
| `InstCfg_Apply()` | `OnConfigsExecuted` | Reads the file and sets every registered cvar it names. Unknown names are skipped and logged once. |

```sourcepawn
#include <instance_config>

public void OnPluginStart()
{
	gCvFoo = CreateConVar("sm_foo", "1", "What foo does.", 0, true, 0.0, true, 1.0);
	InstCfg_Add(gCvFoo);
	InstCfg_Add(gCvOld, true);
	InstCfg_Ensure();
}

public void OnConfigsExecuted()
{
	InstCfg_Apply();
}
```

## File format

- Generated entries: description, default and min/max as `//` comments, then `name "default"`.
- Lines are `name value`; `//` and `;` start a comment line; blank lines are ignored; values may be quoted.
- A `;` after a bare value ends it (`foo 2;` sets `2`); inside quotes it is part of the value.
- Lines longer than 8192 characters are truncated.

## Used by

- [SourceMod-AFKbomb](https://github.com/Ballganda/SourceMod-AFKbomb) — submodule at `incs/`; its
  `tests/afkbomb_tests.sp` suite exercises this include through AFK Bomb's cvars.
- [SourceMod-DisableScopes](https://github.com/Ballganda/SourceMod-DisableScopes) — submodule at `incs/`.

## History

- Config path keeps the plugin's subfolder under `plugins/` (Incs #1).
  A plugin directly in `plugins/` keeps its path. A plugin in a subfolder now uses
  `configs/<subfolder>/<name>.cfg`: the old `configs/<name>.cfg` is no longer read, and a new file with default
  values is written. Server owners move the old file to the new path to keep their settings.
  `InstCfg_Ensure` returns false and logs one error when it cannot create the folder.

Moved here from SourceMod-AFKbomb `scripting/include/instance_config.inc` at commit `145a087`, unchanged.
Earlier changes, in that repo:

- `a11b8f1` — first version (AFKbomb #12)
- `e87d439` — value ends at a trailing semicolon (AFKbomb #26)
- `0947308` — explicit deprecation flag; one line reader (AFKbomb #29)
