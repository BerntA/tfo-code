//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Tommygun - Thompson with drum mag.
//
//=============================================================================//

#include "cbase.h"
#include "weapon_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponThompson : public CHLSelectFireMachineGun
{
public:
	DECLARE_CLASS(CWeaponThompson, CHLSelectFireMachineGun);
	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponThompson();

	void	AddViewKick(void);
	void	ItemPostFrame(void);

	int		GetMinBurst() { return 4; }
	int		GetMaxBurst() { return 6; }
	int     GetOverloadCapacity() { return 15; }
	float	GetFireRate(void) { return 0.100f; }

	bool	Reload(void);

	const Vector& GetBulletSpread(void)
	{
		static Vector cone;

		if (m_bIsIronsighted)
			cone = VECTOR_CONE_2DEGREES;
		else
			cone = VECTOR_CONE_7DEGREES;

		return cone;
	}

	const WeaponProficiencyInfo_t* GetProficiencyValues();
};

IMPLEMENT_SERVERCLASS_ST(CWeaponThompson, DT_WeaponThompson)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_thompson, CWeaponThompson);
PRECACHE_WEAPON_REGISTER(weapon_thompson);

acttable_t	CWeaponThompson::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG1, true },
	{ ACT_RELOAD, ACT_RELOAD_SMG1, true },
	{ ACT_IDLE, ACT_IDLE_SMG1, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SMG1, true },

	{ ACT_WALK, ACT_WALK_RIFLE, true },
	{ ACT_WALK_AIM, ACT_WALK_AIM_RIFLE, true },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED, ACT_IDLE_SMG1_RELAXED, false },//never aims
	{ ACT_IDLE_STIMULATED, ACT_IDLE_SMG1_STIMULATED, false },
	{ ACT_IDLE_AGITATED, ACT_IDLE_ANGRY_SMG1, false },//always aims

	{ ACT_WALK_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_STIMULATED, ACT_WALK_RIFLE_STIMULATED, false },
	{ ACT_WALK_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_STIMULATED, ACT_RUN_RIFLE_STIMULATED, false },
	{ ACT_RUN_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims

	// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED, ACT_IDLE_SMG1_RELAXED, false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED, ACT_IDLE_AIM_RIFLE_STIMULATED, false },
	{ ACT_IDLE_AIM_AGITATED, ACT_IDLE_ANGRY_SMG1, false },//always aims

	{ ACT_WALK_AIM_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_AIM_STIMULATED, ACT_WALK_AIM_RIFLE_STIMULATED, false },
	{ ACT_WALK_AIM_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_AIM_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_AIM_STIMULATED, ACT_RUN_AIM_RIFLE_STIMULATED, false },
	{ ACT_RUN_AIM_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims
	//End readiness activities

	{ ACT_WALK_AIM, ACT_WALK_AIM_RIFLE, true },
	{ ACT_WALK_CROUCH, ACT_WALK_CROUCH_RIFLE, true },
	{ ACT_WALK_CROUCH_AIM, ACT_WALK_CROUCH_AIM_RIFLE, true },
	{ ACT_RUN, ACT_RUN_RIFLE, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_RIFLE, true },
	{ ACT_RUN_CROUCH, ACT_RUN_CROUCH_RIFLE, true },
	{ ACT_RUN_CROUCH_AIM, ACT_RUN_CROUCH_AIM_RIFLE, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_SMG1, true },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SMG1_LOW, true },
	{ ACT_COVER_LOW, ACT_COVER_SMG1_LOW, false },
	{ ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_SMG1_LOW, false },
	{ ACT_RELOAD_LOW, ACT_RELOAD_SMG1_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, true },

	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_SMG1, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_SMG1, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_SMG1, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_SMG1, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_SMG1, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_SMG1, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG1, false },
};

IMPLEMENT_ACTTABLE(CWeaponThompson);

CWeaponThompson::CWeaponThompson()
{
	m_fMinRange1 = 0;
	m_fMaxRange1 = 3000;

	m_bMagazineStyleReloads = true; // Magazine style reloads
	m_bAltFiresUnderwater = false;
}

bool CWeaponThompson::Reload(void)
{
	float fCacheTime = m_flNextSecondaryAttack;

	bool fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
	if (fRet)
	{
		m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;
		WeaponSound(RELOAD);
		EjectClipFx();
	}

	return fRet;
}

void CWeaponThompson::AddViewKick(void)
{
#define	EASY_DAMPEN			0.5f
#define	MAX_VERTICAL_KICK	1.0f	//Degrees
#define	SLIDE_LIMIT			2.0f	//Seconds

	//Get the view kick
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	DoMachineGunKick(pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT);
}

void CWeaponThompson::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();
	DiscardStaleAmmo();
}

const WeaponProficiencyInfo_t* CWeaponThompson::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0, 0.75 },
		{ 5.00, 0.75 },
		{ 10.0 / 3.0, 0.75 },
		{ 5.0 / 3.0, 0.75 },
		{ 1.00, 1.0 },
	};

	COMPILE_TIME_ASSERT(ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}