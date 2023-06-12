//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Server Mode for TFO. Special accessors. 
//
//=============================================================================//
#include "cbase.h"
#include "GameBase_Server.h"
#include "player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CGameBase_Server : public IGameBase_Server
{
public:

	void SendAchievement(const char* szAchievement);
};

static CGameBase_Server g_GBSServer;
IGameBase_Server* GameBaseServer = (IGameBase_Server*)&g_GBSServer;

void CGameBase_Server::SendAchievement(const char* szAchievement)
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer)
		return;

	if (!szAchievement || !szAchievement[0])
		return;

	CSingleUserRecipientFilter user(pPlayer);
	user.MakeReliable();
	UserMessageBegin(user, "AchievementData");
	WRITE_STRING(szAchievement);
	MessageEnd();
}