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

 // #include "smsdk_ext.h"
#include "defines.h"
#include "extension.h"
#include "CDetour/detours.h"
// #include <iserver.h>
// #include "include/dirent.h"

#include <iclient.h>
#include "inetmessage.h"
#include <iserver.h>
#include <IBinTools.h>
#include <ISDKTools.h>

DECL_DETOUR(SV_BroadcastVoiceData);

VoiceManagerExt g_VoiceManager; /**< Global singleton for extension's main interface */
IGameConfig* g_pGameConf = nullptr;

// IServer *sv = NULL;

class CGameClient;
class CFrameSnapshot;

SMEXT_LINK(&g_VoiceManager);

IBinTools* g_pBinTools = nullptr;
ISDKTools* g_pSDKTools = nullptr;
IServer* g_pServer = nullptr;

// CGlobalVars *gpGlobals = NULL;
// ICvar *icvar = NULL;
// ConVar CvarVoiceManagerEnable("vm_enable", "1", FCVAR_NONE, "Enable Voice Manager");
// ConVar CvarVoiceManagerEnableDebugLogs("vm_debug", "0", FCVAR_NONE, "Enable Voice Manager Debug Logs");

// int sampleNum = 0;
struct VoiceOverride
{
    std::vector<int> clients;
};

std::map<int, std::map<int, VoiceOverride>> g_activeOverrides;
std::map<uint64_t, std::vector<UserOverride>> m_userOverrides;

VoiceManagerClientState g_voiceManagerClientStates[MAX_PLAYERS];

void SendVoiceDataMsg(int fromClient, int toClient, uint8_t* data, int nBytes, int64 xuid)
{
    IClient* pToClient = g_pServer->GetClient(toClient - 1);
    if (pToClient != NULL && pToClient->IsConnected() && !pToClient->IsFakeClient())
    {
        SVC_VoiceData msg;
        msg.m_bProximity = true;
        msg.m_nLength = nBytes * 8;
        msg.m_xuid = xuid;
        msg.m_nFromClient = fromClient - 1; // 1 is added to this on the server side when reading the message, idk
        msg.m_DataOut = data;
        pToClient->SendNetMsg(msg);
    }
};

DETOUR_DECL_STATIC4(SV_BroadcastVoiceData, void, IClient*, pClient, int, nBytes, uint8_t*, data, int64, xuid)
{
    int nBytesOut;

    int fromClient = playerhelpers->GetClientOfUserId(pClient->GetUserID());
    // auto name = playerhelpers->GetGamePlayer(clientId)->GetName();
    // int64_t steamId = playerhelpers->GetGamePlayer(clientId)->GetSteamId64();

    auto override = g_activeOverrides.find(fromClient);

    // If there are no active overrides for this user, just broadcast the original data
    if (override == g_activeOverrides.end())
    {
        DETOUR_STATIC_CALL(SV_BroadcastVoiceData)(pClient, nBytes, data, xuid);
        return;
    }

    std::map<int, int> overridingClients;

    // Iterate over each volume level to determine if it has clients requesting it
    for (int i = 0; i < MAX_LEVELS; i++)
    {
        // If the level has one or more clients requesting it, we need to re-encode the data at the specified level
        if (override->second[i].clients.size() > 0)
        {
            // WHY AREN"T WE KEEPING STATE, it makes things shitty quality
            VoiceManager* vm = g_voiceManagerClientStates[override->first].GetVoiceManager(i);
            uint8_t* newVoiceData = vm->OnBroadcastVoiceData(pClient, nBytes, data, &nBytesOut);

            // Call CGameClient::SendNetMsg for each overriding client
            for (int client : override->second[i].clients)
            {
                SendVoiceDataMsg(fromClient, client, newVoiceData, nBytesOut, xuid);
                overridingClients.insert(std::pair<int, int>(client, override->first));
            }
        }
    }

    int clientCount = g_pServer->GetClientCount();
    for (int client = 1; client < clientCount; client++)
    {
        if (overridingClients.find(client) != overridingClients.end())
        {
            continue;
        }

        SendVoiceDataMsg(fromClient, client, data, nBytes, xuid);
    }
}

int64_t GetClientSteamId(int client)
{
    int64_t steamId = -1;
    auto player = playerhelpers->GetGamePlayer(client);
    if (player != NULL && player->IsConnected())
    {
        steamId = player->GetSteamId64();
    }

    return steamId;
}

std::map<int64_t, int> GetClientSteamIdMap()
{
    auto steamIdsToSlots = std::map<int64_t, int>();
    for (int client = 0; client < MAX_PLAYERS; client++)
    {
        int64_t steamId = GetClientSteamId(client);
        if (steamId > 0)
        {
            steamIdsToSlots.insert(std::pair<int64_t, int>(steamId, client));
        }
    }

    return steamIdsToSlots;
}

