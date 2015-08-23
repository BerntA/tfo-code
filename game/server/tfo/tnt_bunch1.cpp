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

class CTNTBunch1 : public CItem
{
public:
	DECLARE_CLASS(CTNTBunch1, CItem);
	DECLARE_DATADESC();

	CTNTBunch1()
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

LINK_ENTITY_TO_CLASS(tnt_bunch1, CTNTBunch1);

BEGIN_DATADESC(CTNTBunch1)
DEFINE_OUTPUT(m_OnUse, "OnUse"),
END_DATADESC()

PRECACHE_REGISTER(tnt_bunch1);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTNTBunch1::Spawn(void)
{
	Precache();
	SetModel("models/props_crates/tnt_bunch1.mdl");
	AddEffects(EF_NOSHADOW | EF_NORECEIVESHADOW);

	BaseClass::Spawn();
}

void CTNTBunch1::Precache(void)
{
	PrecacheModel("models/props_crates/tnt_bunch1.mdl");
}

void CTNTBunch1::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!pActivator)
		return;

	if (!pActivator->IsPlayer())
		return;

	m_OnUse.FireOutput(this, this);
	EmitSound("ItemValve.Touch");
	UTIL_Remove(this);
}