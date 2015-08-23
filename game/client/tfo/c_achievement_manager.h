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

class CAchievementManager
{
public:
	CAchievementManager();
	virtual ~CAchievementManager();

	bool WriteToAchievement(const char *szAchievement);
	bool CanWriteToAchievement(const char *szAchievement);
	bool HasAllAchievements(void);
	bool HasAchievement(const char *szAch = NULL, int iID = 0);

	STEAM_CALLBACK(CAchievementManager, OnUserStatsReceived, UserStatsReceived_t, m_CallbackUserStatsReceived);
	STEAM_CALLBACK(CAchievementManager, OnAchievementStored, UserAchievementStored_t, m_CallbackAchievementStored);
};

#endif // CLIENT_ACHIEVEMENT_HANDLER_H