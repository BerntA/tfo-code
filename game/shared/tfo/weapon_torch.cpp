//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Torch - Light Source
//
//=============================================================================//

#include "cbase.h"
#include "weapon_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_dmg_torch("sk_dmg_torch", "0");

class CWeaponTorch : public CBaseHLBludgeonWeapon
{
public:
	DECLARE_CLASS(CWeaponTorch, CBaseHLBludgeonWeapon);

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

	CWeaponTorch();

	float	GetRange(void) { return tfo_push_range.GetFloat(); }
	float	GetFireRate(void) { return	0.5f; }

	bool	HasIronsights() { return false; }
	bool	IsLightSource(void) { return true; }
	int     GetWeaponDamageType(void) { return DMG_BURN; }

	void	AddViewKick(void);
	float	GetDamageForActivity(Activity hitActivity);

	void	ItemPreFrame(void);
	void	ItemPostFrame(void);
	void	ItemBusyFrame(void);
	void	ItemHolsterFrame(void);
	void	TorchUpdate(void);

private:
	float m_flTorchLifeTime;
	bool m_bWasActive;
};

#ifdef GAME_DLL
BEGIN_DATADESC(CWeaponTorch)
DEFINE_FIELD(m_bWasActive, FIELD_BOOLEAN),
END_DATADESC()
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponTorch, DT_WeaponTorch)

BEGIN_NETWORK_TABLE(CWeaponTorch, DT_WeaponTorch)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponTorch)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_torch, CWeaponTorch);
PRECACHE_WEAPON_REGISTER(weapon_torch);

acttable_t	CWeaponTorch::m_acttable[] =
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

IMPLEMENT_ACTTABLE(CWeaponTorch);

CWeaponTorch::CWeaponTorch(void)
{
	m_flTorchLifeTime = 0.0f;
	m_bWasActive = false;
}

float CWeaponTorch::GetDamageForActivity(Activity hitActivity)
{
	return sk_dmg_torch.GetFloat();
}

void CWeaponTorch::AddViewKick(void)
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	QAngle punchAng;

	punchAng.x = random->RandomFloat(1.0f, 2.0f);
	punchAng.y = random->RandomFloat(-2.0f, -1.0f);
	punchAng.z = 0.0f;

	pPlayer->ViewPunch(punchAng);
}

void CWeaponTorch::ItemPreFrame(void)
{
	BaseClass::ItemPreFrame();
	TorchUpdate();
}

void CWeaponTorch::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();
	TorchUpdate();
}

void CWeaponTorch::ItemBusyFrame(void)
{
	BaseClass::ItemBusyFrame();
	TorchUpdate();
}

void CWeaponTorch::ItemHolsterFrame(void)
{
	BaseClass::ItemHolsterFrame();
	TorchUpdate();
}

void CWeaponTorch::TorchUpdate(void)
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	CBaseViewModel* vm = (pOwner ? pOwner->GetViewModel() : NULL);

	if (!pOwner || !vm)
		return;

	const bool bIsTorchActive = (pOwner->GetActiveWeapon() == this);

	if (bIsTorchActive)
	{
		const float flTime = engine->Time();

		if (flTime > m_flTorchLifeTime)
		{
			m_flTorchLifeTime = (flTime + 0.2f);
			int iAttachment = vm->LookupAttachment("Flame");
			DispatchParticleEffect("torch", PATTACH_POINT_FOLLOW, vm, iAttachment, true);
			m_bWasActive = true;
		}

		return;
	}

	if (m_bWasActive)
	{
		ClearParticles();
		StopParticleEffects(vm);
		StopParticleEffects(this);
		m_bWasActive = false;
		m_flTorchLifeTime = 0.0f;
	}
}