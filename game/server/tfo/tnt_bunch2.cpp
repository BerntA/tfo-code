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

class CTNTBunch2 : public CItem
{
public:
	DECLARE_CLASS(CTNTBunch2, CItem);
	DECLARE_DATADESC();

	CTNTBunch2()
	{
		color32 col32 = { 125, 40, 20, 220 };
		m_GlowColor = col32;
	}

	void Spawn(void);
	void Precache(void);

	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

private:

	COutputEvent	m_OnUse;

};

LINK_ENTITY_TO_CLASS(tnt_bunch2, CTNTBunch2);

BEGIN_DATADESC(CTNTBunch2)
DEFINE_OUTPUT(m_OnUse, "OnUse"),
END_DATADESC()

PRECACHE_REGISTER(tnt_bunch2);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTNTBunch2::Spawn(void)
{
	Precache();
	SetModel("models/props_crates/tnt_bunch2.mdl");
	AddEffects(EF_NOSHADOW | EF_NORECEIVESHADOW);

	BaseClass::Spawn();
}

void CTNTBunch2::Precache(void)
{
	PrecacheModel("models/props_crates/tnt_bunch2.mdl");
}

void CTNTBunch2::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!pActivator)
		return;

	if (!pActivator->IsPlayer())
		return;

	m_OnUse.FireOutput(this, this);
	EmitSound("ItemValve.Touch");
	UTIL_Remove(this);
}