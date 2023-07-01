//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: TNT Entity 
// TODO: Add all three tnt bunches into 1 entity with model selection instead.
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

class CTNTBunch3 : public CItem
{
public:
	DECLARE_CLASS(CTNTBunch3, CItem);

	CTNTBunch3()
	{
		color32 col32 = { 125, 40, 20, 220 };
		m_GlowColor = col32;
	}

	void Spawn(void);
	void Precache(void);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
};

LINK_ENTITY_TO_CLASS(tnt_bunch3, CTNTBunch3);
PRECACHE_REGISTER(tnt_bunch3);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTNTBunch3::Spawn(void)
{
	Precache();
	SetModel("models/props_crates/tnt_bunch3.mdl");
	BaseClass::Spawn();
	AddEffects(EF_NOSHADOW | EF_NORECEIVESHADOW);
}

void CTNTBunch3::Precache(void)
{
	PrecacheModel("models/props_crates/tnt_bunch3.mdl");
}

void CTNTBunch3::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!pActivator || !pActivator->IsPlayer())
		return;

	m_OnUse.FireOutput(pActivator, this);
	EmitSound("ItemValve.Touch");
	UTIL_Remove(this);
}