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

	virtual unsigned int PhysicsSolidMaskForEntity( void ) const;

	virtual CBaseEntity* Respawn( void );
	virtual void Materialize( void );

	virtual int	ObjectCaps();
	virtual void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual bool CreateItemVPhysicsObject( void );

	DECLARE_DATADESC();

protected:
	virtual void TransmitPickup(CBasePlayer *pPicker);

protected:
	COutputEvent m_OnUse;
};

#endif // ITEMS_H
