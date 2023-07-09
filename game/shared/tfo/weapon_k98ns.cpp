//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: K98 Sniper - Non-Scoped.
//
//=============================================================================//

#include "cbase.h"
#include "weapon_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponK98NS : public CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS(CWeaponK98NS, CBaseHLCombatWeapon);
	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponK98NS(void);

	void	ItemPostFrame(void);
	void	PrimaryAttack(void);

	int     GetOverloadCapacity() { return 2; }
	void	AddViewKick(void);
	void	Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator);
	void    FireNPCPrimaryAttack(CBaseCombatCharacter* pOperator, Vector& vecShootOrigin, Vector& vecShootDir);

	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	bool	Reload(void);

	int		GetMinBurst() { return 1; }
	int		GetMaxBurst() { return 1; }
	float	GetFireRate(void) { return 1.225f; }

	const Vector& GetBulletSpread(void)
	{
		static Vector cone;

		if (m_bIsIronsighted)
			cone = VECTOR_CONE_1DEGREES;
		else
			cone = VECTOR_CONE_7DEGREES;

		return cone;
	}
};

IMPLEMENT_SERVERCLASS_ST(CWeaponK98NS, DT_WeaponK98NS)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_k98ns, CWeaponK98NS);
PRECACHE_WEAPON_REGISTER(weapon_k98ns);

acttable_t	CWeaponK98NS::m_acttable[] =
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

			{ ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_RPG,                    false },
			{ ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_RPG,                    false },
			{ ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_RPG,            false },
			{ ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_RPG,            false },
			{ ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_RPG,    false },
			{ ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_AR2,        false },
			{ ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_RPG,                    false },
			{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_RPG,                false },
};

IMPLEMENT_ACTTABLE(CWeaponK98NS);

CWeaponK98NS::CWeaponK98NS(void)
{
	m_fMinRange1 = 0;
	m_fMaxRange1 = 1500;
	m_fMinRange2 = 0;
	m_fMaxRange2 = 200;

	m_bMagazineStyleReloads = true; // Magazine style reloads
	m_bFiresUnderwater = true;
}

void CWeaponK98NS::Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_PISTOL_FIRE:
	case EVENT_WEAPON_SMG1:
	case EVENT_WEAPON_AR1:
	case EVENT_WEAPON_AR2:
	{
		Vector vecShootOrigin, vecShootDir;
		vecShootOrigin = pOperator->Weapon_ShootPosition();

		CAI_BaseNPC* npc = pOperator->MyNPCPointer();
		ASSERT(npc != NULL);

		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);

		CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());

		WeaponSound(SINGLE_NPC);
		pOperator->FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);
		pOperator->DoMuzzleFlash();
		m_iClip1 = m_iClip1 - 1;
	}
	break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

void CWeaponK98NS::FireNPCPrimaryAttack(CBaseCombatCharacter* pOperator, Vector& vecShootOrigin, Vector& vecShootDir)
{
	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());
	pOperator->FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED,
		MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0);

	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;
}

void CWeaponK98NS::PrimaryAttack(void)
{
	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, GetOwner());
	BaseClass::PrimaryAttack();

	m_iPrimaryAttacks++;
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
}

void CWeaponK98NS::ItemPostFrame(void)
{
	if (m_flNextPrimaryAttack > gpGlobals->curtime)
		return;

	BaseClass::ItemPostFrame();
	DiscardStaleAmmo();
}

bool CWeaponK98NS::Reload(void)
{
	bool fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
	if (fRet)
	{
		WeaponSound(RELOAD);
		EjectClipFx();
	}
	return fRet;
}

void CWeaponK98NS::AddViewKick(void)
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	QAngle	viewPunch;

	viewPunch.x = random->RandomFloat(0.25f, 0.5f);
	viewPunch.y = random->RandomFloat(-.6f, .6f);
	viewPunch.z = 0.0f;

	//Add it to the view punch
	pPlayer->ViewPunch(viewPunch);
}