//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Healthkit Entity
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

class CHealthKitTFO : public CItem
{
public:
	DECLARE_CLASS(CHealthKitTFO, CItem);
	DECLARE_DATADESC();

	CHealthKitTFO()
	{
		color32 col32 = { 25, 255, 25, 220 };
		m_GlowColor = col32;
	}

	void Spawn(void);
	void Precache(void);

	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

private:

	COutputEvent m_OnUse;
};

LINK_ENTITY_TO_CLASS(healthkit, CHealthKitTFO);

BEGIN_DATADESC(CHealthKitTFO)

DEFINE_OUTPUT(m_OnUse, "OnUse"),

END_DATADESC()
PRECACHE_REGISTER(healthkit);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHealthKitTFO::Spawn(void)
{
	Precache();
	SetModel("models/items/healthkit.mdl");
	AddEffects(EF_NOSHADOW | EF_NORECEIVESHADOW);

	BaseClass::Spawn();
}

void CHealthKitTFO::Precache(void)
{
	PrecacheModel("models/items/healthkit.mdl");
}

void CHealthKitTFO::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!pActivator)
		return;

	if (!pActivator->IsPlayer())
		return;

	CBasePlayer *pClient = ToBasePlayer(pActivator);
	if (!pClient)
		return;

	if (!pClient->m_bHasHealthkit)
	{
		TransmitPickup(pClient); // Send item pickup notification.

		m_OnUse.FireOutput(this, this);
		EmitSound("ItemTfokit.Touch");
		pClient->m_bHasHealthkit = true;
		UTIL_Remove(this);
	}
	else
		EmitSound("ItemPickup.Reject"); // Only one healthkit is allowed at this time.
}