//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Mauser C96
//
//=============================================================================//

#include "cbase.h"
#include "weapon_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponMauser : public CHLSelectFireMachineGun
{
public:
	DECLARE_CLASS(CWeaponMauser, CHLSelectFireMachineGun);
	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponMauser();

	void	AddViewKick(void);
	void	ItemPostFrame(void);

	int		GetMinBurst() { return 3; }
	int		GetMaxBurst() { return 6; }
	float	GetFireRate(void) { return 0.075f; }

	bool	Reload(void);

	const Vector& GetBulletSpread(void)
	{
		static Vector cone;

		if (m_bIsIronsighted)
			cone = VECTOR_CONE_1DEGREES;
		else
			cone = VECTOR_CONE_4DEGREES;

		return cone;
	}

	const WeaponProficiencyInfo_t* GetProficiencyValues();
};

IMPLEMENT_SERVERCLASS_ST(CWeaponMauser, DT_WeaponMauser)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_mauser, CWeaponMauser);
PRECACHE_WEAPON_REGISTER(weapon_mauser);

acttable_t	CWeaponMauser::m_acttable[] =
{
	{ ACT_IDLE,						ACT_IDLE_PISTOL,				true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_PISTOL,			true },
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_PISTOL,		true },
	{ ACT_RELOAD,					ACT_RELOAD_PISTOL,				true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_PISTOL,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_PISTOL,				true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_PISTOL,true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_PISTOL_LOW,			false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_PISTOL_LOW,	false },
	{ ACT_COVER_LOW,				ACT_COVER_PISTOL_LOW,			false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_PISTOL_LOW,		false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_PISTOL,		false },
	{ ACT_WALK,						ACT_WALK_PISTOL,				false },
	{ ACT_RUN,						ACT_RUN_PISTOL,					false },

	{ ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_PISTOL,                    false },
	{ ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_PISTOL,                    false },
	{ ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_PISTOL,            false },
	{ ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_PISTOL,            false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,    false },
	{ ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_PISTOL,        false },
	{ ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_PISTOL,                    false },
	{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_PISTOL,                false },
};

IMPLEMENT_ACTTABLE(CWeaponMauser);

CWeaponMauser::CWeaponMauser()
{
	m_fMinRange1 = 0;// No minimum range. 
	m_fMaxRange1 = 2000;

	m_bMagazineStyleReloads = true; // Magazine style reloads
	m_bAltFiresUnderwater = false;
}

bool CWeaponMauser::Reload(void)
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

void CWeaponMauser::AddViewKick(void)
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

void CWeaponMauser::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();
	DiscardStaleAmmo();
}

const WeaponProficiencyInfo_t* CWeaponMauser::GetProficiencyValues()
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