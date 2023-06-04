//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Default Hands
//
//=============================================================================//

#ifndef WEAPON_HANDS_H
#define WEAPON_HANDS_H

#include "basebludgeonweapon.h"

#if defined( _WIN32 )
#pragma once
#endif

#ifdef HL2MP
#error weapon_hands.h must not be included in hl2mp. The windows compiler will use the wrong class elsewhere if it is.
#endif

#define	HANDS_RANGE	50.0f
#define	HANDS_REFIRE	0.5f

//-----------------------------------------------------------------------------
// CWeaponHands
//-----------------------------------------------------------------------------

class CWeaponHands : public CBaseHLBludgeonWeapon
{
public:
	DECLARE_CLASS(CWeaponHands, CBaseHLBludgeonWeapon);

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponHands();

	float		GetRange(void)		{ return	HANDS_RANGE; }
	float		GetFireRate(void)		{ return	HANDS_REFIRE; }
	bool HasIronsights() { return false; }
	bool IsHands(void) { return true; }
	bool VisibleInWeaponSelection() { return false; }
	bool CanAttack(void) { return true; }

	void		AddViewKick(void);
	float		GetDamageForActivity(Activity hitActivity);

	virtual int WeaponMeleeAttack1Condition(float flDot, float flDist);

	// Animation event
	virtual void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

private:
	// Animation event handlers
	void HandleAnimEventMeleeHit(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
};

#endif // WEAPON_HANDS_H
