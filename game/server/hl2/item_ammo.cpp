//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The various ammo types for HL2	
//
//=============================================================================//

#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "items.h"
#include "ammodef.h"
#include "eventlist.h"
#include "npcevent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define AMMO_CRATE_MODEL "models/items/ws_supply.mdl"

//---------------------------------------------------------
// Applies ammo quantity scale.
//---------------------------------------------------------
int ITEM_GiveAmmo(CBasePlayer* pPlayer, float flCount, const char* pszAmmoName)
{
	int iAmmoType = GetAmmoDef()->Index(pszAmmoName);
	if (iAmmoType == -1)
	{
		Msg("ERROR: Attempting to give unknown ammo type (%s)\n",pszAmmoName);
		return 0;
	}

	// Don't give out less than 1 of anything.
	flCount = MAX(1.0f, flCount);

	return pPlayer->GiveAmmo(flCount, iAmmoType, true);
}

// ==================================================================
// Ammo crate which will supply infinite ammo of the specified type
// ==================================================================
class CItem_AmmoCrate : public CBaseAnimating
{
public:
	DECLARE_CLASS( CItem_AmmoCrate, CBaseAnimating );

	void	Spawn( void );
	void	Precache( void );
	bool	CreateVPhysics( void );

	int		ObjectCaps(void) { return (BaseClass::ObjectCaps() | (FCAP_IMPULSE_USE | FCAP_USE_IN_RADIUS)); }
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	void	InputKill( inputdata_t &data );
	void	CrateThink( void );
	
	int		OnTakeDamage(const CTakeDamageInfo& info) { return 0; }

protected:
	float	m_flCloseTime;
	COutputEvent	m_OnUsed;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS( item_ammo_crate, CItem_AmmoCrate );

BEGIN_DATADESC( CItem_AmmoCrate )

	DEFINE_FIELD( m_flCloseTime, FIELD_FLOAT ),
	DEFINE_OUTPUT( m_OnUsed, "OnUsed" ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Kill", InputKill ),
	DEFINE_THINKFUNC( CrateThink ),

END_DATADESC()

#define	AMMO_CRATE_CLOSE_DELAY	1.5f

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_AmmoCrate::Spawn( void )
{
	m_GlowColor.Set({ 50, 50, 175, 200 });

	Precache();

	BaseClass::Spawn();

	SetModel(AMMO_CRATE_MODEL);
	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_VPHYSICS);
	CreateVPhysics();

	ResetSequence(LookupSequence("Idle"));

	m_flCloseTime = gpGlobals->curtime;
	m_flAnimTime = gpGlobals->curtime;
	m_flPlaybackRate = 0.0;
	SetCycle( 0 );

	m_takedamage = DAMAGE_EVENTS_ONLY;
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
bool CItem_AmmoCrate::CreateVPhysics( void )
{
	return (VPhysicsInitStatic() != NULL);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_AmmoCrate::Precache( void )
{
	PrecacheModel(AMMO_CRATE_MODEL);
	PrecacheScriptSound( "AmmoCrate.Open" );
	PrecacheScriptSound( "AmmoCrate.Close" );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pActivator - 
//			*pCaller - 
//			useType - 
//			value - 
//-----------------------------------------------------------------------------
void CItem_AmmoCrate::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = ToBasePlayer( pActivator );
	if (pPlayer == NULL)
		return;

	m_OnUsed.FireOutput( pActivator, this );

	int iSequence = LookupSequence( "Open" );

	if (GetSequence() == iSequence)
		return; // already open.

	Vector mins, maxs;
	trace_t tr;

	CollisionProp()->WorldSpaceAABB(&mins, &maxs);

	Vector vOrigin = GetAbsOrigin();
	vOrigin.z += (maxs.z - mins.z);
	mins = (mins - GetAbsOrigin()) * 0.2f;
	maxs = (maxs - GetAbsOrigin()) * 0.2f;
	mins.z = (GetAbsOrigin().z - vOrigin.z);

	UTIL_TraceHull(vOrigin, vOrigin, mins, maxs, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);

	if (tr.startsolid || tr.allsolid)
		return;

	// Animate!
	ResetSequence(iSequence);

	// Make sound
	CPASAttenuationFilter sndFilter(this, "AmmoCrate.Open");
	EmitSound(sndFilter, entindex(), "AmmoCrate.Open");

	// Give Ammo - The Easy Way...
	ITEM_GiveAmmo(pPlayer, 1, "Grenade");

	for (int i = 0; i < 10; i++)
	{
		CBaseCombatWeapon* pWeapon = pPlayer->Weapon_GetSlot(i);
		if (pWeapon == NULL)
			continue;

		if (pWeapon->IsRocketLauncher())
			ITEM_GiveAmmo(pPlayer, 1, "RPG_Round");
		else if (pWeapon->UsesClipsForAmmo1())
			ITEM_GiveAmmo(pPlayer, pWeapon->GetMaxClip1(), pWeapon->GetWpnData().szAmmo1);
	}

	pPlayer->EmitSound("Ammo.Pickup2");

	// Start thinking to make it return
	SetThink(&CItem_AmmoCrate::CrateThink);
	SetNextThink(gpGlobals->curtime + 0.1f);

	// Don't close again for X seconds
	m_flCloseTime = (gpGlobals->curtime + AMMO_CRATE_CLOSE_DELAY);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_AmmoCrate::CrateThink( void )
{
	StudioFrameAdvance();
	DispatchAnimEvents( this );

	SetNextThink( gpGlobals->curtime + 0.1f );

	// Start closing if we're not already
	if ( GetSequence() != LookupSequence( "Close" ) )
	{
		// Not ready to close?
		if ( m_flCloseTime <= gpGlobals->curtime )
		{
			ResetSequence(LookupSequence("Close"));
		}
	}
	else
	{
		// See if we're fully closed
		if ( IsSequenceFinished() )
		{
			// Stop thinking
			SetThink( NULL );
			CPASAttenuationFilter sndFilter( this, "AmmoCrate.Close" );
			EmitSound( sndFilter, entindex(), "AmmoCrate.Close" );

			// FIXME: We're resetting the sequence here
			// but setting Think to NULL will cause this to never have
			// StudioFrameAdvance called. What are the consequences of that?
			ResetSequence(LookupSequence("Idle"));
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void CItem_AmmoCrate::InputKill( inputdata_t &data )
{
	UTIL_Remove( this );
}