/**
 * Per-instance plugin config.
 *
 * SourceMod's AutoExecConfig always writes to, and executes from, the shared game "cfg" folder and ignores
 * the server's SourceMod path, so servers that share one game installation but have separate SourceMod
 * folders overwrite each other's settings. This include keeps the config in
 *
 *   <SourceMod path>/configs/<plugin file name>.cfg
 *
 * (BuildPath(Path_SM, ...) : each server instance gets its own file) and never touches the game cfg folder.
 *
 * Usage, in the plugin:
 *
 *   #include <instance_config>
 *
 *   public void OnPluginStart()
 *   {
 *       gCvFoo = CreateConVar("sm_foo", "1", "What foo does.", 0, true, 0.0, true, 1.0);
 *       InstCfg_Add(gCvFoo);      // every cvar that belongs in the config; leave the version cvar out
 *       InstCfg_Add(gCvOld, true); // a deprecated cvar: its entry gets a comment saying so
 *       InstCfg_Ensure();         // file is created, or completed, once all cvars are registered
 *   }
 *
 *   public void OnConfigsExecuted()
 *   {
 *       InstCfg_Apply();          // every map
 *   }
 *
 * The file is generated from the registered cvars (description, default and limits as comments, then
 * `name "default"`). Entries missing from an existing file are appended at the end; existing lines are never
 * rewritten. Only cvars added with InstCfg_Add are applied; any other name is skipped and logged once.
 * Lines are `name value`; `//` and `;` start a comment line, blank lines are ignored, values may be quoted.
 * A `;` after a value ends it (`afkbomb 2;` sets 2); inside quotes it is part of the value.
 */
#if defined _instance_config_included
 #endinput
#endif
#define _instance_config_included

#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>

#define INSTCFG_LINE_MAX 8192 // longest line read; anything beyond is ignored

static ArrayList g_InstCfgCvars;      // ConVar handles added with InstCfg_Add
static ArrayList g_InstCfgDeprecated; // for each of them, whether it was added as deprecated
static StringMap g_InstCfgReported;   // lower-cased unknown names already written to the log

/**
 * Adds a cvar to the config. Call it for every cvar of the plugin that belongs in the file, after creating it.
 *
 * @param cvar          A cvar created by this plugin.
 * @param deprecated    True for a cvar that is only kept so old configs still load: its entry in a generated
 *                      file says so. The cvar's description plays no part in this.
 */
stock void InstCfg_Add(Handle cvar, bool deprecated = false)
{
	if (g_InstCfgCvars == null)
	{
		g_InstCfgCvars = new ArrayList();
		g_InstCfgDeprecated = new ArrayList();
	}
	if (cvar != null && g_InstCfgCvars.FindValue(cvar) == -1)
	{
		g_InstCfgCvars.Push(cvar);
		g_InstCfgDeprecated.Push(deprecated);
	}
}

/**
 * Makes sure the config file exists and holds an entry for every added cvar: creates it when missing,
 * otherwise appends the missing entries at the end. Nothing already in the file is changed.
 *
 * @return        True when the file is complete.
 */
stock bool InstCfg_Ensure()
{
	if (g_InstCfgCvars == null)
		return false;

	char path[PLATFORM_MAX_PATH];
	InstCfg_Path(path, sizeof(path));

	bool exists = FileExists(path);
	StringMap present = new StringMap();
	bool endsWithNewline = true;

	if (exists)
	{
		File f = OpenFile(path, "rb");
		if (f == null)
		{
			LogError("Cannot read config %s", path);
			delete present;
			return false;
		}
		char line[INSTCFG_LINE_MAX], name[128], value[8];
		bool hasValue;
		while (InstCfg_ReadLine(f, line, sizeof(line)))
		{
			if (InstCfg_ParseLine(line, name, sizeof(name), value, 0, hasValue))
			{
				InstCfg_Lower(name);
				present.SetValue(name, true);
			}
		}
		delete f;

		if (FileSize(path) > 0)
		{
			f = OpenFile(path, "rb");
			if (f != null)
			{
				f.Seek(-1, SEEK_END);
				int last;
				if (f.ReadInt8(last))
					endsWithNewline = (last == '\n');
				delete f;
			}
		}
	}

	File out = OpenFile(path, exists ? "ab" : "wb");
	if (out == null)
	{
		LogError("Cannot write config %s", path);
		delete present;
		return false;
	}

	if (!exists)
		InstCfg_WriteHeader(out);

	int added = 0;
	int count = g_InstCfgCvars.Length;
	for (int i = 0; i < count; i++)
	{
		ConVar cvar = view_as<ConVar>(g_InstCfgCvars.Get(i));
		char name[128];
		cvar.GetName(name, sizeof(name));
		char key[128];
		strcopy(key, sizeof(key), name);
		InstCfg_Lower(key);
		bool dummy;
		if (present.GetValue(key, dummy))
			continue;

		if (exists && added == 0 && !endsWithNewline)
			out.WriteString("\n", false);
		out.WriteString("\n", false); // blank line before every entry
		InstCfg_WriteEntry(out, cvar, g_InstCfgDeprecated.Get(i) != 0);
		added++;
	}
	delete out;
	delete present;

	if (added > 0)
		LogMessage("Config %s: %s %d entr%s", path, exists ? "appended" : "created with", added, added == 1 ? "y" : "ies");
	return true;
}

