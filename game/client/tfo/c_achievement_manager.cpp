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

extern bool IsInCommentaryMode(void);

static CAchievementManager g_sAchievementManager;
CAchievementManager* AchievementManager = &g_sAchievementManager;

static AchievementEntry ACHIEVEMENTS[] =
{
	{ "ACH_HEALTHKIT", false },
	{ "ACH_WEAPON_SPECIAL_1", false },
	{ "ACH_WEAPON_SPECIAL_2", false },
	{ "ACH_WEAPON_SPECIAL_3", false },
	{ "ACH_EASTER_SLENDER", false },
	{ "ACH_TOURISTS_FRIEND", false },
	{ "ACH_WEAPON_BASH", false },
	{ "ACH_DARKBOOK", false },
	{ "ACH_NPC_BURN", false },
	{ "ACH_TOURISTS_BETRAY", false },
	{ "ACH_VENGEANCE", false },
	{ "ACH_ENDGAME", false },
};

static void __MsgFunc_AchievementData(bf_read& msg)
{
	char szAchievement[80];
	msg.ReadString(szAchievement, sizeof(szAchievement));
	AchievementManager->WriteToAchievement(szAchievement);
}

CAchievementManager::CAchievementManager() : m_CallbackUserStatsReceived(this, &CAchievementManager::OnUserStatsReceived)
{
	m_bHasLoadedSteamStats = false;
}

CAchievementManager::~CAchievementManager()
{
}

void CAchievementManager::Load()
{
	HOOK_MESSAGE(AchievementData);

	if (!steamapicontext || !steamapicontext->SteamUserStats())
		return;

	steamapicontext->SteamUserStats()->RequestCurrentStats();
}

void CAchievementManager::Reset()
{
	if (!steamapicontext || !steamapicontext->SteamUserStats())
		return;

	Warning("You've reset your stats & achievements!\n");
	m_bHasLoadedSteamStats = false;
	steamapicontext->SteamUserStats()->ResetAllStats(true);
	steamapicontext->SteamUserStats()->RequestCurrentStats();
}

// Check if we have all the available achievements.
bool CAchievementManager::HasAllAchievements(void)
{
	for (int i = 0; i < _ARRAYSIZE(ACHIEVEMENTS); i++)
	{
		if (ACHIEVEMENTS[i].bAchieved == false)
			return false;
	}
	return true;
}

// Check if a certain achievement has been achieved by the user.
bool CAchievementManager::HasAchievement(int index)
{
	if ((index < 0) || (index >= _ARRAYSIZE(ACHIEVEMENTS)))
		return false;
	return ACHIEVEMENTS[index].bAchieved;
}

// Set an achievement from 0 to 1 = set to achieved. YOU CAN'T SET THE ACHIEVEMENT PROGRESS HERE.
// To set the achievement progress you must first add a stat and link it to the achievement in Steamworks, increase the stat to see the progress update of the achievement in Steam, etc...
void CAchievementManager::WriteToAchievement(const char* szAchievement)
{
	// Make sure that we're connected.
	if (!engine->IsInGame() || engine->IsPlayingDemo() || IsInCommentaryMode() ||
		!m_bHasLoadedSteamStats || !szAchievement || !szAchievement[0] ||
		!steamapicontext || !steamapicontext->SteamUserStats() || !steamapicontext->SteamUser() || !steamapicontext->SteamUser()->BLoggedOn())
		return;

	CHudAchievement* pHudHR = GET_HUDELEMENT(CHudAchievement);

	for (int i = 0; i < _ARRAYSIZE(ACHIEVEMENTS); i++)
	{
		AchievementEntry* pAchievement = &ACHIEVEMENTS[i];

		if (pAchievement->bAchieved || (strcmp(pAchievement->achievement, szAchievement) != 0))
			continue;

		if (steamapicontext->SteamUserStats()->SetAchievement(pAchievement->achievement))
		{
			pAchievement->bAchieved = true;
			steamapicontext->SteamUserStats()->StoreStats();

			if (pHudHR)
				pHudHR->ShowAchievement();

			break;
		}
	}
}

void CAchievementManager::OnUserStatsReceived(UserStatsReceived_t* pCallback)
{
	if (m_bHasLoadedSteamStats || !steamapicontext || !steamapicontext->SteamUserStats())
		return;

	DevMsg("Steam Stats: EResult %d\n", pCallback->m_eResult);

	if (pCallback->m_eResult != k_EResultOK)
	{
		Warning("Unable to load steam stats, achievements will be disabled!\n");
		return;
	}

	m_bHasLoadedSteamStats = true; // Ensure we do not try to load this again!

	// Load achievement values, states, etc.
	for (int i = 0; i < _ARRAYSIZE(ACHIEVEMENTS); i++)
	{
		bool bAchieved = false;
		AchievementEntry* pAchievement = &ACHIEVEMENTS[i];
		steamapicontext->SteamUserStats()->GetAchievement(pAchievement->achievement, &bAchieved);
		pAchievement->bAchieved = bAchieved;
	}
}

CON_COMMAND_F(tfo_reset_achievements, "Reset Stats & Achievements", FCVAR_HIDDEN)
{
	AchievementManager->Reset();
};