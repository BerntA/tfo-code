//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Handling for the base world item. Most of this was moved from items.cpp.
//
// $NoKeywords: $
//===========================================================================//

#include "cbase.h"
#include "player.h"
#include "items.h"
#include "gamerules.h"
#include "engine/IEngineSound.h"
#include "iservervehicle.h"
#include "physics_saverestore.h"
#include "world.h"
#include "ammodef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ITEM_PICKUP_BOX_BLOAT		40

BEGIN_DATADESC( CItem )

	// Function Pointers
	DEFINE_THINKFUNC( Materialize ),

	// Outputs
	DEFINE_OUTPUT(m_OnUse, "OnUse"),

END_DATADESC()

CItem::CItem()
{
}

bool CItem::CreateItemVPhysicsObject( void )
{
	// Create the object in the physics system
	int nSolidFlags = GetSolidFlags() | FSOLID_NOT_STANDABLE;

	if (VPhysicsInitNormal(SOLID_BBOX, nSolidFlags, false) == NULL)
	{
		SetSolid( SOLID_BBOX );
		AddSolidFlags( nSolidFlags );

		// If it's not physical, drop it to the floor
		if (UTIL_DropToFloor(this, MASK_SOLID) == 0)
		{
			Warning( "Item %s fell out of level at %f,%f,%f\n", GetClassname(), GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z);
			UTIL_Remove( this );
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem::Spawn( void )
{
	if ( g_pGameRules->IsAllowedToSpawn( this ) == false )
	{
		UTIL_Remove( this );
		return;
	}

	SetMoveType( MOVETYPE_FLYGRAVITY );
	SetSolid( SOLID_BBOX );
	SetBlocksLOS( false );
	AddEFlags( EFL_NO_ROTORWASH_PUSH );
	AddEffects(EF_ITEM_BLINK);

	// This will make them not collide with the player, but will collide
	// against other items + weapons
	SetCollisionGroup(COLLISION_GROUP_WEAPON);
	CollisionProp()->UseTriggerBounds(true, ITEM_PICKUP_BOX_BLOAT);

	if (CreateItemVPhysicsObject() == false)
		return;

	m_takedamage = DAMAGE_EVENTS_ONLY;
}

unsigned int CItem::PhysicsSolidMaskForEntity( void ) const
{ 
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_PLAYERCLIP);
}

void CItem::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = ToBasePlayer( pActivator );
	if (pPlayer && !IsEffectActive(EF_NODRAW))
		pPlayer->PickupObject(this);
}

//-----------------------------------------------------------------------------
// Tell the client directly that we've picked up this item.
//-----------------------------------------------------------------------------
void CItem::TransmitPickup(CBasePlayer *pPicker)
{
	if (!pPicker)
		return;

	CSingleUserRecipientFilter user(pPicker);
	user.MakeReliable();
	UserMessageBegin(user, "ItemPickup");
	WRITE_STRING(this->GetClassname());
	MessageEnd();
}

CBaseEntity* CItem::Respawn( void )
{
	SetTouch( NULL );

	AddEffects( EF_NODRAW );
	VPhysicsDestroyObject();

	SetMoveType( MOVETYPE_NONE );
	SetSolid( SOLID_BBOX );
	//AddSolidFlags( FSOLID_TRIGGER );

	UTIL_SetOrigin( this, g_pGameRules->VecItemRespawnSpot( this ) );// blip to whereever you should respawn.
	SetAbsAngles( g_pGameRules->VecItemRespawnAngles( this ) );// set the angles.

	UTIL_DropToFloor(this, MASK_SOLID);
	RemoveAllDecals(); //remove any decals

	SetThink ( &CItem::Materialize );
	SetNextThink(gpGlobals->curtime + g_pGameRules->FlItemRespawnTime(this));

	return this;
}

void CItem::Materialize( void )
{
	CreateItemVPhysicsObject();

	if ( IsEffectActive( EF_NODRAW ) )
	{
		// changing from invisible state to visible.
		EmitSound("Item.Materialize");
		RemoveEffects( EF_NODRAW );
		DoMuzzleFlash();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem::Precache()
{
	BaseClass::Precache();
	PrecacheScriptSound("Item.Materialize");
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPhysGunUser - 
//			PICKED_UP_BY_CANNON - 
//-----------------------------------------------------------------------------
void CItem::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	if (reason == PICKED_UP_BY_CANNON)
		CollisionProp()->UseTriggerBounds(true, ITEM_PICKUP_BOX_BLOAT * 2);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPhysGunUser - 
//			reason - 
//-----------------------------------------------------------------------------
void CItem::OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason )
{
	// Restore the pickup box to the original
	CollisionProp()->UseTriggerBounds( true, ITEM_PICKUP_BOX_BLOAT );
}

//-----------------------------------------------------------------------------
// Purpose: New input caps to accept inputs properly... FCAP_WCEDIT_POSITION
//-----------------------------------------------------------------------------
int	CItem::ObjectCaps(void)
{
	return (BaseClass::ObjectCaps() | FCAP_IMPULSE_USE | FCAP_USE_IN_RADIUS);
}

// Ammo Related Code:
int CItem::GiveAmmo(CBasePlayer *pPlayer, float flCount, const char *pszAmmoName, bool bSuppressSound)
{
	int iAmmoType = GetAmmoDef()->Index(pszAmmoName);
	if (iAmmoType == -1)
	{
		Msg("ERROR: Attempting to give unknown ammo type (%s)\n", pszAmmoName);
		return 0;
	}

	flCount *= g_pGameRules->GetAmmoQuantityScale(iAmmoType);

	// Don't give out less than 1 of anything.
	flCount = max(1.0f, flCount);

	return pPlayer->GiveAmmo(flCount, iAmmoType, bSuppressSound);
}