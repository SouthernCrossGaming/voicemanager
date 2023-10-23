/**
 * Loads an adjustment for a player into voicemanager
 *
 * @param adjuster	        The player invoking the volume adjustment
 * @param adjustedSteamId	The steam id 64 (string) of the player being adjusted.
 * @param volume	        The volume setting they're being adjusted to
 */
native bool LoadPlayerAdjustment(int caller, char adjustedSteamId[18], int volume);

/**
 * Adjusts the volume of a player for a player
 *
 * @param caller	The player invoking the volume adjustment
 * @param client	The player being adjusted locally
 * @param volume	The volume setting they're being adjusted to
 */
native bool OnPlayerAdjustVolume(int caller, int client, int volume);

native bool OnPlayerGlobalAdjust(int caller, int volume);

native bool RefreshActiveOverrides();

/**
 * Clears all overrides a client has set
 *
 * @param client The player invoking the clear request
 */
native bool ClearClientOverrides(int client);

/**
 * Checks to see if a client has another client muted
 *
 * @param client The player invoking the request
 * @param otherClient The player that is being checked for an override
 *
 * @return The value of override this client has set on the other client
 */
native int GetClientOverride(int client, int otherClient);