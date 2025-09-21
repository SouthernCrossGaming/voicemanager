#pragma once

#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_

#include <map>
#include <vector>
#include <convar.h>
#include <iclient.h>
#include <iserver.h>
#include <IBinTools.h>
#include <ISDKTools.h>
#include "smsdk_ext.h"
#include "defines.h"
#include "CDetour/detours.h"
#include "voicemanagerclientstate.h"

extern ConVar vm_enable;

struct ActiveOverride
{
	int client;
	int level;
};

struct UserOverride
{
	uint64_t steamId;
	int      level;
};

class VoiceManagerExt : public SDKExtension, public IConCommandBaseAccessor, public IPluginsListener
{
public:
	virtual bool SDK_OnLoad(char* error, size_t maxlen, bool late);
	virtual void SDK_OnAllLoaded();
	virtual bool SDK_OnMetamodLoad(ISmmAPI* ismm, char* error, size_t maxlength, bool late);
	bool         RegisterConCommandBase(ConCommandBase* pCommand);
};

#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
