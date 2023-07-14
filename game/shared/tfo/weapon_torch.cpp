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

#define TORCH_ACTION_CREATE_FIRE 1
#define TORCH_ACTION_DELETE_FIRE 2

class CWeaponTorch : public CBaseHLBludgeonWeapon
{
public:
	DECLARE_CLASS(CWeaponTorch, CBaseHLBludgeonWeapon);

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponTorch();

	float	GetRange(void) { return tfo_push_range.GetFloat(); }
	float	GetFireRate(void) { return	0.5f; }

	bool	HasIronsights() { return false; }
	bool	IsLightSource(void) { return true; }
	int     GetWeaponDamageType(void) { return DMG_BURN; }

	void	AddViewKick(void);
	float	GetDamageForActivity(Activity hitActivity) { return sk_dmg_torch.GetFloat(); }

	void	ClearParticles(void);
	void	UpdateOnRemove(void);
	bool	Deploy(void);
	void	OnRestore();

#ifdef CLIENT_DLL
	void ReceiveMessage(int classID, bf_read& msg);
	void CreateFireEffect(void);
	void DeleteFireEffect(void);
#else
	void SendTorchAction(int action);
#endif

private:
#ifdef CLIENT_DLL
	CNewParticleEffect* m_pParticleFire;
#endif
};

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
#ifdef CLIENT_DLL
	m_pParticleFire = NULL;
#endif
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

void CWeaponTorch::ClearParticles(void)
{
	BaseClass::ClearParticles();

#ifdef GAME_DLL
	SendTorchAction(TORCH_ACTION_DELETE_FIRE);
#endif
}

void CWeaponTorch::UpdateOnRemove(void)
{
	BaseClass::UpdateOnRemove();

#ifdef CLIENT_DLL
	DeleteFireEffect();
#endif
}

bool CWeaponTorch::Deploy(void)
{
	const bool ret = BaseClass::Deploy();

	if (ret)
	{
#ifdef GAME_DLL
		SendTorchAction(TORCH_ACTION_CREATE_FIRE);
#endif
	}

	return ret;
}

void CWeaponTorch::OnRestore()
{
	BaseClass::OnRestore();

#ifdef GAME_DLL
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if ((pPlayer == NULL) || (pPlayer->GetActiveWeapon() != this))
		return;

	SendTorchAction(TORCH_ACTION_CREATE_FIRE);
#endif
}

#ifdef CLIENT_DLL
void CWeaponTorch::ReceiveMessage(int classID, bf_read& msg)
{
	if (classID != GetClientClass()->m_ClassID)
	{
		// message is for subclass
		BaseClass::ReceiveMessage(classID, msg);
		return;
	}

	const int action = msg.ReadByte();

	switch (action)
	{
	case TORCH_ACTION_CREATE_FIRE:
		CreateFireEffect();
		break;

	case TORCH_ACTION_DELETE_FIRE:
		DeleteFireEffect();
		break;
	}
}

void CWeaponTorch::CreateFireEffect(void)
{
	if (m_pParticleFire != NULL)
		return;

	C_BasePlayer* pPlayer = ToBasePlayer(GetOwner());
	C_BaseViewModel* pVM = (pPlayer ? pPlayer->GetViewModel() : NULL);

	if (pVM == NULL || pVM->ParticleProp() == NULL)
		return;

	m_pParticleFire = pVM->ParticleProp()->Create("torch", PATTACH_POINT_FOLLOW, "Flame");
}

void CWeaponTorch::DeleteFireEffect(void)
{
	if (m_pParticleFire == NULL)
		return;

	::ParticleMgr()->RemoveEffect(m_pParticleFire);
	m_pParticleFire = NULL;
}
#else
void CWeaponTorch::SendTorchAction(int action)
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	EntityMessageBegin(this, true);
	WRITE_BYTE(action);
	MessageEnd();
}
#endif