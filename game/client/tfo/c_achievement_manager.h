//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Achievement handler & manager. 
//
//=============================================================================//

#ifndef CLIENT_ACHIEVEMENT_HANDLER_H
#define CLIENT_ACHIEVEMENT_HANDLER_H

#ifdef _WIN32
#pragma once
#endif

#include <steam/steam_api.h>

struct AchievementEntry
{
	const char* achievement;
	bool bAchieved;
};

class CAchievementManager
{
public:
	CAchievementManager();
	~CAchievementManager();

	void Load();
	void Reset();

	void WriteToAchievement(const char* szAchievement);
	bool HasAllAchievements(void);
	bool HasAchievement(int index);

private:
	STEAM_CALLBACK(CAchievementManager, OnUserStatsReceived, UserStatsReceived_t, m_CallbackUserStatsReceived);

	bool m_bHasLoadedSteamStats;
};

extern CAchievementManager* AchievementManager;

#endif // CLIENT_ACHIEVEMENT_HANDLER_H