//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Achievement Manager - Deals with sending achievements from the server to the client.
//
//=============================================================================//

#ifndef TFO_ACHIEVEMENT_MANAGER_H
#define TFO_ACHIEVEMENT_MANAGER_H

#ifdef _WIN32
#pragma once
#endif

#include "baseentity.h"

class CAchievementManager : public CLogicalEntity
{
	DECLARE_CLASS(CAchievementManager, CLogicalEntity);
	DECLARE_DATADESC();

public:

	CAchievementManager();

	void InputGiveAchievement(inputdata_t& input);
	void InputSendChapterTitle(inputdata_t& input);

	static void SendAchievement(const char* szAchievement);
};

#endif // TFO_ACHIEVEMENT_MANAGER_H