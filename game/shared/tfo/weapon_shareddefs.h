//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Shared Weapon Definitions.
//
//=============================================================================//

#ifndef WEAPON_SHARED_DEFS_H
#define WEAPON_SHARED_DEFS_H

#if defined( _WIN32 )
#pragma once
#endif

#include "in_buttons.h"
#include "npcevent.h"
#include "particle_parse.h"

#ifdef CLIENT_DLL

#include "client_class.h"
#include "basehlcombatweapon_shared.h"
#include "c_basehlcombatweapon.h"
#include "c_baseplayer.h"
#include "c_ai_basenpc.h"

#define CBaseHLCombatWeapon C_BaseHLCombatWeapon
#define CHLSelectFireMachineGun C_HLSelectFireMachineGun
#define CBaseHLBludgeonWeapon C_BaseHLBludgeonWeapon

//#define CWeaponFG42 C_WeaponFG42
//#define CWeaponThompson C_WeaponThompson
//#define CWeaponSTG44 C_WeaponSTG44
//#define CWeaponMP40 C_WeaponMP40
//#define CWeaponSCHIENZEL C_WeaponSCHIENZEL
//#define CWeaponG43 C_WeaponG43
//#define CWeaponSVT40 C_WeaponSVT40
//#define CWeaponMauser C_WeaponMauser
//#define CWeaponFrag C_WeaponFrag
//#define CWeaponRPG C_WeaponRPG
//#define CWeaponP38 C_WeaponP38
//#define CWeaponK98 C_WeaponK98
//#define CWeaponK98NS C_WeaponK98NS
//#define CWeaponHands C_WeaponHands

#define CWeaponLantern C_WeaponLantern
#define CWeaponTorch C_WeaponTorch

#define STUB_WEAPON_CLASS_IMPLEMENT( entityName, className )		\
	BEGIN_PREDICTION_DATA( className )								\
	END_PREDICTION_DATA()											\
	LINK_ENTITY_TO_CLASS( entityName, className );

#define STUB_WEAPON_CLASS( entityName, className, baseClassName )	\
	class C_##className : public baseClassName					\
	{																\
		DECLARE_CLASS( C_##className, baseClassName );							\
	public:															\
		DECLARE_PREDICTABLE();										\
		DECLARE_CLIENTCLASS();										\
		C_##className() {};											\
	private:														\
		C_##className( const C_##className & );						\
	};																\
	STUB_WEAPON_CLASS_IMPLEMENT( entityName, C_##className );		\
	IMPLEMENT_CLIENTCLASS_DT( C_##className, DT_##className, C##className )	\
	END_RECV_TABLE()

#else

#include "basehlcombatweapon.h"
#include "basebludgeonweapon.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"

#endif

#endif // WEAPON_SHARED_DEFS_H