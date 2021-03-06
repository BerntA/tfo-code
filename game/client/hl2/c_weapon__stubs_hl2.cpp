//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "c_weapon__stubs.h"
#include "basehlcombatweapon_shared.h"
#include "c_basehlcombatweapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

STUB_WEAPON_CLASS( cycler_weapon, WeaponCycler, C_BaseCombatWeapon );

STUB_WEAPON_CLASS( weapon_binoculars, WeaponBinoculars, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_flaregun, Flaregun, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_annabelle, WeaponAnnabelle, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_gauss, WeaponGaussGun, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_cubemap, WeaponCubemap, C_BaseCombatWeapon );
STUB_WEAPON_CLASS( weapon_alyxgun, WeaponAlyxGun, C_HLSelectFireMachineGun );
STUB_WEAPON_CLASS( weapon_citizenpackage, WeaponCitizenPackage, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_citizensuitcase, WeaponCitizenSuitcase, C_WeaponCitizenPackage );

#ifndef HL2MP
STUB_WEAPON_CLASS( weapon_stiel, WeaponFrag, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_panzer, WeaponRPG, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_p38, WeaponP38, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_k98, WeaponK98, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_k98ns, WeaponK98NS, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_mp40, WeaponMP40, C_HLSelectFireMachineGun );
STUB_WEAPON_CLASS(weapon_thompson, WeaponThompson, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_lantern, WeaponLantern, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_torch, WeaponTorch, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS( weapon_stg44, WeaponSTG44, C_HLSelectFireMachineGun );
STUB_WEAPON_CLASS( weapon_schienzel, WeaponSCHIENZEL, C_HLSelectFireMachineGun );
STUB_WEAPON_CLASS( weapon_fg42, WeaponFG42, C_HLSelectFireMachineGun );
STUB_WEAPON_CLASS( weapon_g43, WeaponG43, C_HLSelectFireMachineGun );
STUB_WEAPON_CLASS( weapon_svt40, WeaponSVT40, C_HLSelectFireMachineGun );
STUB_WEAPON_CLASS( weapon_mauser, WeaponMauser, C_HLSelectFireMachineGun );
STUB_WEAPON_CLASS( weapon_crossbow, WeaponCrossbow, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_slam, Weapon_SLAM, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_hands, WeaponHands, C_BaseHLBludgeonWeapon );
#ifdef HL2_EPISODIC
STUB_WEAPON_CLASS( weapon_hopwire, WeaponHopwire, C_BaseHLCombatWeapon );
#endif
#ifdef HL2_LOSTCOAST
STUB_WEAPON_CLASS( weapon_oldmanharpoon, WeaponOldManHarpoon, C_WeaponCitizenPackage );
#endif
#endif


