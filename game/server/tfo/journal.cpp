//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Save Station - Opens the Save Panel on +USE input.
//
//=============================================================================//

#include "cbase.h"
#include "gamerules.h"
#include "baseanimating.h"
#include "items.h"
#include "hl2_player.h"
#include "basecombatweapon.h"
#include "player.h"
#include "ammodef.h"
#include "npcevent.h"
#include "eventlist.h"

class CJournal : public CItem
{
public:
	DECLARE_CLASS(CJournal, CItem);
	DECLARE_DATADESC();

	CJournal()
	{
		color32 col32 = { 255, 25, 25, 240 };
		m_GlowColor = col32;
	}

	void Spawn(void);
	void Precache(void);

	// Glowing
	bool CanGlow() { return true; }

	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

private:

	COutputEvent	m_OnUse;	// Output when somebody clicks us
};

LINK_ENTITY_TO_CLASS(journal, CJournal);

BEGIN_DATADESC(CJournal)

// Links our input name from Hammer to our input member function
DEFINE_OUTPUT(m_OnUse, "OnUse"),

END_DATADESC()
PRECACHE_REGISTER(journal);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CJournal::Spawn(void)
{
	BaseClass::Spawn();

	Precache();
	SetModel("models/static_props/journal.mdl");

	AddEffects(EF_NOSHADOW | EF_NORECEIVESHADOW);

	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_BBOX);
	AddEFlags(EFL_NO_ROTORWASH_PUSH);
	SetBlocksLOS(false);
	SetCollisionGroup(COLLISION_GROUP_WEAPON);
}

void CJournal::Precache(void)
{
	PrecacheModel("models/static_props/journal.mdl");
}

void CJournal::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!pActivator)
		return;

	if (!pActivator->IsPlayer())
		return;

	CBasePlayer *pClient = ToBasePlayer(pActivator);
	if (!pClient)
		return;

	m_OnUse.FireOutput(this, this);

	// Open journal / save station.
	engine->ClientCommand(pClient->edict(), "tfo_gameui_command OpenSavePanel\n");
}