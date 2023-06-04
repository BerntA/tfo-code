//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Lantern 
//
//=============================================================================//

#ifndef WEAPON_LANTERN_H
#define WEAPON_LANTERN_H

#include "basebludgeonweapon.h"

#if defined( _WIN32 )
#pragma once
#endif

class CWeaponLantern : public CBaseHLBludgeonWeapon
{
public:
	DECLARE_CLASS(CWeaponLantern, CBaseHLBludgeonWeapon);

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponLantern();

	float		GetRange(void) { return tfo_push_range.GetFloat(); }
	float		GetFireRate(void) { return	0.5f; }

	bool HasIronsights() { return false; }
	bool IsLightSource(void) { return true; }

	void		AddViewKick(void);
	float		GetDamageForActivity(Activity hitActivity);

	virtual int WeaponMeleeAttack1Condition(float flDot, float flDist);

	virtual void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

private:
	void HandleAnimEventMeleeHit(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
};

#endif // WEAPON_LANTERN_H
