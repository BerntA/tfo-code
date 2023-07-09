//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Lantern - Light Source
//
//=============================================================================//

#include "cbase.h"
#include "weapon_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_dmg_lantern("sk_dmg_lantern", "0");

class CWeaponLantern : public CBaseHLBludgeonWeapon
{
public:
	DECLARE_CLASS(CWeaponLantern, CBaseHLBludgeonWeapon);

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponLantern();

	float		GetRange(void) { return tfo_push_range.GetFloat(); }
	float		GetFireRate(void) { return	0.5f; }

	bool		HasIronsights() { return false; }
	bool		IsLightSource(void) { return true; }

	void		AddViewKick(void);
	float		GetDamageForActivity(Activity hitActivity);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponLantern, DT_WeaponLantern)

BEGIN_NETWORK_TABLE(CWeaponLantern, DT_WeaponLantern)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponLantern)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_lantern, CWeaponLantern);
PRECACHE_WEAPON_REGISTER(weapon_lantern);

acttable_t	CWeaponLantern::m_acttable[] =
{
	{ ACT_IDLE, ACT_IDLE_PISTOL, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_PISTOL, true },
	{ ACT_RELOAD, ACT_RELOAD_PISTOL, true },
	{ ACT_WALK_AIM, ACT_WALK_AIM_PISTOL, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_PISTOL, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_MELEE_ATTACK1, true },
	{ ACT_RELOAD_LOW, ACT_RELOAD_PISTOL_LOW, false },
	{ ACT_COVER_LOW, ACT_COVER_PISTOL_LOW, false },
	{ ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_PISTOL_LOW, false },
	{ ACT_WALK, ACT_WALK_PISTOL, false },
	{ ACT_RUN, ACT_RUN_PISTOL, false },

	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_PISTOL, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_PISTOL, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_PISTOL, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_PISTOL, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_PISTOL, false },
	{ ACT_MELEE_ATTACK1, ACT_MELEE_ATTACK1, true },
	{ ACT_RANGE_ATTACK1, ACT_MELEE_ATTACK1, true },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_MELEE_ATTACK1, false },
};

IMPLEMENT_ACTTABLE(CWeaponLantern);

CWeaponLantern::CWeaponLantern(void)
{
}

float CWeaponLantern::GetDamageForActivity(Activity hitActivity)
{
	return sk_dmg_lantern.GetFloat();
}

void CWeaponLantern::AddViewKick(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	QAngle punchAng;

	punchAng.x = random->RandomFloat(1.0f, 2.0f);
	punchAng.y = random->RandomFloat(-2.0f, -1.0f);
	punchAng.z = 0.0f;

	pPlayer->ViewPunch(punchAng);
}