void RefreshActiveOverrides()
{
    auto newActiveOverrides = std::map<int, std::map<int, VoiceOverride>>();
    auto steamIdsToSlots = GetClientSteamIdMap();

    for (auto const& overridePreference : m_userOverrides)
    {
        auto overriderSteamIdToSlot = steamIdsToSlots.find(overridePreference.first);
        if (overriderSteamIdToSlot == steamIdsToSlots.end())
        {
            // The overriding player is not in the server, move on
            continue;
        }

        int overriderSlot = overriderSteamIdToSlot->second;
        auto overrides = overridePreference.second;
        for (auto override: overrides)
        {
            auto overrideeSteamIdToSlot = steamIdsToSlots.find(override.steamId);
            if (overrideeSteamIdToSlot == steamIdsToSlots.end())
            {
                // The overridden player is not in the server, move on
                continue;
            }

            int overrideeSlot = overrideeSteamIdToSlot->second;

            auto newActiveOverride = newActiveOverrides.find(overrideeSlot);
            if (newActiveOverride == newActiveOverrides.end())
            {
                auto overrideLevels = std::map<int, VoiceOverride>();
                for (int i = 0; i < MAX_LEVELS; i++)
                {
                    VoiceOverride vo;
                    vo.clients = std::vector<int>();

                    if (override.level == i)
                    {
                        vo.clients.push_back(overriderSlot);
                    }

                    overrideLevels.insert(std::pair<int, VoiceOverride>{i, vo});
                }

                newActiveOverrides.insert(std::pair<int, std::map<int, VoiceOverride>>{overrideeSlot, overrideLevels});
            }
            else
            {
                newActiveOverride->second.at(override.level).clients.push_back(overriderSlot);
            }
        }
    }

    g_activeOverrides = newActiveOverrides;
}

static cell_t OnPlayerAdjustVolume(IPluginContext* pContext, const cell_t* params)
{
    int adjuster = params[1];
    int adjusted = params[2];
    int volume = params[3];

    smutils->LogError(myself, "We called the native: %i | %i | %i", adjuster, adjusted, volume);

    int64_t adjusterSteamId = GetClientSteamId(adjuster);
    int64_t adjustedSteamId = GetClientSteamId(adjusted);
    if (adjusterSteamId <= 0 || adjustedSteamId <= 0)
    {
        return false;
    }

    auto playerAdjusted = m_userOverrides.find(adjusterSteamId);

    bool isReset = volume < 0;
    if (isReset)
    {
        // Do nothing, they have no overrides and they're setting a preference as normal
        if (playerAdjusted == m_userOverrides.end())
        {
            return true;
        }
        else
        {
            auto existingOverrides = playerAdjusted->second;

            for (int i = 0; i < existingOverrides.size(); i++)
            {
                if (existingOverrides.at(i).steamId == adjustedSteamId)
                {
                    playerAdjusted->second.erase(playerAdjusted->second.begin() + i);
                    break;
                }
            }
        }
    }
    else
    {
        UserOverride uo;
        uo.steamId = adjustedSteamId;
        uo.level = volume;

        if (playerAdjusted != m_userOverrides.end())
        {
            bool existingFound = false;
            for (int i = 0; i < playerAdjusted->second.size(); i++)
            {
                if (playerAdjusted->second.at(i).steamId == adjustedSteamId)
                {
                    playerAdjusted->second.at(i).level = uo.level;
                    existingFound = true;
                    break;
                }
            }

            // If we did not find an existing override, push it
            if (!existingFound)
            {
                playerAdjusted->second.push_back(uo);
            }
        }
        else
        {
            m_userOverrides.insert(std::pair<uint64_t, std::vector<UserOverride>>{
                adjusterSteamId,
                    std::vector<UserOverride>{uo}
            });
        }
    }

    RefreshActiveOverrides();

    return true;
}

const sp_nativeinfo_t g_Natives[] =
{
    {"OnPlayerAdjustVolume", OnPlayerAdjustVolume},
    {NULL,                   NULL},
};

void VoiceManagerExt::SDK_OnAllLoaded()
{
    SM_GET_LATE_IFACE(SDKTOOLS, g_pSDKTools);
    SM_GET_LATE_IFACE(BINTOOLS, g_pBinTools);

    g_pServer = g_pSDKTools->GetIServer();
}

bool VoiceManagerExt::SDK_OnMetaModLoad(ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
    // gpGlobals = ismm->GetCGlobals();

    // GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);

    // g_pCVar = icvar;

    // smutils->LogError(myself, "Registering convar In SDK_OnMetaModLoad");
    // ConVar_Register(0, this);

    return true;
}

