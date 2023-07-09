//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Weapon shareddefs and client class defs.
//
//=============================================================================//

#include "cbase.h"
#include "weapon_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
class C_BaseFragWeapon : public C_BaseHLCombatWeapon
{
public:
	DECLARE_CLASS(C_BaseFragWeapon, C_BaseHLCombatWeapon);

	bool	IsGrenade(void) { return true; }
	bool	HasIronsights() { return false; }
	bool	VisibleInWeaponSelection() { return false; }
};

class C_BaseRPGWeapon : public C_BaseHLCombatWeapon
{
public:
	DECLARE_CLASS(C_BaseRPGWeapon, C_BaseHLCombatWeapon);

	bool	IsRocketLauncher(void) { return true; }
};

class C_BasePistolWeapon : public C_BaseHLCombatWeapon
{
public:
	DECLARE_CLASS(C_BasePistolWeapon, C_BaseHLCombatWeapon);

	bool	UseIronsightAnims(void) { return true; }
};

class C_BaseHandWeapon : public C_BaseHLBludgeonWeapon
{
public:
	DECLARE_CLASS(C_BaseHandWeapon, C_BaseHLBludgeonWeapon);

	bool		HasIronsights() { return false; }
	bool		IsHands(void) { return true; }
	bool		VisibleInWeaponSelection() { return false; }
};

STUB_WEAPON_CLASS(weapon_stiel, WeaponFrag, C_BaseFragWeapon);
STUB_WEAPON_CLASS(weapon_panzer, WeaponRPG, C_BaseRPGWeapon);
STUB_WEAPON_CLASS(weapon_p38, WeaponP38, C_BasePistolWeapon);
STUB_WEAPON_CLASS(weapon_hands, WeaponHands, C_BaseHandWeapon);

STUB_WEAPON_CLASS(weapon_k98, WeaponK98, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_k98ns, WeaponK98NS, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_mp40, WeaponMP40, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_thompson, WeaponThompson, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_stg44, WeaponSTG44, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_schienzel, WeaponSchienzel, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_fg42, WeaponFG42, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_g43, WeaponG43, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_svt40, WeaponSVT40, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_mauser, WeaponMauser, C_HLSelectFireMachineGun);
#else
// UNUSED FOR NOW
#endif