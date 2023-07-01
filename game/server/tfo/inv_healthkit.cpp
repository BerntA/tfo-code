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

	CHealthKitTFO()
	{
		color32 col32 = { 25, 255, 25, 220 };
		m_GlowColor = col32;
	}

	void Spawn(void);
	void Precache(void);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
};

LINK_ENTITY_TO_CLASS(healthkit, CHealthKitTFO);
PRECACHE_REGISTER(healthkit);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHealthKitTFO::Spawn(void)
{
	Precache();
	SetModel("models/items/healthkit.mdl");
	BaseClass::Spawn();
	AddEffects(EF_NOSHADOW | EF_NORECEIVESHADOW);
}

void CHealthKitTFO::Precache(void)
{
	PrecacheModel("models/items/healthkit.mdl");
}

void CHealthKitTFO::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!pActivator || !pActivator->IsPlayer())
		return;

	CBasePlayer* pClient = ToBasePlayer(pActivator);
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