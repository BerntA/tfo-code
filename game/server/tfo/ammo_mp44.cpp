//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Ammo Entity
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

class CMP44Ammo : public CItem
{
public:
	DECLARE_CLASS(CMP44Ammo, CItem);
	DECLARE_DATADESC();

	CMP44Ammo()
	{
		color32 col32 = { 150, 100, 100, 180 };
		m_GlowColor = col32;
	}

	void Spawn(void);
	void Precache(void);

	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

private:

	COutputEvent	m_OnUse;

};

LINK_ENTITY_TO_CLASS(mp44_ammo, CMP44Ammo);

BEGIN_DATADESC(CMP44Ammo)
DEFINE_OUTPUT(m_OnUse, "OnUse"),
END_DATADESC()

PRECACHE_REGISTER(mp44_ammo);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMP44Ammo::Spawn(void)
{
	Precache();
	SetModel("models/items/stg44_mag.mdl");
	AddEffects(EF_NOSHADOW);
	BaseClass::Spawn();
}

void CMP44Ammo::Precache(void)
{
	PrecacheModel("models/items/stg44_mag.mdl");
}

void CMP44Ammo::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!pActivator)
		return;

	if (!pActivator->IsPlayer())
		return;

	CBasePlayer *pPlayer = ToBasePlayer(pActivator);
	if (!pPlayer)
		return;

	if (GiveAmmo(pPlayer, SIZE_AMMO_STG44, "STG44"))
	{
		m_OnUse.FireOutput(this, this);
		EmitSound("Ammo.Pickup2");
		UTIL_Remove(this);
	}
}