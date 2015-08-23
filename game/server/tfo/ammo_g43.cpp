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

class CG43Ammo : public CItem
{
public:
	DECLARE_CLASS(CG43Ammo, CItem);
	DECLARE_DATADESC();

	CG43Ammo()
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

LINK_ENTITY_TO_CLASS(g43_ammo, CG43Ammo);

BEGIN_DATADESC(CG43Ammo)
DEFINE_OUTPUT(m_OnUse, "OnUse"),
END_DATADESC()

PRECACHE_REGISTER(g43_ammo);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CG43Ammo::Spawn(void)
{
	Precache();
	SetModel("models/items/g43_mag.mdl");
	AddEffects(EF_NOSHADOW | EF_NORECEIVESHADOW);
	BaseClass::Spawn();
}

void CG43Ammo::Precache(void)
{
	PrecacheModel("models/items/g43_mag.mdl");
}

void CG43Ammo::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!pActivator)
		return;

	if (!pActivator->IsPlayer())
		return;

	CBasePlayer *pPlayer = ToBasePlayer(pActivator);
	if (!pPlayer)
		return;

	if (GiveAmmo(pPlayer, SIZE_AMMO_G43, "G43"))
	{
		m_OnUse.FireOutput(this, this);
		EmitSound("Ammo.Pickup2");
		UTIL_Remove(this);
	}
}