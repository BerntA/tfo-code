//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: FG-42 Donation/End Game Reward Weapon. Unlimited Ammo.
//
//=============================================================================//

#include "cbase.h"
#include "weapon_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponFG42 : public CHLSelectFireMachineGun
{
public:
	DECLARE_CLASS(CWeaponFG42, CHLSelectFireMachineGun);
	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponFG42();

	void	AddViewKick(void);
	bool	Reload(void);
	float	GetFireRate(void) { return 0.100f; }

	const Vector& GetBulletSpread(void)
	{
		static Vector cone;

		if (m_bIsIronsighted)
			cone = VECTOR_CONE_1DEGREES;
		else
			cone = VECTOR_CONE_5DEGREES;

		return cone;
	}

	const WeaponProficiencyInfo_t* GetProficiencyValues();
};

IMPLEMENT_SERVERCLASS_ST(CWeaponFG42, DT_WeaponFG42)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_fg42, CWeaponFG42);
PRECACHE_WEAPON_REGISTER(weapon_fg42);

acttable_t	CWeaponFG42::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_RPG, true },
	{ ACT_IDLE_RELAXED,				ACT_IDLE_RPG_RELAXED,			true },
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_ANGRY_RPG,				true },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_RPG,				true },
	{ ACT_IDLE,						ACT_IDLE_RPG,					true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_RPG,				true },
	{ ACT_WALK,						ACT_WALK_RPG,					true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RPG,			true },
	{ ACT_RUN,						ACT_RUN_RPG,					true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RPG,				true },
	{ ACT_COVER_LOW,				ACT_COVER_LOW_RPG,				true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },		// FIXME: hook to AR2 unique
	{ ACT_WALK_RELAXED,				ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_STIMULATED,			ACT_WALK_RIFLE_STIMULATED,		false },
	{ ACT_WALK_AGITATED,			ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_RELAXED,				ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_STIMULATED,			ACT_RUN_RIFLE_STIMULATED,		false },
	{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false },//always aims

	// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_SMG1_RELAXED,			false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_RIFLE_STIMULATED,	false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_RIFLE_STIMULATED,	false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_RIFLE_STIMULATED,	false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false },//always aims
	//End readiness activities

	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,		true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_AR2,	false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_AR2_LOW,			false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SMG1_LOW,		true },		// FIXME: hook to AR2 unique
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true },
	//	{ ACT_RANGE_ATTACK2, ACT_RANGE_ATTACK_AR2_GRENADE, true },

	{ ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_RPG,                    false },
	{ ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_RPG,                    false },
	{ ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_RPG,            false },
	{ ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_RPG,            false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_RPG,    false },
	{ ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_AR2,        false },
	{ ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_RPG,                    false },
	{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_RPG,                false },
};

IMPLEMENT_ACTTABLE(CWeaponFG42);

CWeaponFG42::CWeaponFG42()
{
	m_fMinRange1 = 0;
	m_fMaxRange1 = 3000;

	m_bMagazineStyleReloads = true;
	m_bAltFiresUnderwater = false;
}

bool CWeaponFG42::Reload(void)
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

void CWeaponFG42::AddViewKick(void)
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

const WeaponProficiencyInfo_t* CWeaponFG42::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0,		0.75	},
		{ 5.00,		0.75	},
		{ 10.0 / 3.0, 0.75	},
		{ 5.0 / 3.0,	0.75	},
		{ 1.00,		1.0		},
	};

	COMPILE_TIME_ASSERT(ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}