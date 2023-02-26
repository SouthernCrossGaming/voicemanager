/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod Sample Extension
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#pragma once

#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_

#include "smsdk_ext.h"
#include <convar.h>
#include "voicemanagerclientstate.h"
#include <map>

 // extern ConVar CvarVoiceManagerEnable;
 // extern ConVar CvarVoiceManagerEnableDebugLogs;

struct ActiveOverride {
	int client;
	int level;
};

struct UserOverride {
	uint64_t steamId;
	int level;
};

class VoiceManagerExt: public SDKExtension, public IConCommandBaseAccessor, public IPluginsListener
{

public:
	virtual bool SDK_OnLoad(char* error, size_t maxlen, bool late);
	virtual void SDK_OnAllLoaded();
	virtual bool SDK_OnMetaModLoad(ISmmAPI* ismm, char* error, size_t maxlen, bool late);
bool RegisterConCommandBase(ConCommandBase* pCommand);public:
	void OnServerActivate(edict_t* pEdictList, int edictCount, int clientMax);
};

#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
