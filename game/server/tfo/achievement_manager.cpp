//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Achievement Manager - Deals with sending achievements from the server to the client.
//
//=============================================================================//

#include "cbase.h"
#include "usermessages.h"
#include "baseentity.h"
#include "player.h"
#include "gamerules.h"
#include "GameBase_Server.h"
#include "util.h"

class CAchievementManager : public CLogicalEntity
{
	DECLARE_CLASS(CAchievementManager, CLogicalEntity);
	DECLARE_DATADESC();

public:

	CAchievementManager();
	void Spawn();
	void InputGiveAchievement(inputdata_t &input);
	void InputSendChapterTitle(inputdata_t &input);
};

CAchievementManager::CAchievementManager()
{
}

BEGIN_DATADESC(CAchievementManager)

DEFINE_INPUTFUNC(FIELD_STRING, "GiveAchievement", InputGiveAchievement),
DEFINE_INPUTFUNC(FIELD_STRING, "ChapterTitle", InputSendChapterTitle),

END_DATADESC()

LINK_ENTITY_TO_CLASS(logic_achievement_manager, CAchievementManager);

void CAchievementManager::Spawn()
{
	BaseClass::Spawn();
}

void CAchievementManager::InputGiveAchievement(inputdata_t &input)
{
	GameBaseServer->SendAchievement(STRING(input.value.StringID()));
}

void CAchievementManager::InputSendChapterTitle(inputdata_t &input)
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer)
		return;

	char chapterString[64];
	Q_strncpy(chapterString, STRING(input.value.StringID()), 64);

	CRecipientFilter user;
	user.AddAllPlayers();
	user.MakeReliable();
	UserMessageBegin(user, "ChapterTitle");
	WRITE_STRING(chapterString);
	MessageEnd();
}