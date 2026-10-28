#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <PR/ultratypes.h>
#include "fs.h"
#include "config.h"
#include "system.h"
#include "utils.h"

#define CONFIG_MAX_SECNAME 128
#define CONFIG_MAX_KEYNAME 256
#define CONFIG_MAX_SETTINGS 500

typedef enum {
	CFG_NONE,
	CFG_S32,
	CFG_F32,
	CFG_U32,
	CFG_STR
} configtype;

struct configentry {
	char key[CONFIG_MAX_KEYNAME + 1];
	s32 seclen;
	configtype type;
	void *ptr;
	union {
		struct { f32 min_f32, max_f32; };
		struct { s32 min_s32, max_s32; };
		struct { u32 min_u32, max_u32; };
		u32 max_str;
	};
} settings[CONFIG_MAX_SETTINGS];

static s32 numSettings = 0;
static u8 configMaxWarningLogged = 0;

static inline s32 configClampInt(s32 val, s32 min, s32 max)
{
	return (val < min) ? min : ((val > max) ? max : val);
}

static inline u32 configClampUInt(u32 val, u32 min, u32 max)
{
	return (val < min) ? min : ((val > max) ? max : val);
}

static inline f32 configClampFloat(f32 val, f32 min, f32 max)
{
	return (val < min) ? min : ((val > max) ? max : val);
}

static inline struct configentry *configFindEntry(const char *key)
{
	for (s32 i = 0; i < numSettings; ++i) {
		if (!strncasecmp(settings[i].key, key, CONFIG_MAX_KEYNAME)) {
			return &settings[i];
		}
	}
	return NULL;
}

static inline struct configentry *configAddEntry(const char *key)
{
	if (numSettings < CONFIG_MAX_SETTINGS) {
		struct configentry *cfg = &settings[numSettings++];
		snprintf(cfg->key, CONFIG_MAX_KEYNAME, "%s", key);
		const char *delim = strrchr(cfg->key, '.');
		cfg->seclen = delim ? (delim - cfg->key) : 0;
		return cfg;
	}
	if (!configMaxWarningLogged) {
		sysLogPrintf(LOG_WARNING, "Maximum number of configuration entries exceeded: %d", CONFIG_MAX_SETTINGS);
		configMaxWarningLogged = 1;
	}
	return NULL;
}

static inline struct configentry *configFindOrAddEntry(const char *key)
{
	for (s32 i = 0; i < numSettings; ++i) {
		if (!strncasecmp(settings[i].key, key, CONFIG_MAX_KEYNAME)) {
			return &settings[i];
		}
	}
	return configAddEntry(key);
}

static inline const char *configGetSection(char *sec, const struct configentry *cfg)
{
	if (!cfg->seclen || cfg->seclen > CONFIG_MAX_SECNAME) {
		strncpy(sec, cfg->key, CONFIG_MAX_SECNAME);
		sec[CONFIG_MAX_SECNAME] = '\0';
		return sec;
	}

	memcpy(sec, cfg->key, cfg->seclen);
	sec[cfg->seclen] = '\0';

	return sec;
}

void configRegisterInt(const char *key, s32 *var, s32 min, s32 max)
{
	struct configentry *cfg = configFindOrAddEntry(key);
	if (cfg) {
		cfg->type = CFG_S32;
		cfg->ptr = var;
		cfg->min_s32 = min;
		cfg->max_s32 = max;
	}
}

void configRegisterUInt(const char* key, u32* var, u32 min, u32 max)
{
	struct configentry* cfg = configFindOrAddEntry(key);
	if (cfg) {
		cfg->type = CFG_U32;
		cfg->ptr = var;
		cfg->min_u32 = min;
		cfg->max_u32 = max;
	}
}

void configRegisterFloat(const char *key, f32 *var, f32 min, f32 max)
{
	struct configentry *cfg = configFindOrAddEntry(key);
	if (cfg) {
		cfg->type = CFG_F32;
		cfg->ptr = var;
		cfg->min_f32 = min;
		cfg->max_f32 = max;
	}
}

void configRegisterString(const char *key, char *var, u32 maxstr)
{
	struct configentry *cfg = configFindOrAddEntry(key);
	if (cfg) {
		cfg->type = CFG_STR;
		cfg->ptr = var;
		cfg->max_str = maxstr;
	}
}

static void configSetFromString(const char *key, const char *val)
{
	struct configentry *cfg = configFindEntry(key);
	if (!cfg) return;

	s32 tmp_s32;
	f32 tmp_f32;
	u32 tmp_u32;
	switch (cfg->type) {
		case CFG_S32:
			tmp_s32 = strtol(val, NULL, 0);
			if (cfg->min_s32 < cfg->max_s32) {
				tmp_s32 = configClampInt(tmp_s32, cfg->min_s32, cfg->max_s32);
			}
			*(s32 *)cfg->ptr = tmp_s32;
			break;
		case CFG_F32:
			tmp_f32 = strtof(val, NULL);
			if (cfg->min_f32 < cfg->max_f32) {
				tmp_f32 = configClampFloat(tmp_f32, cfg->min_f32, cfg->max_f32);
			}
			*(f32 *)cfg->ptr = tmp_f32;
			break;
		case CFG_U32:
			tmp_u32 = strtoul(val, NULL, 0);
			if (cfg->min_u32 < cfg->max_u32) {
				tmp_u32 = configClampUInt(tmp_u32, cfg->min_u32, cfg->max_u32);
			}
			*(u32*)cfg->ptr = tmp_u32;
			break;
		case CFG_STR:
			strncpy(cfg->ptr, val, cfg->max_str ? cfg->max_str - 1 : 4096);
			break;
		default:
			break;
	}
}

