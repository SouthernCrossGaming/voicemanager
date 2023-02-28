#include "extension.h"
#include "inetmessage.h"

DECL_DETOUR(SV_BroadcastVoiceData);

VoiceManagerExt g_VoiceManager; // Global singleton for extension's main interface
IGameConfig* g_pGameConf = nullptr;

class CGameClient;
class CFrameSnapshot;

SMEXT_LINK(&g_VoiceManager);

IBinTools* g_pBinTools = nullptr;
ISDKTools* g_pSDKTools = nullptr;
IServer* g_pServer = nullptr;

struct VoiceOverride
{
    std::vector<int> clients;
};

std::map<int, std::map<int, VoiceOverride>> g_activeOverrides;
std::map<uint64_t, std::vector<UserOverride>> m_userOverrides;

VoiceManagerClientState g_voiceManagerClientStates[MAXPLAYERS + 1];

void SendVoiceDataMsg(int fromClientSlot, IClient* pToClient, uint8_t* data, int nBytes, int64 xuid)
{
    SVC_VoiceData msg;
    msg.m_bProximity = false;
    msg.m_nLength = nBytes * 8;
    msg.m_xuid = xuid;
    msg.m_nFromClient = fromClientSlot;
    msg.m_DataOut = data;
    pToClient->SendNetMsg(msg);
};

DETOUR_DECL_STATIC4(SV_BroadcastVoiceData, void, IClient*, pClient, int, nBytes, uint8_t*, data, int64, xuid)
{
    int fromClientSlot = pClient->GetPlayerSlot();
    int fromClientIndex = pClient->GetPlayerSlot() + 1;

    // If there are no active overrides for this user, just broadcast the original data
    auto override = g_activeOverrides.find(fromClientIndex);
    if (override == g_activeOverrides.end())
    {
        DETOUR_STATIC_CALL(SV_BroadcastVoiceData)(pClient, nBytes, data, xuid);
        return;
    }

    // Populate a map of potential recipients.
    // The criteria for a recipient:
    // - The client is found
    // - The client is connected
    // - The client is not a bot (does not include replay/source tv)
    // - The client can hear the player (accounts for mutes, voice loopback)
    std::map<int, IClient*> recipientMap;
    for (int client = 1; client <= playerhelpers->GetMaxClients(); client++)
    {
        IGamePlayer* pGamePlayer = playerhelpers->GetGamePlayer(client);
        if (pGamePlayer == nullptr || !pGamePlayer->IsConnected() || pGamePlayer->IsFakeClient())
        {
            continue;
        }

        IClient* pToClient = g_pServer->GetClient(client - 1);
        if (pToClient == nullptr || !pToClient->IsHearingClient(fromClientSlot))
        {
            continue;
        }

        recipientMap.insert({ client, pToClient });
    }

    // Iterate over each volume level to determine if it has clients requesting it
    std::map<int, int> overridingClients;
    for (int level = 0; level < MAX_LEVELS; level++)
    {
        // If the level has one or more clients requesting it, we need to re-encode the data at the specified level
        if (override->second[level].clients.size() > 0)
        {
            VoiceManager* vm = g_voiceManagerClientStates[override->first].GetVoiceManager(level);

            int nBytesOverride;
            uint8_t* newVoiceData = vm->OnBroadcastVoiceData(pClient, nBytes, data, &nBytesOverride);

            // Call CGameClient::SendNetMsg for each overriding client
            for (int client : override->second[level].clients)
            {
                // If the overriding client is not found, move on
                auto toClientPair = recipientMap.find(client);
                if (toClientPair == recipientMap.end())
                {
                    continue;
                }

                SendVoiceDataMsg(fromClientSlot, toClientPair->second, newVoiceData, nBytesOverride, xuid);

                // Remove the recipient from the map as we've already sent them the voice message
                recipientMap.erase(client);
            }
        }
    }

    for (const auto& [index, client] : recipientMap)
    {
        SendVoiceDataMsg(fromClientSlot, client, data, nBytes, xuid);
    }
}

int64_t GetClientSteamId(int client)
{
    auto player = playerhelpers->GetGamePlayer(client);
    if (player == nullptr || !player->IsConnected())
    {
        return -1;
    }

    return player->GetSteamId64();
}

std::map<int64_t, int> GetClientSteamIdMap()
{
    auto steamIdsToSlots = std::map<int64_t, int>();
    for (int client = 0; client < playerhelpers->GetMaxClients(); client++)
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
            auto overriddenSteamIdToSlot = steamIdsToSlots.find(override.steamId);
            if (overriddenSteamIdToSlot == steamIdsToSlots.end())
            {
                // The overridden player is not in the server, move on
                continue;
            }

            int overriddenSlot = overriddenSteamIdToSlot->second;

            auto newActiveOverride = newActiveOverrides.find(overriddenSlot);
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

                newActiveOverrides.insert(std::pair<int, std::map<int, VoiceOverride>>{overriddenSlot, overrideLevels});
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
    {nullptr, nullptr},
};

void VoiceManagerExt::SDK_OnAllLoaded()
{
    SM_GET_LATE_IFACE(SDKTOOLS, g_pSDKTools);
    SM_GET_LATE_IFACE(BINTOOLS, g_pBinTools);

    g_pServer = g_pSDKTools->GetIServer();
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
        {
            snprintf(error, maxlength, "Could not read config file voicemanager.txt: %s", conf_error);
        }

        return false;
    }

    CDetourManager::Init(smutils->GetScriptingEngine(), g_pGameConf);

    bool bDetoursInited = false;
    CREATE_DETOUR_STATIC(SV_BroadcastVoiceData, "SV_BroadcastVoiceData", bDetoursInited);

    for (int i = 1; i <= playerhelpers->GetMaxClients(); i++)
    {
        g_voiceManagerClientStates[i] = VoiceManagerClientState();
    }

    return true;
}

bool VoiceManagerExt::RegisterConCommandBase(ConCommandBase* pCommand)
{
    META_REGCVAR(pCommand);
    return true;
}