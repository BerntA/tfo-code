//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Achievement Manager - Deals with sending achievements from the server to the client.
//
//=============================================================================//

#include "cbase.h"
#include "achievement_manager.h"
#include "usermessages.h"
#include "player.h"
#include "gamerules.h"
#include "util.h"

BEGIN_DATADESC(CAchievementManager)

DEFINE_INPUTFUNC(FIELD_STRING, "GiveAchievement", InputGiveAchievement),
DEFINE_INPUTFUNC(FIELD_STRING, "ChapterTitle", InputSendChapterTitle),

END_DATADESC()

LINK_ENTITY_TO_CLASS(logic_achievement_manager, CAchievementManager);

CAchievementManager::CAchievementManager()
{
}

void CAchievementManager::InputGiveAchievement(inputdata_t& input)
{
	SendAchievement(STRING(input.value.StringID()));
}

void CAchievementManager::InputSendChapterTitle(inputdata_t& input)
{
	char chapterString[64];
	Q_strncpy(chapterString, STRING(input.value.StringID()), sizeof(chapterString));

	CRecipientFilter user;
	user.AddAllPlayers();
	user.MakeReliable();

	UserMessageBegin(user, "ChapterTitle");
	WRITE_STRING(chapterString);
	MessageEnd();
}

/*static*/ void CAchievementManager::SendAchievement(const char* szAchievement)
{
	if (!szAchievement || !szAchievement[0])
		return;

	CRecipientFilter user;
	user.AddAllPlayers();
	user.MakeReliable();

	UserMessageBegin(user, "AchievementData");
	WRITE_STRING(szAchievement);
	MessageEnd();
}