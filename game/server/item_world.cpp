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

#ifdef HL2MP
#include "hl2mp_gamerules.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ITEM_PICKUP_BOX_BLOAT		40

BEGIN_DATADESC( CItem )

	DEFINE_FIELD( m_bActivateWhenAtRest,	 FIELD_BOOLEAN ),
	DEFINE_PHYSPTR( m_pConstraint ),

	// Function Pointers
	DEFINE_ENTITYFUNC( ItemTouch ),
	DEFINE_THINKFUNC( Materialize ),
	DEFINE_THINKFUNC( ComeToRest ),

	// Outputs
	DEFINE_OUTPUT(m_OnPlayerTouch, "OnPlayerTouch"),
	DEFINE_OUTPUT(m_OnCacheInteraction, "OnCacheInteraction"),
	DEFINE_OUTPUT(m_OnUse, "OnUse"),

END_DATADESC()


//-----------------------------------------------------------------------------
// Constructor 
//-----------------------------------------------------------------------------
CItem::CItem()
{
	m_bActivateWhenAtRest = false;
}

bool CItem::CreateItemVPhysicsObject( void )
{
	// Create the object in the physics system
	int nSolidFlags = GetSolidFlags() | FSOLID_NOT_STANDABLE;

	if ( !m_bActivateWhenAtRest )
	{
		nSolidFlags |= FSOLID_TRIGGER;
	}

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
	
	if( IsX360() )
	{
		AddEffects( EF_ITEM_BLINK );
	}

	// This will make them not collide with the player, but will collide
	// against other items + weapons
	SetCollisionGroup( COLLISION_GROUP_WEAPON );
	CollisionProp()->UseTriggerBounds( true, ITEM_PICKUP_BOX_BLOAT );
	SetTouch(&CItem::ItemTouch);

	if ( CreateItemVPhysicsObject() == false )
		return;

	m_takedamage = DAMAGE_EVENTS_ONLY;

#if !defined( CLIENT_DLL )
	// Constrained start?
	if ( HasSpawnFlags( SF_ITEM_START_CONSTRAINED ) )
	{
		//Constrain the weapon in place
		IPhysicsObject *pReferenceObject, *pAttachedObject;

		pReferenceObject = g_PhysWorldObject;
		pAttachedObject = VPhysicsGetObject();

		if ( pReferenceObject && pAttachedObject )
		{
			constraint_fixedparams_t fixed;
			fixed.Defaults();
			fixed.InitWithCurrentObjectState( pReferenceObject, pAttachedObject );

			fixed.constraint.forceLimit	= lbs2kg( 10000 );
			fixed.constraint.torqueLimit = lbs2kg( 10000 );

			m_pConstraint = physenv->CreateFixedConstraint( pReferenceObject, pAttachedObject, NULL, fixed );

			m_pConstraint->SetGameData( (void *) this );
		}
	}
#endif //CLIENT_DLL
}

unsigned int CItem::PhysicsSolidMaskForEntity( void ) const
{ 
	return BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_PLAYERCLIP;
}

void CItem::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = ToBasePlayer( pActivator );

	if ( pPlayer )
	{
		pPlayer->PickupObject( this );
	}
}

extern int gEvilImpulse101;

//-----------------------------------------------------------------------------
// Activate when at rest, but don't allow pickup until then
//-----------------------------------------------------------------------------
void CItem::ActivateWhenAtRest( float flTime /* = 0.5f */ )
{
	RemoveSolidFlags( FSOLID_TRIGGER );
	m_bActivateWhenAtRest = true;
	SetThink( &CItem::ComeToRest );
	SetNextThink( gpGlobals->curtime + flTime );
}

//-----------------------------------------------------------------------------
// Become touchable when we are at rest
//-----------------------------------------------------------------------------
void CItem::OnEntityEvent( EntityEvent_t event, void *pEventData )
{
	BaseClass::OnEntityEvent( event, pEventData );

	switch( event )
	{
	case ENTITY_EVENT_WATER_TOUCH:
		{
			// Delay rest for a sec, to avoid changing collision 
			// properties inside a collision callback.
			SetThink( &CItem::ComeToRest );
			SetNextThink( gpGlobals->curtime + 0.1f );
		}
		break;
	}
}