/**
 * Reads the config file and sets every added cvar it names. Call it from OnConfigsExecuted.
 * Names of cvars that were not added are skipped, and each such name is logged once.
 *
 * @return        True when the file was read.
 */
stock bool InstCfg_Apply()
{
	if (g_InstCfgCvars == null)
		return false;

	char path[PLATFORM_MAX_PATH];
	InstCfg_Path(path, sizeof(path));

	File f = OpenFile(path, "rb");
	if (f == null)
	{
		LogMessage("Config %s cannot be read, settings left as they are", path);
		return false;
	}

	StringMap own = new StringMap();
	int count = g_InstCfgCvars.Length;
	for (int i = 0; i < count; i++)
	{
		ConVar cvar = view_as<ConVar>(g_InstCfgCvars.Get(i));
		char key[128];
		cvar.GetName(key, sizeof(key));
		InstCfg_Lower(key);
		own.SetValue(key, cvar);
	}

	if (g_InstCfgReported == null)
		g_InstCfgReported = new StringMap();

	char line[INSTCFG_LINE_MAX], name[128], value[INSTCFG_LINE_MAX];
	bool hasValue;
	int lineNo = 0;
	while (InstCfg_ReadLine(f, line, sizeof(line)))
	{
		lineNo++;
		if (!InstCfg_ParseLine(line, name, sizeof(name), value, sizeof(value), hasValue))
			continue;

		InstCfg_Lower(name);
		ConVar cvar;
		if (!own.GetValue(name, cvar))
		{
			bool reported;
			if (!g_InstCfgReported.GetValue(name, reported))
			{
				g_InstCfgReported.SetValue(name, true);
				LogMessage("Config %s line %d: \"%s\" is not a setting of this plugin, line skipped", path, lineNo, name);
			}
			continue;
		}
		if (hasValue)
			cvar.SetString(value, false, false);
	}
	delete f;
	delete own;
	return true;
}

//===== internals =====

static void InstCfg_Path(char[] path, int maxlen)
{
	char file[PLATFORM_MAX_PATH];
	GetPluginFilename(GetMyHandle(), file, sizeof(file));
	int slash = FindCharInString(file, '/', true);
	int back = FindCharInString(file, '\\', true);
	if (back > slash)
		slash = back;
	int dot = FindCharInString(file, '.', true);
	if (dot > slash)
		file[dot] = '\0';
	BuildPath(Path_SM, path, maxlen, "configs/%s.cfg", file[slash + 1]);
}

static void InstCfg_Lower(char[] str)
{
	for (int i = 0; str[i] != '\0'; i++)
		str[i] = CharToLower(str[i]);
}

static void InstCfg_WriteHeader(File out)
{
	char file[PLATFORM_MAX_PATH], text[512];
	GetPluginFilename(GetMyHandle(), file, sizeof(file));
	Format(text, sizeof(text), "// Settings of %s, in this server's own SourceMod folder.\n", file);
	out.WriteString(text, false);
	out.WriteString("// Applied on every map change. Lines are: name value  (\"//\" and \";\" start a comment).\n", false);
	out.WriteString("// Entries missing here are appended automatically when the plugin loads; existing lines are never changed.\n", false);
}

