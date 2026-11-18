# instance_config

Per-instance plugin config. SourceMod's `AutoExecConfig` always writes to, and runs from, the shared game `cfg`
folder, so servers that share one game install but have separate SourceMod folders overwrite each other's
settings. This include keeps the file at `<SourceMod path>/configs/<plugin file name>.cfg`
(`BuildPath(Path_SM, ...)`) and never touches the game `cfg` folder.

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

- [SourceMod-AFKbomb](https://github.com/Ballganda/SourceMod-AFKbomb) — its `tests/afkbomb_tests.sp` suite
  exercises this include through AFK Bomb's cvars.

## History

Moved here from SourceMod-AFKbomb `scripting/include/instance_config.inc` at commit `145a087`, unchanged.
Earlier changes, in that repo:

- `a11b8f1` — first version (AFKbomb #12)
- `e87d439` — value ends at a trailing semicolon (AFKbomb #26)
- `0947308` — explicit deprecation flag; one line reader (AFKbomb #29)