//-----------------------------------------------------------------------------
// Become touchable when we are at rest
//-----------------------------------------------------------------------------
void CItem::ComeToRest( void )
{
	if ( m_bActivateWhenAtRest )
	{
		m_bActivateWhenAtRest = false;
		AddSolidFlags( FSOLID_TRIGGER );
		SetThink( NULL );
	}
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

//-----------------------------------------------------------------------------
// Purpose: Used to tell whether an item may be picked up by the player.  This
//			accounts for solid obstructions being in the way.
// Input  : *pItem - item in question
//			*pPlayer - player attempting the pickup
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool UTIL_ItemCanBeTouchedByPlayer( CBaseEntity *pItem, CBasePlayer *pPlayer )
{
	if ( pItem == NULL || pPlayer == NULL )
		return false;

	// For now, always allow a vehicle riding player to pick up things they're driving over
	// NO! Just... NO!!!
	if ( pPlayer->IsInAVehicle() )
		return false;

	// Get our test positions
	Vector vecStartPos;
	IPhysicsObject *pPhysObj = pItem->VPhysicsGetObject();
	if ( pPhysObj != NULL )
	{
		// Use the physics hull's center
		QAngle vecAngles;
		pPhysObj->GetPosition( &vecStartPos, &vecAngles );
	}
	else
	{
		// Use the generic bbox center
		vecStartPos = pItem->CollisionProp()->WorldSpaceCenter();
	}

	Vector vecEndPos = pPlayer->EyePosition();

	// FIXME: This is the simple first try solution towards the problem.  We need to take edges and shape more into account
	//		  for this to be fully robust.

	// Trace between to see if we're occluded
	trace_t tr;
	CTraceFilterSkipTwoEntities filter( pPlayer, pItem, COLLISION_GROUP_PLAYER_MOVEMENT );
	UTIL_TraceLine( vecStartPos, vecEndPos, MASK_SOLID, &filter, &tr );

	// Occluded
	// FIXME: For now, we exclude starting in solid because there are cases where this doesn't matter
	if ( tr.fraction < 1.0f )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Whether or not the item can be touched and picked up by the player, taking
//			into account obstructions and other hinderances
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CItem::ItemCanBeTouchedByPlayer( CBasePlayer *pPlayer )
{
	return UTIL_ItemCanBeTouchedByPlayer( this, pPlayer );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pOther - 
//-----------------------------------------------------------------------------
void CItem::ItemTouch( CBaseEntity *pOther )
{
	// if it's not a player, ignore
	if ( !pOther->IsPlayer() )
		return;

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;

	// Must be a valid pickup scenario (no blocking). Though this is a more expensive
	// check than some that follow, this has to be first Obecause it's the only one
	// that inhibits firing the output OnCacheInteraction.
	if ( ItemCanBeTouchedByPlayer( pPlayer ) == false )
		return;

	m_OnCacheInteraction.FireOutput(pOther, this);

	// Can I even pick stuff up?
	if ( !pPlayer->IsAllowedToPickupWeapons() )
		return;

	// ok, a player is touching this item, but can he have it?
	if ( !g_pGameRules->CanHaveItem( pPlayer, this ) )
	{
		// no? Ignore the touch.
		return;
	}

	if ( MyTouch( pPlayer ) )
	{
		m_OnPlayerTouch.FireOutput(pOther, this);

		SetTouch( NULL );
		SetThink( NULL );

		// player grabbed the item. 
		g_pGameRules->PlayerGotItem( pPlayer, this );
		if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_YES )
		{
			Respawn(); 
		}
		else
		{
			UTIL_Remove( this );

#ifdef HL2MP
			HL2MPRules()->RemoveLevelDesignerPlacedObject( this );
#endif
		}
	}
	else if (gEvilImpulse101)
	{
		UTIL_Remove( this );
	}
}

CBaseEntity* CItem::Respawn( void )
{
	SetTouch( NULL );
	AddEffects( EF_NODRAW );

	VPhysicsDestroyObject();

	SetMoveType( MOVETYPE_NONE );
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_TRIGGER );

	UTIL_SetOrigin( this, g_pGameRules->VecItemRespawnSpot( this ) );// blip to whereever you should respawn.
	SetAbsAngles( g_pGameRules->VecItemRespawnAngles( this ) );// set the angles.

#if !defined( TF_DLL )
	UTIL_DropToFloor( this, MASK_SOLID );
#endif

	RemoveAllDecals(); //remove any decals

	SetThink ( &CItem::Materialize );
	SetNextThink( gpGlobals->curtime + g_pGameRules->FlItemRespawnTime( this ) );
	return this;
}

void CItem::Materialize( void )
{
	CreateItemVPhysicsObject();

	if ( IsEffectActive( EF_NODRAW ) )
	{
		// changing from invisible state to visible.

#ifdef HL2MP
		EmitSound( "AlyxEmp.Charge" );
#else
		EmitSound( "Item.Materialize" );
#endif
		RemoveEffects( EF_NODRAW );
		DoMuzzleFlash();
	}

	SetTouch( &CItem::ItemTouch );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound( "Item.Materialize" );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPhysGunUser - 
//			PICKED_UP_BY_CANNON - 
//-----------------------------------------------------------------------------
void CItem::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	m_OnCacheInteraction.FireOutput(pPhysGunUser, this);

	if ( reason == PICKED_UP_BY_CANNON )
	{
		// Expand the pickup box
		CollisionProp()->UseTriggerBounds( true, ITEM_PICKUP_BOX_BLOAT * 2 );

		if( m_pConstraint != NULL )
		{
			physenv->DestroyConstraint( m_pConstraint );
			m_pConstraint = NULL;
		}
	}
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