bool VoiceManagerExt::SDK_OnLoad(char* error, size_t maxlength, bool late)
{
    sharesys->AddDependency(myself, "sdktools.ext", true, true);
    sharesys->AddDependency(myself, "bintools.ext", true, true);
    plsys->AddPluginsListener(this);
    sharesys->AddNatives(myself, g_Natives);

    char conf_error[255];
    if (!gameconfs->LoadGameConfigFile("voicemanager", &g_pGameConf, conf_error, sizeof(conf_error)))
    {
        if (conf_error[0])
            snprintf(error, maxlength, "Could not read config file voicemanager.txt: %s", conf_error);

        return false;
    }

    CDetourManager::Init(smutils->GetScriptingEngine(), g_pGameConf);

    bool bDetoursInited = false;
    CREATE_DETOUR_STATIC(SV_BroadcastVoiceData, "SV_BroadcastVoiceData", bDetoursInited);

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        g_voiceManagerClientStates[i] = VoiceManagerClientState();
    }

    return true;
}

void DoTranscriptionThing()
{
    char buffer[2056];


    smutils->LogError(myself, "Transcription Output: %s", buffer);
}

// void VoiceManager::DoTheThing()
// {
// struct dirent *entry = nullptr;
// DIR *dp = nullptr;

// std::vector<DecodedChunk> combinedDecodedChunks = std::vector<DecodedChunk>();

// dp = opendir("samples\\compressed");
// if (dp != nullptr)
// {
//     while ((entry = readdir(dp)))
//     {
//         std::string dirName = std::string(entry->d_name);
//         if (dirName.find(".wav") != std::string::npos)
//         {
//             char filepath[64];
//             sprintf(filepath, "samples\\compressed\\%s", entry->d_name);
//             std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
//             int filesize = 0;
//             filesize = stream.tellg();
//             stream.seekg(0, std::ios::beg);

//             uint8_t* buf = new uint8_t[filesize]();
//             stream.read((char*) buf, filesize);
//             stream.close();

//             smutils->LogError(myself, "File %s : %i bytes", entry->d_name, filesize);
//             ParseSteamVoicePacket(buf, filesize);

//             for (auto chunk : m_vecDecodedChunks)
//             {
//                 combinedDecodedChunks.push_back(chunk);
//             }

//             m_vecDecodedChunks.clear();
//             stream.close();
//             // VoiceManager::PrintBytes(entry->d_name, filesize, buf);
//         }
//     }
// }

// closedir(dp);

// int totalSamples = 0;
// for (auto chunk : combinedDecodedChunks)
// {
//     totalSamples += chunk.samples;
// }

// smutils->LogError(myself, "Total Samples: %i", totalSamples);

// int16_t* combinedSamples = new int16_t[totalSamples]();

// int offset = 0;
// for (auto chunk : combinedDecodedChunks)
// {
//     smutils->LogError(myself, "Offset: %i", offset);
//     std::copy(chunk.data, chunk.data + chunk.samples, &combinedSamples[offset]);
//     offset += chunk.samples;
// }

// smutils->LogError(myself, "Here 1");

// writeWAVData("samples\\decompressed\\merged_1.wav", combinedSamples, totalSamples * sizeof(int16_t), 24000, 1);
// std::vector<EncodedChunk> encodedChunks = std::vector<EncodedChunk>();

// smutils->LogError(myself, "Here 2");

// InitOpusEncoder(m_SampleRate);
// for (auto chunk : combinedDecodedChunks)
// {
//     EncodedChunk eChunk = OpusCompress(chunk);
//     encodedChunks.push_back(eChunk);
// }

// smutils->LogError(myself, "Here 3");

// combinedDecodedChunks.clear();
// smutils->LogError(myself, "Here 4");

// for (auto chunk : encodedChunks)
// {
//     DecodedChunk dChunk = OpusDecompress(chunk, false);
//     combinedDecodedChunks.push_back(dChunk);
// }

// smutils->LogError(myself, "Here 5");

// // Decode round 2
// totalSamples = 0;
// for (auto chunk : combinedDecodedChunks)
// {
//     totalSamples += chunk.samples;
// }

// smutils->LogError(myself, "Total Samples: %i", totalSamples);

// combinedSamples = new int16_t[totalSamples]();

// offset = 0;
// for (auto chunk : combinedDecodedChunks)
// {
//     smutils->LogError(myself, "Offset: %i", offset);
//     std::copy(chunk.data, chunk.data + chunk.samples, &combinedSamples[offset]);
//     offset += chunk.samples;
// }

// writeWAVData("samples\\decompressed\\merged_2.wav", combinedSamples, totalSamples * sizeof(int16_t), 24000, 1);
// }

bool VoiceManagerExt::RegisterConCommandBase(ConCommandBase* pCommand)
{
    META_REGCVAR(pCommand);
    return true;
}

void VoiceManagerExt::OnServerActivate(edict_t* pEdictList, int edictCount, int clientMax)
{
    // g_pServer = g_pSDKTools->GetIServer();
    // if (!g_pServer) {
    //     smutils->LogError(myself, "IServer not found");
    // }
}
