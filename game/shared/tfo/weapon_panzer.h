//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Panzershreck
//
//=============================================================================//

#ifndef WEAPON_RPG_H
#define WEAPON_RPG_H

#ifdef _WIN32
#pragma once
#endif

#include "weapon_shareddefs.h"

class RocketTrail;

#ifdef GAME_DLL
class CMissile : public CBaseCombatCharacter
{
	DECLARE_CLASS(CMissile, CBaseCombatCharacter);

public:
	static const int EXPLOSION_RADIUS = 200;

	CMissile();
	~CMissile();

	Class_T Classify(void) { return CLASS_MISSILE; }

	void	Spawn(void);
	void	Precache(void);
	void	MissileTouch(CBaseEntity* pOther);
	void	Explode(void);
	void	ShotDown(void);
	void	AccelerateThink(void);
	void	AugerThink(void);
	void	IgniteThink(void);

	// Create a dumb think method. This should make it so the rpg obeys gravity.
	void    DumbThink(void);

	void	SetGracePeriod(float flGracePeriod);

	int		OnTakeDamage_Alive(const CTakeDamageInfo& info);
	void	Event_Killed(const CTakeDamageInfo& info);

	virtual float	GetDamage() { return m_flDamage; }
	virtual void	SetDamage(float flDamage) { m_flDamage = flDamage; }

	unsigned int	PhysicsSolidMaskForEntity(void) const;

	static CMissile* Create(const Vector& vecOrigin, const QAngle& vecAngles, edict_t* pentOwner);

	void CreateDangerSounds(bool bState) { m_bCreateDangerSounds = bState; }

protected:
	virtual void DoExplosion();
	virtual int AugerHealth() { return m_iMaxHealth - 20; }

	void CreateSmokeTrail(void);

	CHandle<RocketTrail>	m_hRocketTrail;
	float					m_flAugerTime;		// Amount of time to auger before blowing up anyway
	float					m_flMarkDeadTime;
	float					m_flDamage;

private:
	float					m_flGracePeriodEndsAt;
	bool					m_bCreateDangerSounds;

	DECLARE_DATADESC();
};
#endif

class CWeaponRPG : public CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS(CWeaponRPG, CBaseHLCombatWeapon);

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

	CWeaponRPG();
	~CWeaponRPG();

	void	Precache(void);

	void	PrimaryAttack(void);
	float	GetFireRate(void) { return 1; }
	void	ItemPostFrame(void);

	void	DecrementAmmo(CBaseCombatCharacter* pOwner);
	void    CheckReload(void);

	bool	Reload(void);
	bool	WeaponShouldBeLowered(void);

	bool	IsRocketLauncher(void) { return true; }

	int		GetMinBurst() { return 1; }
	int		GetMaxBurst() { return 1; }

	float	GetMinRestTime() { return 4.0; }
	float	GetMaxRestTime() { return 4.0; }

#ifdef GAME_DLL
	bool	WeaponLOSCondition(const Vector& ownerPos, const Vector& targetPos, bool bSetConditions);
	int		WeaponRangeAttack1Condition(float flDot, float flDist);
	void	Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator);
#endif

	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	const Vector& GetBulletSpread(void)
	{
		static Vector cone = VECTOR_CONE_1DEGREES;
		return cone;
	}

protected:
	bool m_bWantsReload;
};

#endif // WEAPON_RPG_H