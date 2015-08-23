//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		The class from which all bludgeon melee
//				weapons are derived. 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "basehlcombatweapon.h"

#ifndef BASEBLUDGEONWEAPON_H
#define BASEBLUDGEONWEAPON_H

//=========================================================
// CBaseHLBludgeonWeapon 
//=========================================================
class CBaseHLBludgeonWeapon : public CBaseHLCombatWeapon
{
	DECLARE_CLASS( CBaseHLBludgeonWeapon, CBaseHLCombatWeapon );
public:
	CBaseHLBludgeonWeapon();

	DECLARE_SERVERCLASS();

	virtual	void	Spawn( void );
	virtual	void	Precache( void );
	
	//Attack functions
	virtual	void	PrimaryAttack( void );
	virtual	void	SecondaryAttack( void );
	virtual int     GetWeaponDamageType(void) { return DMG_SLASH; }

	virtual void	ItemPostFrame( void );

	//Functions to select animation sequences 
	virtual Activity	GetPrimaryAttackActivity( void )	{	return	ACT_VM_HITCENTER;	}
	virtual Activity	GetSecondaryAttackActivity( void )	{	return	ACT_VM_HITCENTER2;	}

	virtual	float	GetFireRate( void )								{	return	0.2f;	}
	virtual float	GetRange( void )								{	return	32.0f;	}
	virtual	float	GetDamageForActivity( Activity hitActivity )	{	return	1.0f;	}

	virtual int		CapabilitiesGet( void );
	virtual	int		WeaponMeleeAttack1Condition( float flDot, float flDist );

	void			Swing(void);

protected:
	virtual	void	ImpactEffect( trace_t &trace );
	virtual bool    CanAttack(void);

private:

	bool			ImpactWater( const Vector &start, const Vector &end );
	void			Hit( trace_t &traceHit );
	void     		ChooseIntersectionPointAndActivity( trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner );
};

#endif