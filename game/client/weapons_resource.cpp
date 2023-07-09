//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Weapons Resource implementation
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "history_resource.h"
#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>
#include "c_baseplayer.h"
#include "hud.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

WeaponsResource gWR;

void FreeHudTextureList( CUtlDict< CHudTexture *, int >& list );

static CHudTexture *FindHudTextureInDict( CUtlDict< CHudTexture *, int >& list, const char *psz )
{
	int idx = list.Find( psz );
	if ( idx == list.InvalidIndex() )
		return NULL;

	return list[ idx ];
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : 
//-----------------------------------------------------------------------------
WeaponsResource::WeaponsResource( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : 
//-----------------------------------------------------------------------------
WeaponsResource::~WeaponsResource( void )
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void WeaponsResource::Init( void )
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void WeaponsResource::Reset( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Load all the sprites needed for all registered weapons
//-----------------------------------------------------------------------------
void WeaponsResource::LoadAllWeaponSprites( void )
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if ( !player )
		return;

	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if ( player->GetWeapon(i) )
		{
			LoadWeaponSprites( player->GetWeapon(i)->GetWeaponFileInfoHandle() );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void WeaponsResource::LoadWeaponSprites( WEAPON_FILE_INFO_HANDLE hWeaponFileInfo )
{
	// WeaponsResource is a friend of C_BaseCombatWeapon
	FileWeaponInfo_t* pWeaponInfo = GetFileWeaponInfoFromHandle(hWeaponFileInfo);

	// NULL or Already parsed the hud elements?
	if (!pWeaponInfo || pWeaponInfo->bLoadedHudElements)
		return;

	pWeaponInfo->bLoadedHudElements = true;

	pWeaponInfo->iconWeapon = NULL;
	pWeaponInfo->iconAmmo = NULL;

	char sz[128];
	Q_snprintf(sz, sizeof( sz ), "scripts/%s", pWeaponInfo->szClassName);

	CUtlDict< CHudTexture *, int > tempList;

	LoadHudTextures( tempList, sz, g_pGameRules->GetEncryptionKey() );

	if ( !tempList.Count() )
	{
		// no sprite description file for weapon, use default small blocks
		pWeaponInfo->iconWeapon = gHUD.GetIcon("selection");
		pWeaponInfo->iconAmmo = gHUD.GetIcon("bucket1");
		return;
	}

	CHudTexture* p = NULL;

	CHudHistoryResource *pHudHR = GET_HUDELEMENT( CHudHistoryResource );	
	if( pHudHR )
	{
		p = FindHudTextureInDict( tempList, "weapon" );
		if ( p )
		{
			pWeaponInfo->iconWeapon = gHUD.AddUnsearchableHudIconToList( *p );
			if ( pWeaponInfo->iconWeapon)
			{
				pWeaponInfo->iconWeapon->Precache();
				pHudHR->SetHistoryGap( pWeaponInfo->iconWeapon->Height() );
			}
		}

		p = FindHudTextureInDict( tempList, "ammo" );
		if ( p )
		{
			pWeaponInfo->iconAmmo = gHUD.AddUnsearchableHudIconToList( *p );
			if ( pWeaponInfo->iconAmmo )
			{
				pWeaponInfo->iconAmmo->Precache();
				pHudHR->SetHistoryGap( pWeaponInfo->iconAmmo->Height() );
			}
		}
	}

	FreeHudTextureList( tempList );
}

//-----------------------------------------------------------------------------
// Purpose: Helper function to return a Ammo pointer from id
//-----------------------------------------------------------------------------
CHudTexture *WeaponsResource::GetAmmoIconFromWeapon( int iAmmoId )
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if ( !player )
		return NULL;

	for ( int i = 0; i < MAX_WEAPONS; i++ )
	{
		C_BaseCombatWeapon *weapon = player->GetWeapon( i );
		if (!weapon)
			continue;

		if (weapon->GetPrimaryAmmoType() == iAmmoId)
			return weapon->GetWpnData().iconAmmo;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Get a pointer to a weapon using this ammo
//-----------------------------------------------------------------------------
const FileWeaponInfo_t *WeaponsResource::GetWeaponFromAmmo( int iAmmoId )
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if ( !player )
		return NULL;

	for ( int i = 0; i < MAX_WEAPONS; i++ )
	{
		C_BaseCombatWeapon *weapon = player->GetWeapon( i );
		if ( !weapon )
			continue;

		if ( weapon->GetPrimaryAmmoType() == iAmmoId )
		{
			return &weapon->GetWpnData();
		}
		else if ( weapon->GetSecondaryAmmoType() == iAmmoId )
		{
			return &weapon->GetWpnData();
		}
	}

	return NULL;
}