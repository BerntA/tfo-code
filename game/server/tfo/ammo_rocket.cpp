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

class CPanzerAmmo : public CItem
{
public:
	DECLARE_CLASS(CPanzerAmmo, CItem);
	DECLARE_DATADESC();

	CPanzerAmmo()
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

LINK_ENTITY_TO_CLASS(panzer_ammo, CPanzerAmmo);

BEGIN_DATADESC(CPanzerAmmo)
DEFINE_OUTPUT(m_OnUse, "OnUse"),
END_DATADESC()

PRECACHE_REGISTER(panzer_ammo);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPanzerAmmo::Spawn(void)
{
	Precache();
	SetModel("models/weapons/w_panzerschreck_rocket.mdl");
	AddEffects(EF_NOSHADOW | EF_NORECEIVESHADOW);
	BaseClass::Spawn();
}

void CPanzerAmmo::Precache(void)
{
	PrecacheModel("models/weapons/w_panzerschreck_rocket.mdl");
}

void CPanzerAmmo::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!pActivator)
		return;

	if (!pActivator->IsPlayer())
		return;

	CBasePlayer *pPlayer = ToBasePlayer(pActivator);
	if (!pPlayer)
		return;

	if (GiveAmmo(pPlayer, SIZE_AMMO_RPG_ROUND, "RPG_Round"))
	{
		m_OnUse.FireOutput(this, this);
		EmitSound("Ammo.Pickup2");
		UTIL_Remove(this);
	}
}