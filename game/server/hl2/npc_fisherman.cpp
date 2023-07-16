//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
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
#include "entitylist.h"
#include "activitylist.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "sceneentity.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define FISHERMAN_MODEL "models/lostcoast/fisherman/fisherman.mdl"

class CNPC_Fisherman : public CNPC_PlayerCompanion
{
public:
	DECLARE_CLASS( CNPC_Fisherman, CNPC_PlayerCompanion );
	DECLARE_DATADESC();

	virtual void Precache()
	{
		// Prevents a warning
		SelectModel();
		BaseClass::Precache();

		PrecacheScriptSound( "NPC_Fisherman.FootstepLeft" );
		PrecacheScriptSound( "NPC_Fisherman.FootstepRight" );
		PrecacheScriptSound( "NPC_Fisherman.Die" );
	}

	void	Spawn( void );
	void	SelectModel();
	Class_T Classify( void );

	void HandleAnimEvent( animevent_t *pEvent );

	bool ShouldLookForBetterWeapon() { return false; }
	bool IgnorePlayerPushing( void ) { return true; }
	void DeathSound( const CTakeDamageInfo &info );

	DEFINE_CUSTOM_AI;
};

LINK_ENTITY_TO_CLASS( npc_fisherman, CNPC_Fisherman );

BEGIN_DATADESC( CNPC_Fisherman )
END_DATADESC()

void CNPC_Fisherman::SelectModel()
{
	SetModelName( AllocPooledString( FISHERMAN_MODEL ) );
}

void CNPC_Fisherman::Spawn( void )
{
	Precache();

	m_iHealth = 80;

	BaseClass::Spawn();

	AddEFlags( EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION );

	NPCInit();
}

Class_T	CNPC_Fisherman::Classify( void )
{
	return	CLASS_PLAYER_ALLY_VITAL;
}

void CNPC_Fisherman::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == NPC_EVENT_LEFTFOOT )
	{
		EmitSound( "NPC_Fisherman.FootstepLeft", pEvent->eventtime );
	}
	else if ( pEvent->event == NPC_EVENT_RIGHTFOOT )
	{
		EmitSound( "NPC_Fisherman.FootstepRight", pEvent->eventtime );
	}
	else
	{
		BaseClass::HandleAnimEvent( pEvent );
	}
}

void CNPC_Fisherman::DeathSound( const CTakeDamageInfo &info )
{
	// Sentences don't play on dead NPCs
	SentenceStop();
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_fisherman, CNPC_Fisherman )
AI_END_CUSTOM_NPC()