static void configSaveEntry(struct configentry *cfg, FILE *f)
{
	switch (cfg->type) {
		case CFG_S32:
			if (cfg->min_s32 < cfg->max_s32) {
				*(s32 *)cfg->ptr = configClampInt(*(s32 *)cfg->ptr, cfg->min_s32, cfg->max_s32);
			}
			fprintf(f, "%s=%d\n", cfg->key + cfg->seclen + 1, *(s32 *)cfg->ptr);
			break;
		case CFG_F32:
			if (cfg->min_f32 < cfg->max_f32) {
				*(f32 *)cfg->ptr = configClampFloat(*(f32 *)cfg->ptr, cfg->min_f32, cfg->max_f32);
			}
			fprintf(f, "%s=%f\n", cfg->key + cfg->seclen + 1, *(f32 *)cfg->ptr);
			break;
		case CFG_U32:
			if (cfg->min_u32 < cfg->max_u32) {
				*(u32*)cfg->ptr = configClampUInt(*(u32*)cfg->ptr, cfg->min_u32, cfg->max_u32);
			}
			fprintf(f, "%s=%u\n", cfg->key + cfg->seclen + 1, *(u32 *)cfg->ptr);
			break;
		case CFG_STR:
			fprintf(f, "%s=%s\n", cfg->key + cfg->seclen + 1, (char *)cfg->ptr);
			break;
		default:
			break;
	}
}

s32 configSave(const char *fname)
{
	FILE *f = fsFileOpenWrite(fname);
	if (!f) {
		return 0;
	}

	char tmpSec[CONFIG_MAX_SECNAME + 1] = { 0 };
	char curSec[CONFIG_MAX_SECNAME + 1] = { 0 };
	configGetSection(curSec, &settings[0]);
	fprintf(f, "[%s]\n", curSec);

	for (s32 i = 0; i < numSettings; ++i) {
		struct configentry *cfg = &settings[i];
		configGetSection(tmpSec, cfg);
		if (strncmp(curSec, tmpSec, CONFIG_MAX_SECNAME) != 0) {
			fprintf(f, "\n[%s]\n", tmpSec);
			strncpy(curSec, tmpSec, CONFIG_MAX_SECNAME);
		}
		configSaveEntry(cfg, f);
	}

	fsFileFree(f);
	return 1;
}

s32 configLoad(const char *fname)
{
	FILE *f = fsFileOpenRead(fname);
	if (!f) {
		return 0;
	}

	char curSec[CONFIG_MAX_SECNAME + 1] = { 0 };
	char keyBuf[CONFIG_MAX_SECNAME * 2 + 2] = { 0 }; // SECTION + . + KEY + \0
	char token[UTIL_MAX_TOKEN + 1] = { 0 };
	char lineBuf[2048] = { 0 };
	char *line = lineBuf;
	s32 lineLen = 0;

	while (fgets(lineBuf, sizeof(lineBuf), f)) {
		line = lineBuf;

		line = strParseToken(line, token, NULL);

		if (token[0] == '[' && token[1] == '\0') {
			// section; get name
			line = strParseToken(line, token, NULL);
			if (!token[0]) {
				sysLogPrintf(LOG_ERROR, "configLoad: malformed section line: %s", lineBuf);
				continue;
			}
			strncpy(curSec, token, CONFIG_MAX_SECNAME);
			// eat ]
			line = strParseToken(line, token, NULL);
			if (token[0] != ']' || token[1] != '\0') {
				sysLogPrintf(LOG_ERROR, "configLoad: malformed section line: %s", lineBuf);
			}
		} else if (token[0]) {
			// probably a key=value pair; append key name to section name
			snprintf(keyBuf, sizeof(keyBuf) - 1, "%s.%s", curSec, token);
			// eat =
			line = strParseToken(line, token, NULL);
			if (token[0] != '=' || token[1] != '\0') {
				sysLogPrintf(LOG_ERROR, "configLoad: malformed keyvalue line: %s", lineBuf);
				continue;
			}
			// the rest of the line is the value
			line = strTrim(line);
			if (line[0] == '"') {
				line = strUnquote(line);
			}
			configSetFromString(keyBuf, line);
		}
	}

	fsFileFree(f);

	return 1;
}

void configInit(void)
{
	if (fsFileSize(CONFIG_PATH) > 0) {
		configLoad(CONFIG_PATH);
	}
}
