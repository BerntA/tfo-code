//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Torch 
//
//=============================================================================//

#ifndef WEAPON_TORCH_H
#define WEAPON_TORCH_H

#include "basebludgeonweapon.h"

#if defined( _WIN32 )
#pragma once
#endif

class CWeaponTorch : public CBaseHLBludgeonWeapon
{
public:
	DECLARE_CLASS(CWeaponTorch, CBaseHLBludgeonWeapon);

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();
	DECLARE_DATADESC();

	CWeaponTorch();

	float		GetRange(void) { return tfo_push_range.GetFloat(); }
	float		GetFireRate(void) { return	0.5f; }

	bool HasIronsights() { return false; }
	int     GetWeaponDamageType(void) { return DMG_BURN; }

	void		AddViewKick(void);
	float		GetDamageForActivity(Activity hitActivity);

	virtual int WeaponMeleeAttack1Condition(float flDot, float flDist);

	virtual void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	void ItemPostFrame(void);

private:
	void HandleAnimEventMeleeHit(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	float m_flTorchLifeTime;
	bool m_bWasActive;
};

#endif // WEAPON_TORCH_H
