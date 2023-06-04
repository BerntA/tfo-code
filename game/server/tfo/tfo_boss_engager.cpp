//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Boss Engager - Starts boss fights. Sends a message to the hud_bossbar for reading the name of the npc and his health.
//
//=============================================================================//

#include "cbase.h"
#include "usermessages.h"
#include "baseentity.h"
#include "player.h"
#include "gamerules.h"
#include "util.h"
#include "GameEventListener.h"

// Get class deffs
#include "npc_monster.h"
#include "npc_soldier.h"

class CBossEngager : public CLogicalEntity
{
	DECLARE_CLASS(CBossEngager, CLogicalEntity);
	DECLARE_DATADESC();

public:
	CBossEngager();

	void OnThink();
	void InputStart(inputdata_t& input);

private:
	string_t m_szTarget;
	EHANDLE m_hTarget;
	COutputEvent OnStartedBossFight;
};

CBossEngager::CBossEngager()
{
	m_szTarget = NULL_STRING;
	m_hTarget = NULL;
}

BEGIN_DATADESC(CBossEngager)

DEFINE_THINKFUNC(OnThink),

DEFINE_KEYFIELD(m_szTarget, FIELD_STRING, "target"),
DEFINE_FIELD(m_hTarget, FIELD_EHANDLE),

DEFINE_OUTPUT(OnStartedBossFight, "OnStart"),

DEFINE_INPUTFUNC(FIELD_VOID, "StartFight", InputStart),

END_DATADESC()

LINK_ENTITY_TO_CLASS(tfo_boss_engager, CBossEngager);

void CBossEngager::OnThink()
{
	CBaseEntity* pTarget = m_hTarget.Get();

	if (pTarget)
	{
		CNPC_Soldier* pSoldier = dynamic_cast<CNPC_Soldier*> (pTarget);
		CNPC_Monster* pMonster = dynamic_cast<CNPC_Monster*> (pTarget);

		static char szMessageName[64];
		int iMaxHealth = 0;
		int iCurrHealth = 0;

		if (pSoldier)
		{
			Q_strncpy(szMessageName, pSoldier->GetEntName(), sizeof(szMessageName));
			iMaxHealth = pSoldier->GetMaxHP();
			iCurrHealth = pSoldier->GetHealth();
		}
		else if (pMonster)
		{
			Q_strncpy(szMessageName, pMonster->GetEntName(), sizeof(szMessageName));
			iMaxHealth = pMonster->GetMaxHP();
			iCurrHealth = pMonster->GetHealth();
		}

		CRecipientFilter user;
		user.AddAllPlayers();
		user.MakeReliable();
		UserMessageBegin(user, "BossData");
		WRITE_BYTE(1);
		WRITE_FLOAT(iCurrHealth);
		WRITE_FLOAT(iMaxHealth);
		WRITE_STRING(szMessageName);
		MessageEnd();
	}
	else // Reset Data
	{
		SetThink(NULL);
		m_hTarget = NULL;

		CRecipientFilter user;
		user.AddAllPlayers();
		user.MakeReliable();
		UserMessageBegin(user, "BossData");
		WRITE_BYTE(0);
		WRITE_FLOAT(0);
		WRITE_FLOAT(1);
		WRITE_STRING("");
		MessageEnd();

		return;
	}

	SetNextThink(gpGlobals->curtime + 0.01);
}

void CBossEngager::InputStart(inputdata_t& input)
{
	m_hTarget = gEntList.FindEntityByName(NULL, STRING(m_szTarget));
	if (m_hTarget.Get())
	{
		SetThink(&CBossEngager::OnThink);
		SetNextThink(gpGlobals->curtime + 0.01);
	}
	else
	{
		m_hTarget = NULL;
		SetThink(NULL);
	}
}