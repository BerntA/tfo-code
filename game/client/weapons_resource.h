//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPONS_RESOURCE_H
#define WEAPONS_RESOURCE_H
#pragma once

#include "shareddefs.h"
#include "weapon_parse.h"
#include "utldict.h"
#include "hud.h"

class C_BaseCombatWeapon;
class CHudTexture;

//-----------------------------------------------------------------------------
// Purpose: Stores data about the Weapon Definitions passed to the client when
//			the client first connects to a server. 
//-----------------------------------------------------------------------------
class WeaponsResource
{
public:
	WeaponsResource(void);
	~WeaponsResource(void);

	void Init(void);
	void Reset(void);

	// Sprite handling
	void LoadWeaponSprites(WEAPON_FILE_INFO_HANDLE hWeaponFileInfo);
	void LoadAllWeaponSprites(void);
};

extern WeaponsResource gWR;

#endif // WEAPONS_RESOURCE_H