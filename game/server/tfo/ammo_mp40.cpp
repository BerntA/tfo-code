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

class CMP40Ammo : public CItem
{
public:
	DECLARE_CLASS(CMP40Ammo, CItem);
	DECLARE_DATADESC();

	CMP40Ammo()
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

LINK_ENTITY_TO_CLASS(mp40_ammo, CMP40Ammo);

BEGIN_DATADESC(CMP40Ammo)
DEFINE_OUTPUT(m_OnUse, "OnUse"),
END_DATADESC()

PRECACHE_REGISTER(mp40_ammo);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMP40Ammo::Spawn(void)
{
	Precache();
	SetModel("models/items/mp_40_mag.mdl");
	AddEffects(EF_NOSHADOW | EF_NORECEIVESHADOW);
	BaseClass::Spawn();
}

void CMP40Ammo::Precache(void)
{
	PrecacheModel("models/items/mp_40_mag.mdl");
}

void CMP40Ammo::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!pActivator)
		return;

	if (!pActivator->IsPlayer())
		return;

	CBasePlayer *pPlayer = ToBasePlayer(pActivator);
	if (!pPlayer)
		return;

	if (GiveAmmo(pPlayer, SIZE_AMMO_MP40, "MP40"))
	{
		m_OnUse.FireOutput(this, this);
		EmitSound("Ammo.Pickup2");
		UTIL_Remove(this);
	}
}