static void InstCfg_WriteEntry(File out, ConVar cvar, bool deprecated)
{
	char name[128], desc[2048], def[512], text[4096];
	cvar.GetName(name, sizeof(name));
	cvar.GetDescription(desc, sizeof(desc));
	cvar.GetDefault(def, sizeof(def));
	ReplaceString(desc, sizeof(desc), "\r", "");
	ReplaceString(desc, sizeof(desc), "\n", "\n// ");
	ReplaceString(def, sizeof(def), "\"", "'");

	Format(text, sizeof(text), "// %s\n// Default: %s\n", desc, def);
	out.WriteString(text, false);

	float bound;
	char num[32];
	if (cvar.GetBounds(ConVarBound_Lower, bound))
	{
		InstCfg_FormatNumber(bound, num, sizeof(num));
		Format(text, sizeof(text), "// Min: %s\n", num);
		out.WriteString(text, false);
	}
	if (cvar.GetBounds(ConVarBound_Upper, bound))
	{
		InstCfg_FormatNumber(bound, num, sizeof(num));
		Format(text, sizeof(text), "// Max: %s\n", num);
		out.WriteString(text, false);
	}
	if (deprecated)
		out.WriteString("// DEPRECATED: kept so old configs still load; it has no effect.\n", false);

	Format(text, sizeof(text), "%s \"%s\"\n", name, def);
	out.WriteString(text, false);
}

// 2.0 -> "2", 1.5 -> "1.5"
static void InstCfg_FormatNumber(float value, char[] buffer, int maxlen)
{
	Format(buffer, maxlen, "%.4f", value);
	int len = strlen(buffer);
	while (len > 1 && buffer[len - 1] == '0')
		buffer[--len] = '\0';
	if (len > 0 && buffer[len - 1] == '.')
		buffer[len - 1] = '\0';
}

// Reads the next line of the file into line. A line longer than the buffer is read to its end and only its
// first maxlen - 1 characters are kept, so the next call starts at the next line. False at the end of the file.
static bool InstCfg_ReadLine(File f, char[] line, int maxlen)
{
	if (!f.ReadLine(line, maxlen))
		return false;

	int len = strlen(line);
	bool whole = len > 0 && line[len - 1] == '\n';
	char rest[256];
	while (!whole && !f.EndOfFile() && f.ReadLine(rest, sizeof(rest)))
	{
		len = strlen(rest);
		whole = len > 0 && rest[len - 1] == '\n';
	}
	return true;
}

// Splits one line into name and value. False for blank and comment lines. A value-less name returns hasValue false.
// valueLen 0 : the value is parsed but not stored.
static bool InstCfg_ParseLine(const char[] line, char[] name, int nameLen, char[] value, int valueLen, bool &hasValue)
{
	hasValue = false;
	name[0] = '\0';
	if (valueLen > 0)
		value[0] = '\0';

	int len = strlen(line);
	int i = InstCfg_SkipSpace(line, 0, len);
	if (i >= len)
		return false;
	if (line[i] == ';' || (line[i] == '/' && i + 1 < len && line[i + 1] == '/'))
		return false;

	i = InstCfg_Token(line, i, len, name, nameLen);
	if (name[0] == '\0')
		return false;

	i = InstCfg_SkipSpace(line, i, len);
	if (i >= len || line[i] == ';' || (line[i] == '/' && i + 1 < len && line[i + 1] == '/'))
		return true;

	hasValue = true;
	if (valueLen > 0)
		InstCfg_Token(line, i, len, value, valueLen);
	return true;
}

static int InstCfg_SkipSpace(const char[] line, int i, int len)
{
	while (i < len && (line[i] == ' ' || line[i] == '\t' || line[i] == '\r' || line[i] == '\n'))
		i++;
	return i;
}

// Reads a quoted ("..." may hold spaces) or bare token starting at i; returns the index after it.
// A bare token ends at whitespace or ';' ; a ';' inside quotes is part of the token.
static int InstCfg_Token(const char[] line, int i, int len, char[] out, int outLen)
{
	int n = 0;
	if (line[i] == '"')
	{
		i++;
		while (i < len && line[i] != '"')
		{
			if (n < outLen - 1)
				out[n++] = line[i];
			i++;
		}
		if (i < len)
			i++; // closing quote
	}
	else
	{
		while (i < len && line[i] != ' ' && line[i] != '\t' && line[i] != '\r' && line[i] != '\n' && line[i] != ';')
		{
			if (n < outLen - 1)
				out[n++] = line[i];
			i++;
		}
	}
	out[n] = '\0';
	return i;
}
