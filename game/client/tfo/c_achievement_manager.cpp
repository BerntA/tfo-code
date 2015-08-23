//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Achievement handler & manager. 
//
//=============================================================================//

#include "cbase.h"
#include "c_achievement_manager.h"
#include "hud.h"
#include "hud_macros.h"
#include "hudelement.h"
#include "hud_achievement.h"

// Contains a list of available achievements.
const char *GameAchievements[] =
{
	"ACH_HEALTHKIT",
	"ACH_WEAPON_SPECIAL_1",
	"ACH_WEAPON_SPECIAL_2",
	"ACH_WEAPON_SPECIAL_3",
	"ACH_EASTER_SLENDER",
	"ACH_TOURISTS_FRIEND",
	"ACH_WEAPON_BASH",
	"ACH_DARKBOOK",
	"ACH_NPC_BURN",
	"ACH_TOURISTS_BETRAY",
	"ACH_VENGEANCE",
	"ACH_ENDGAME",
};

CAchievementManager::CAchievementManager() : m_CallbackUserStatsReceived(this, &CAchievementManager::OnUserStatsReceived), m_CallbackAchievementStored(this, &CAchievementManager::OnAchievementStored)
{
}

CAchievementManager::~CAchievementManager()
{
}

// Check if we have all the available achievements.
bool CAchievementManager::HasAllAchievements(void)
{
	int iCount = 0;

	for (int i = 0; i < _ARRAYSIZE(GameAchievements); i++)
	{
		bool bAchieved = false;
		steamapicontext->SteamUserStats()->GetAchievement(GameAchievements[i], &bAchieved);
		if (bAchieved)
			iCount++;
	}

	return (iCount >= _ARRAYSIZE(GameAchievements));
}

// Check if a certain achievement has been achieved by the user.
bool CAchievementManager::HasAchievement(const char *szAch, int iID)
{
	if (iID >= _ARRAYSIZE(GameAchievements))
		return false;

	bool bAchieved = false;

	const char *szAchievement = szAch;
	if (!szAchievement)
		szAchievement = GameAchievements[iID];

	steamapicontext->SteamUserStats()->GetAchievement(szAchievement, &bAchieved);

	return bAchieved;
}

// Set an achievement from 0 to 1 = set to achieved. YOU CAN'T SET THE ACHIEVEMENT PROGRESS HERE.
// To set the achievement progress you must first add a stat and link it to the achievement in Steamworks, increase the stat to see the progress update of the achievement in Steam, etc...
bool CAchievementManager::WriteToAchievement(const char *szAchievement)
{
	if (!CanWriteToAchievement(szAchievement))
	{
		DevMsg("Failed to write to an achievement!\n");
		return false;
	}

	// Do the change.
	if (steamapicontext->SteamUserStats()->SetAchievement(szAchievement))
	{
		// Store the change.
		if (!steamapicontext->SteamUserStats()->StoreStats())
			DevMsg("Failed to store the achievement!\n");

		steamapicontext->SteamUserStats()->RequestCurrentStats();
		return true;
	}

	return false;
}

// Make sure that we can write to the achievements before we actually write so we don't crash the game.
bool CAchievementManager::CanWriteToAchievement(const char *szAchievement)
{
	// Make sure that we're connected.
	if (!engine->IsInGame())
		return false;

	// Make sure that we will not write to any achievements if we have cheats enabled.
	ConVar *varCheats = cvar->FindVar("sv_cheats");
	if (varCheats)
	{
		if (varCheats->GetBool())
			return false;
	}

	// Make sure our interface is running.
	if (!steamapicontext)
		return false;

	// Make sure that we're logged in.
	if (!steamapicontext->SteamUserStats())
		return false;

	if (!steamapicontext->SteamUser())
		return false;

	if (!steamapicontext->SteamUser()->BLoggedOn())
		return false;

	bool bFound = false;
	for (int i = 0; i < _ARRAYSIZE(GameAchievements); i++)
	{
		if (!strcmp(GameAchievements[i], szAchievement))
		{
			bFound = true;
			break;
		}
	}

	if (!bFound)
		return false;

	bool bAchieved = false;
	steamapicontext->SteamUserStats()->GetAchievement(szAchievement, &bAchieved);

	return !bAchieved;
}

void CAchievementManager::OnUserStatsReceived(UserStatsReceived_t *pCallback)
{
	DevMsg("Loaded Stats and Achievements!\nResults %i\n", (int)pCallback->m_eResult);
}

void CAchievementManager::OnAchievementStored(UserAchievementStored_t *pCallback)
{
	if (pCallback->m_nCurProgress == pCallback->m_nMaxProgress)
	{
		CHudAchievement *pHudHR = GET_HUDELEMENT(CHudAchievement);
		if (pHudHR)
		{
			pHudHR->ShowAchievement();
		}
	}
}