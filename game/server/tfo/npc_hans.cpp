//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Hans - Ally NPC
//
//=============================================================================//

#include "cbase.h"
#include "ai_default.h"
#include "ai_task.h"
#include "ai_schedule.h"
#include "ai_node.h"
#include "ai_hull.h"
#include "ai_hint.h"
#include "ai_squad.h"
#include "ai_senses.h"
#include "ai_navigator.h"
#include "ai_motor.h"
#include "ai_behavior.h"
#include "ai_baseactor.h"
#include "ai_behavior_lead.h"
#include "ai_behavior_follow.h"
#include "ai_behavior_standoff.h"
#include "ai_behavior_assault.h"
#include "npc_playercompanion.h"
#include "soundent.h"
#include "game.h"
#include "npcevent.h"
#include "activitylist.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "sceneentity.h"
#include "ai_behavior_functank.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define HANS_MODEL "models/characters/hans.mdl"

ConVar	sk_friend_hans_health("sk_friend_hans_health", "0");

class CNPC_Hans : public CNPC_PlayerCompanion
{
public:
	DECLARE_CLASS(CNPC_Hans, CNPC_PlayerCompanion);
	DECLARE_DATADESC();

	void Precache()
	{
		// Prevents a warning
		SelectModel();
		BaseClass::Precache();

		PrecacheScriptSound("NPC_Barney.FootstepLeft");
		PrecacheScriptSound("NPC_Barney.FootstepRight");
		PrecacheScriptSound("NPC_Barney.Die");
	}

	void	Spawn(void);
	void	SelectModel();
	Class_T Classify(void);

	bool CreateBehaviors(void);

	void HandleAnimEvent(animevent_t* pEvent);

	bool ShouldLookForBetterWeapon() { return false; }

	void OnChangeRunningBehavior(CAI_BehaviorBase* pOldBehavior, CAI_BehaviorBase* pNewBehavior);

	void DeathSound(const CTakeDamageInfo& info);
	void GatherConditions();
	void UseFunc(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	CAI_FuncTankBehavior		m_FuncTankBehavior;
	COutputEvent				m_OnPlayerUse;

	DEFINE_CUSTOM_AI;
};

LINK_ENTITY_TO_CLASS(npc_hans, CNPC_Hans);

BEGIN_DATADESC(CNPC_Hans)
DEFINE_OUTPUT(m_OnPlayerUse, "OnPlayerUse"),
DEFINE_USEFUNC(UseFunc),
END_DATADESC()

void CNPC_Hans::SelectModel()
{
	SetModelName(AllocPooledString(HANS_MODEL));
}

void CNPC_Hans::Spawn(void)
{
	Precache();

	m_iHealth = sk_friend_hans_health.GetFloat();

	BaseClass::Spawn();

	AddEFlags(EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION);

	NPCInit();

	SetUse(&CNPC_Hans::UseFunc);
}

Class_T	CNPC_Hans::Classify(void)
{
	return	CLASS_PLAYER_ALLY_VITAL;
}

void CNPC_Hans::HandleAnimEvent(animevent_t* pEvent)
{
	switch (pEvent->event)
	{
	case NPC_EVENT_LEFTFOOT:
	{
		EmitSound("NPC_Barney.FootstepLeft", pEvent->eventtime);
	}
	break;
	case NPC_EVENT_RIGHTFOOT:
	{
		EmitSound("NPC_Barney.FootstepRight", pEvent->eventtime);
	}
	break;

	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}

void CNPC_Hans::DeathSound(const CTakeDamageInfo& info)
{
	// Sentences don't play on dead NPCs
	SentenceStop();
	EmitSound("npc_barney.die");
}

bool CNPC_Hans::CreateBehaviors(void)
{
	BaseClass::CreateBehaviors();
	AddBehavior(&m_FuncTankBehavior);
	return true;
}

void CNPC_Hans::OnChangeRunningBehavior(CAI_BehaviorBase* pOldBehavior, CAI_BehaviorBase* pNewBehavior)
{
	if (pNewBehavior == &m_FuncTankBehavior)
	{
		m_bReadinessCapable = false;
	}
	else if (pOldBehavior == &m_FuncTankBehavior)
	{
		m_bReadinessCapable = IsReadinessCapable();
	}

	BaseClass::OnChangeRunningBehavior(pOldBehavior, pNewBehavior);
}

void CNPC_Hans::GatherConditions()
{
	BaseClass::GatherConditions();

	// Handle speech AI. Don't do AI speech if we're in scripts unless permitted by the EnableSpeakWhileScripting input.
	if (m_NPCState == NPC_STATE_IDLE || m_NPCState == NPC_STATE_ALERT || m_NPCState == NPC_STATE_COMBAT ||
		((m_NPCState == NPC_STATE_SCRIPT) && CanSpeakWhileScripting()))
	{
		DoCustomSpeechAI();
	}
}

void CNPC_Hans::UseFunc(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	m_bDontUseSemaphore = true;
	SpeakIfAllowed(TLK_USE);
	m_bDontUseSemaphore = false;

	m_OnPlayerUse.FireOutput(pActivator, pCaller);
}

AI_BEGIN_CUSTOM_NPC(npc_hans, CNPC_Hans)
AI_END_CUSTOM_NPC()