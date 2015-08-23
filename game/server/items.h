//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef ITEMS_H
#define ITEMS_H

#ifdef _WIN32
#pragma once
#endif

#include "entityoutput.h"
#include "player_pickup.h"
#include "vphysics/constraints.h"

// Armor given by a battery
#define MAX_NORMAL_BATTERY	100

// Ammo counts given by ammo items
#define SIZE_AMMO_P38   			8
#define SIZE_AMMO_MP40   			30
#define SIZE_AMMO_STG44		        30
#define SIZE_AMMO_FG42		        10
#define SIZE_AMMO_G43               10
#define SIZE_AMMO_THOMPSON          50
#define SIZE_AMMO_SVT40             10
#define SIZE_AMMO_K98               5
#define SIZE_AMMO_MAUSER            20
#define SIZE_AMMO_RPG_ROUND			1

#define SF_ITEM_START_CONSTRAINED	0x00000001


class CItem : public CBaseAnimating, public CDefaultPlayerPickupVPhysics
{
public:
	DECLARE_CLASS( CItem, CBaseAnimating );

	CItem();

	virtual void Spawn( void );
	virtual void Precache();

	// TFO
	virtual bool IsItem() { return true; }
	virtual int GiveAmmo(CBasePlayer *pPlayer, float flCount, const char *pszAmmoName, bool bSuppressSound = false);

	unsigned int PhysicsSolidMaskForEntity( void ) const;

	virtual CBaseEntity* Respawn( void );
	virtual void ItemTouch( CBaseEntity *pOther );
	virtual void Materialize( void );
	virtual bool MyTouch( CBasePlayer *pPlayer ) { return false; };

	// Become touchable when we are at rest
	virtual void OnEntityEvent( EntityEvent_t event, void *pEventData );

	// Activate when at rest, but don't allow pickup until then
	void ActivateWhenAtRest( float flTime = 0.5f );

	// IPlayerPickupVPhysics
	virtual void OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason = PICKED_UP_BY_CANNON );
	virtual void OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason );

	virtual int	ObjectCaps();
	virtual void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	Vector	GetOriginalSpawnOrigin( void ) { return m_vOriginalSpawnOrigin;	}
	QAngle	GetOriginalSpawnAngles( void ) { return m_vOriginalSpawnAngles;	}
	void	SetOriginalSpawnOrigin( const Vector& origin ) { m_vOriginalSpawnOrigin = origin; }
	void	SetOriginalSpawnAngles( const QAngle& angles ) { m_vOriginalSpawnAngles = angles; }
	bool	CreateItemVPhysicsObject( void );
	virtual bool	ItemCanBeTouchedByPlayer( CBasePlayer *pPlayer );

#if defined( HL2MP ) || defined( TF_DLL )
	void	FallThink( void );
	float  m_flNextResetCheckTime;
#endif

	DECLARE_DATADESC();
protected:
	virtual void ComeToRest( void );
	virtual void TransmitPickup(CBasePlayer *pPicker);

private:

	bool		m_bActivateWhenAtRest;
	COutputEvent m_OnPlayerTouch;
	COutputEvent m_OnCacheInteraction;

	Vector		m_vOriginalSpawnOrigin;
	QAngle		m_vOriginalSpawnAngles;

	IPhysicsConstraint		*m_pConstraint;
};

#endif // ITEMS_H
