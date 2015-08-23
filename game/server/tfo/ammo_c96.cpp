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

class CC96Ammo : public CItem
{
public:
	DECLARE_CLASS(CC96Ammo, CItem);
	DECLARE_DATADESC();

	CC96Ammo()
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

LINK_ENTITY_TO_CLASS(mauser_ammo, CC96Ammo);

BEGIN_DATADESC(CC96Ammo)
DEFINE_OUTPUT(m_OnUse, "OnUse"),
END_DATADESC()

PRECACHE_REGISTER(mauser_ammo);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CC96Ammo::Spawn(void)
{
	Precache();
	SetModel("models/items/mauser_mag.mdl");
	AddEffects(EF_NOSHADOW | EF_NORECEIVESHADOW);
	BaseClass::Spawn();
}

void CC96Ammo::Precache(void)
{
	PrecacheModel("models/items/mauser_mag.mdl");
}

void CC96Ammo::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!pActivator)
		return;

	if (!pActivator->IsPlayer())
		return;

	CBasePlayer *pPlayer = ToBasePlayer(pActivator);
	if (!pPlayer)
		return;

	if (GiveAmmo(pPlayer, SIZE_AMMO_MAUSER, "Mauser"))
	{
		m_OnUse.FireOutput(this, this);
		EmitSound("Ammo.Pickup2");
		UTIL_Remove(this);
	}
}