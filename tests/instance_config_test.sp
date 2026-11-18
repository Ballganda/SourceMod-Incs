// Compile check for instance_config.inc: uses every public function the way a plugin would.
// Loaded on a server it also writes and applies configs/instance_config_test.cfg under the SourceMod path.
// Behaviour is covered in depth by the AFK Bomb regression suite (Ballganda/SourceMod-AFKbomb, tests/).
#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <instance_config>

public Plugin myinfo =
{
	name = "instance_config test",
	author = "Ballganda",
	description = "Compile and smoke test for instance_config.inc",
	version = "1.0.0",
	url = "https://github.com/Ballganda/SourceMod-Incs"
};

ConVar gCvValue;

public void OnPluginStart()
{
	gCvValue = CreateConVar("instcfg_test_value", "1", "Test value.", 0, true, 0.0, true, 2.0);
	ConVar old = CreateConVar("instcfg_test_old", "0", "Deprecated test value.");
	InstCfg_Add(gCvValue);
	InstCfg_Add(old, true);
	if (!InstCfg_Ensure())
		LogError("InstCfg_Ensure failed");
}

public void OnConfigsExecuted()
{
	if (!InstCfg_Apply())
		LogError("InstCfg_Apply failed");
}
