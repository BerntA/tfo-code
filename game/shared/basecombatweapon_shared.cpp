//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"
#include "ammodef.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "physics_saverestore.h"
#include "datacache/imdlcache.h"
#include "activitylist.h"
#include "baseviewmodel_shared.h"
#include "particle_parse.h"
#include "basehlcombatweapon_shared.h"
#include "basegrenade_shared.h"

// NVNT start extra includes
#include "haptics/haptic_utils.h"
#ifdef CLIENT_DLL
#include "prediction.h"
#endif
// NVNT end extra includes

#if !defined( CLIENT_DLL )

// Game DLL Headers
#include "soundent.h"
#include "eventqueue.h"
#include "fmtstr.h"
#include "gameweaponmanager.h"
#include "hl2_player.h"

#ifdef HL2MP
#include "hl2mp_gamerules.h"
#endif

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define HIDEWEAPON_THINK_CONTEXT			"BaseCombatWeapon_HideThink"

ConVar tfo_push_range("tfo_push_range", "90", FCVAR_REPLICATED, "Set the range of the bash attack.");
ConVar* view_model_fov = NULL;

Vector CBaseCombatWeapon::GetIronsightPositionOffset( void ) const
{
	return GetWpnData().vecIronsightPosOffset;
}

QAngle CBaseCombatWeapon::GetIronsightAngleOffset( void ) const
{
	return GetWpnData().angIronsightAngOffset;
}

float CBaseCombatWeapon::GetIronsightFOVOffset( void ) const
{
	return GetWpnData().flIronsightFOVOffset;
}

float CBaseCombatWeapon::GetWeaponFOVOffset( void ) const
{
	return GetWpnData().flViewFov;
}

bool CBaseCombatWeapon::IsIronsighted( void )
{
	return m_bIsIronsighted;
}

void CBaseCombatWeapon::EnableIronsights( void )
{
	if( !HasIronsights() || m_bIsIronsighted )
		return;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if( !pOwner )
		return;

	if (UseIronsightAnims())
	{
		SendWeaponAnim( ACT_VM_IDLE_TO_AIM );
		m_fIronsightIN = gpGlobals->curtime + GetViewModelSequenceDuration();
		m_flNextPrimaryAttack = m_fIronsightIN;
	}

	if( pOwner->SetFOV( this, pOwner->GetDefaultFOV() + GetIronsightFOVOffset(), 0.6f ) ) //modify the last value to adjust how fast the fov is applied
	{
		pOwner->EmitSound( "BFPlayer.IronSightIn" );
		m_bIsIronsighted = true;
		m_flIronsightedTime = gpGlobals->curtime;
	}
}

void CBaseCombatWeapon::DisableIronsights( void )
{
	if( !HasIronsights() || !m_bIsIronsighted )
		return;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if( !pOwner )
		return;

	if (UseIronsightAnims())
		SendWeaponAnim( ACT_VM_AIM_TO_IDLE );

	if( pOwner->SetFOV( this, 0, 0.4f ) ) //modify the last value to adjust how fast the fov is applied
	{
		pOwner->EmitSound( "BFPlayer.IronSightOut" );
		m_bIsIronsighted = false;
		m_flIronsightedTime = gpGlobals->curtime;
	}
}

CBaseCombatWeapon::CBaseCombatWeapon()
{
	// Constructor must call this
	// CONSTRUCT_PREDICTABLE( CBaseCombatWeapon );

	// Some default values.  There should be set in the particular weapon classes
	m_fMinRange1		= 65;
	m_fMinRange2		= 65;
	m_fMaxRange1		= 1024;
	m_fMaxRange2		= 1024;

	m_bReloadsSingly	= false;

	// TFO
	m_bMagazineStyleReloads	= false;

	// Defaults to zero
	m_nViewModelIndex	= 0;

	// TFO
	m_iShotsFired = 0;

	m_bIsIronsighted = false;
	m_flIronsightedTime = 0.0f;

	m_bFlipViewModel	= false;

	// Holstering
	m_bWantsHolster = false;
	m_bNoSwitch = false;
	m_flHolsteredTime = 0.0f;

#if defined( CLIENT_DLL )
	m_iState = m_iOldState = WEAPON_NOT_CARRIED;
	m_iClip1 = -1;
	m_iClip2 = -1;
	m_iPrimaryAmmoType = -1;
	m_iSecondaryAmmoType = -1;
#endif

#if !defined( CLIENT_DLL )
	m_pConstraint = NULL;
	OnBaseCombatWeaponCreated( this );
#endif

	m_hWeaponFileInfo = GetInvalidWeaponInfoHandle();

	// TFO Glow Color for all weapons :
	color32 col32 = { 110, 95, 95, 220 };
	m_GlowColor = col32;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CBaseCombatWeapon::~CBaseCombatWeapon( void )
{
#if !defined( CLIENT_DLL )
	//Remove our constraint, if we have one
	if ( m_pConstraint != NULL )
	{
		physenv->DestroyConstraint( m_pConstraint );
		m_pConstraint = NULL;
	}
	OnBaseCombatWeaponDestroyed( this );
#endif
}

void CBaseCombatWeapon::Activate( void )
{
	BaseClass::Activate();

#ifndef CLIENT_DLL
	if ( GetOwnerEntity() )
		return;

	if ( g_pGameRules->IsAllowedToSpawn( this ) == false )
	{
		UTIL_Remove( this );
		return;
	}
#endif

}
void CBaseCombatWeapon::GiveDefaultAmmo( void )
{
	// If I use clips, set my clips to the default
	if ( UsesClipsForAmmo1() )
	{
		m_iClip1 = AutoFiresFullClip() ? 0 : GetDefaultClip1();
	}
	else
	{
		SetPrimaryAmmoCount( GetDefaultClip1() );
		m_iClip1 = WEAPON_NOCLIP;
	}
	if ( UsesClipsForAmmo2() )
	{
		m_iClip2 = GetDefaultClip2();
	}
	else
	{
		SetSecondaryAmmoCount( GetDefaultClip2() );
		m_iClip2 = WEAPON_NOCLIP;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set mode to world model and start falling to the ground
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Spawn( void )
{
	Precache();

	BaseClass::Spawn();

	SetSolid( SOLID_BBOX );
	m_flNextEmptySoundTime = 0.0f;

	// Weapons won't show up in trace calls if they are being carried...
	RemoveEFlags( EFL_USE_PARTITION_WHEN_NOT_SOLID );

	m_iState = WEAPON_NOT_CARRIED;
	// Assume 
	m_nViewModelIndex = 0;

	GiveDefaultAmmo();

	if ( GetWorldModel() )
	{
		SetModel( GetWorldModel() );
	}

#if !defined( CLIENT_DLL )
	if( IsX360() )
	{
		AddEffects( EF_ITEM_BLINK );
	}

	FallInit();
	SetCollisionGroup( COLLISION_GROUP_WEAPON );
	m_takedamage = DAMAGE_EVENTS_ONLY;

	SetBlocksLOS( false );

	// Default to non-removeable, because we don't want the
	// game_weapon_manager entity to remove weapons that have
	// been hand-placed by level designers. We only want to remove
	// weapons that have been dropped by NPC's.
	SetRemoveable( false );
#endif

	// Bloat the box for player pickup
	CollisionProp()->UseTriggerBounds( true, 36 );

	// Use more efficient bbox culling on the client. Otherwise, it'll setup bones for most
	// characters even when they're not in the frustum.
	AddEffects( EF_BONEMERGE_FASTCULL );

	// TFO Ironsight
	m_bIsIronsighted = false;
	m_flIronsightedTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: get this game's encryption key for decoding weapon kv files
// Output : virtual const unsigned char
//-----------------------------------------------------------------------------
const unsigned char *CBaseCombatWeapon::GetEncryptionKey( void ) 
{ 
	return g_pGameRules->GetEncryptionKey(); 
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Precache( void )
{
#if defined( CLIENT_DLL )
	Assert( Q_strlen( GetClassname() ) > 0 );
	// Msg( "Client got %s\n", GetClassname() );
#endif
	m_iPrimaryAmmoType = m_iSecondaryAmmoType = -1;

	// Add this weapon to the weapon registry, and get our index into it
	// Get weapon data from script file
	if ( ReadWeaponDataFromFileForSlot( filesystem, GetClassname(), &m_hWeaponFileInfo, GetEncryptionKey() ) )
	{
		// Get the ammo indexes for the ammo's specified in the data file
		if ( GetWpnData().szAmmo1[0] )
		{
			m_iPrimaryAmmoType = GetAmmoDef()->Index( GetWpnData().szAmmo1 );
			if (m_iPrimaryAmmoType == -1)
			{
				Msg("ERROR: Weapon (%s) using undefined primary ammo type (%s)\n",GetClassname(), GetWpnData().szAmmo1);
			}
		}
		if ( GetWpnData().szAmmo2[0] )
		{
			m_iSecondaryAmmoType = GetAmmoDef()->Index( GetWpnData().szAmmo2 );
			if (m_iSecondaryAmmoType == -1)
			{
				Msg("ERROR: Weapon (%s) using undefined secondary ammo type (%s)\n",GetClassname(),GetWpnData().szAmmo2);
			}

		}
#if defined( CLIENT_DLL )
		gWR.LoadWeaponSprites( GetWeaponFileInfoHandle() );
#endif
		// Precache models (preload to avoid hitch)
		m_iViewModelIndex = 0;
		m_iWorldModelIndex = 0;
		if ( GetViewModel() && GetViewModel()[0] )
		{
			m_iViewModelIndex = CBaseEntity::PrecacheModel( GetViewModel() );
		}
		if ( GetWorldModel() && GetWorldModel()[0] )
		{
			m_iWorldModelIndex = CBaseEntity::PrecacheModel( GetWorldModel() );
		}

		// Precache sounds, too

		// Ironsight
		PrecacheScriptSound( "BFPlayer.IronSightIn" );
		PrecacheScriptSound( "BFPlayer.IronSightOut" );

		// Lantern
		PrecacheScriptSound( "Lantern.On" );
		// Torch
		PrecacheScriptSound( "Torch.On" );

		// TFO Jump
		PrecacheScriptSound( "TFO.Land" );

		// Slash sound(s)
		PrecacheScriptSound( "TFO.Slash" );

		// Announcement sound(s)
		PrecacheScriptSound( "TFO.ThrowG" );

		// Swim Code
		PrecacheScriptSound( "Player.Swim" );

		// Sprint and jump/flying
		PrecacheScriptSound( "Weapon.Lower" );
		PrecacheScriptSound( "Weapon.Raise" );
		PrecacheScriptSound( "Weapon.Melee" );

		PrecacheParticleSystem( "weapon_muzzle_smoke_b" );
		PrecacheParticleSystem( "torch" );

		for ( int i = 0; i < NUM_SHOOT_SOUND_TYPES; ++i )
		{
			const char *shootsound = GetShootSound( i );
			if ( shootsound && shootsound[0] )
			{
				CBaseEntity::PrecacheScriptSound( shootsound );
			}
		}
	}
	else
	{
		// Couldn't read data file, remove myself
		Warning( "Error reading weapon data file for: %s\n", GetClassname() );
		//	Remove( );	//don't remove, this gets released soon!
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get my data in the file weapon info array
//-----------------------------------------------------------------------------
const FileWeaponInfo_t &CBaseCombatWeapon::GetWpnData( void ) const
{
	return *GetFileWeaponInfoFromHandle( m_hWeaponFileInfo );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CBaseCombatWeapon::GetViewModel( int /*viewmodelindex = 0 -- this is ignored in the base class here*/ ) const
{
	return GetWpnData().szViewModel;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CBaseCombatWeapon::GetWorldModel( void ) const
{
	return GetWpnData().szWorldModel;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CBaseCombatWeapon::GetAnimPrefix( void ) const
{
	return GetWpnData().szAnimationPrefix;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : char const
//-----------------------------------------------------------------------------
const char *CBaseCombatWeapon::GetPrintName( void ) const
{
	return GetWpnData().szPrintName;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetMaxClip1( void ) const
{
	return GetWpnData().iMaxClip1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetMaxClip2( void ) const
{
	return GetWpnData().iMaxClip2;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetDefaultClip1( void ) const
{
	return GetWpnData().iDefaultClip1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetDefaultClip2( void ) const
{
	return GetWpnData().iDefaultClip2;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::UsesClipsForAmmo1( void ) const
{
	return ( GetMaxClip1() != WEAPON_NOCLIP );
}

bool CBaseCombatWeapon::IsMeleeWeapon() const
{
	return GetWpnData().m_bMeleeWeapon;
}

float CBaseCombatWeapon::GetBashDamage() const
{
	return GetWpnData().m_flBashDamage;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::UsesClipsForAmmo2( void ) const
{
	return ( GetMaxClip2() != WEAPON_NOCLIP );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetWeight( void ) const
{
	return GetWpnData().iWeight;
}

//-----------------------------------------------------------------------------
// Purpose: Whether this weapon can be autoswitched to when the player runs out
//			of ammo in their current weapon or they pick this weapon up.
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::AllowsAutoSwitchTo( void ) const
{
	return GetWpnData().bAutoSwitchTo;
}

//-----------------------------------------------------------------------------
// Purpose: Whether this weapon can be autoswitched away from when the player
//			runs out of ammo in this weapon or picks up another weapon or ammo.
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::AllowsAutoSwitchFrom( void ) const
{
	return GetWpnData().bAutoSwitchFrom;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetWeaponFlags( void ) const
{
	return GetWpnData().iFlags;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetSlot( void ) const
{
	return GetWpnData().iSlot;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetPosition( void ) const
{
	return GetWpnData().iPosition;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CBaseCombatWeapon::GetName( void ) const
{
	return GetWpnData().szClassName;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteActive( void ) const
{
	return GetWpnData().iconActive;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteInactive( void ) const
{
	return GetWpnData().iconInactive;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteAmmo( void ) const
{
	return GetWpnData().iconAmmo;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteAmmo2( void ) const
{
	return GetWpnData().iconAmmo2;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteCrosshair( void ) const
{
	return GetWpnData().iconCrosshair;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteAutoaim( void ) const
{
	return GetWpnData().iconAutoaim;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteZoomedCrosshair( void ) const
{
	return GetWpnData().iconZoomedCrosshair;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteZoomedAutoaim( void ) const
{
	return GetWpnData().iconZoomedAutoaim;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CBaseCombatWeapon::GetShootSound( int iIndex ) const
{
	return GetWpnData().aShootSounds[ iIndex ];
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetRumbleEffect() const
{
	return GetWpnData().iRumbleEffect;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseCombatCharacter	*CBaseCombatWeapon::GetOwner() const
{
	return ToBaseCombatCharacter( m_hOwner.Get() );
}	

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : BaseCombatCharacter - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::SetOwner( CBaseCombatCharacter *owner )
{
	if ( !owner )
	{ 
#ifndef CLIENT_DLL
		// Make sure the weapon updates its state when it's removed from the player
		// We have to force an active state change, because it's being dropped and won't call UpdateClientData()
		int iOldState = m_iState;
		m_iState = WEAPON_NOT_CARRIED;
		OnActiveStateChanged( iOldState );
#endif

		// make sure we clear out our HideThink if we have one pending
		SetContextThink( NULL, 0, HIDEWEAPON_THINK_CONTEXT );
	}

	m_hOwner = owner;

#ifndef CLIENT_DLL
	DispatchUpdateTransmitState();
#else
	UpdateVisibility();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Return false if this weapon won't let the player switch away from it
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::IsAllowedToSwitch( void )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Return true if this weapon can be selected via the weapon selection
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::CanBeSelected( void )
{
	if ( !VisibleInWeaponSelection() )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Return true if this weapon has some ammo
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::HasAmmo( void )
{
	// Weapons with no ammo types can always be selected
	if ( m_iPrimaryAmmoType == -1 && m_iSecondaryAmmoType == -1  )
		return true;
	if ( GetWeaponFlags() & ITEM_FLAG_SELECTONEMPTY )
		return true;

	CBasePlayer *player = ToBasePlayer( GetOwner() );
	if ( !player )
		return false;
	return ( m_iClip1 > 0 || player->GetAmmoCount( m_iPrimaryAmmoType ) || m_iClip2 > 0 || player->GetAmmoCount( m_iSecondaryAmmoType ) );
}

//-----------------------------------------------------------------------------
// Purpose: Return true if this weapon should be seen, and hence be selectable, in the weapon selection
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::VisibleInWeaponSelection( void )
{
	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::HasWeaponIdleTimeElapsed( void )
{
	if ( gpGlobals->curtime > m_flTimeWeaponIdle )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : time - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::SetWeaponIdleTime( float time )
{
	m_flTimeWeaponIdle = time;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CBaseCombatWeapon::GetWeaponIdleTime( void )
{
	return m_flTimeWeaponIdle;
}

//-----------------------------------------------------------------------------
// Purpose: Drop/throw the weapon with the given velocity.
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Drop( const Vector &vecVelocity )
{
#if !defined( CLIENT_DLL )

	// Once somebody drops a gun, it's fair game for removal when/if
	// a game_weapon_manager does a cleanup on surplus weapons in the
	// world.
	DisableIronsights();
	SetRemoveable( true );
	WeaponManager_AmmoMod( this );

	// Holstering
	m_bWantsHolster = false;
	m_bNoSwitch = false;
	m_flHolsteredTime = 0.0f;

	//If it was dropped then there's no need to respawn it.
	AddSpawnFlags( SF_NORESPAWN );

	StopAnimation();
	StopFollowingEntity( );
	SetMoveType( MOVETYPE_FLYGRAVITY );
	// clear follow stuff, setup for collision
	SetGravity(1.0);
	m_iState = WEAPON_NOT_CARRIED;
	RemoveEffects( EF_NODRAW );
	FallInit();
	SetGroundEntity( NULL );
	SetThink( &CBaseCombatWeapon::SetPickupTouch );
	SetTouch(NULL);

	if( hl2_episodic.GetBool() )
	{
		RemoveSpawnFlags( SF_WEAPON_NO_PLAYER_PICKUP );
	}

	IPhysicsObject *pObj = VPhysicsGetObject();
	if ( pObj != NULL )
	{
		AngularImpulse	angImp( 200, 200, 200 );
		pObj->AddVelocity( &vecVelocity, &angImp );
	}
	else
	{
		SetAbsVelocity( vecVelocity );
	}

	CBaseEntity *pOwner = GetOwnerEntity();

	SetNextThink( gpGlobals->curtime + 1.0f );
	SetOwnerEntity( NULL );
	SetOwner( NULL );

	// If we're not allowing to spawn due to the gamerules,
	// remove myself when I'm dropped by an NPC.
	if ( pOwner && pOwner->IsNPC() )
	{
		if ( g_pGameRules->IsAllowedToSpawn( this ) == false )
		{
			UTIL_Remove( this );
			return;
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPicker - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::OnPickedUp( CBaseCombatCharacter *pNewOwner )
{
#if !defined( CLIENT_DLL )
	RemoveEffects( EF_ITEM_BLINK );

	if( pNewOwner->IsPlayer() )
	{
		m_OnPlayerPickup.FireOutput(pNewOwner, this);

		// Play the pickup sound for 1st-person observers
		CRecipientFilter filter;
		for ( int i=1; i <= gpGlobals->maxClients; ++i )
		{
			CBasePlayer *player = UTIL_PlayerByIndex(i);
			if ( player && !player->IsAlive() && player->GetObserverMode() == OBS_MODE_IN_EYE )
			{
				filter.AddRecipient( player );
			}
		}
		if ( filter.GetRecipientCount() )
		{
			CBaseEntity::EmitSound( filter, pNewOwner->entindex(), "Player.PickupWeapon" );
		}

		// Robin: We don't want to delete weapons the player has picked up, so 
		// clear the name of the weapon. This prevents wildcards that are meant 
		// to find NPCs finding weapons dropped by the NPCs as well.
		SetName( NULL_STRING );
	}
	else
	{
		m_OnNPCPickup.FireOutput(pNewOwner, this);
	}

#ifdef HL2MP
	HL2MPRules()->RemoveLevelDesignerPlacedObject( this );
#endif

	// Someone picked me up, so make it so that I can't be removed.
	SetRemoveable( false );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &vecTracerSrc - 
//			&tr - 
//			iTracerType - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType )
{
	if ( IsIronsighted() )
		return;

	CBaseEntity *pOwner = GetOwner();

	if ( pOwner == NULL )
	{
		BaseClass::MakeTracer( vecTracerSrc, tr, iTracerType );
		return;
	}

	const char *pszTracerName = GetTracerType();

	Vector vNewSrc = vecTracerSrc;
	int iEntIndex = pOwner->entindex();

	if ( g_pGameRules->IsMultiplayer() )
	{
		iEntIndex = entindex();
	}

	int iAttachment = GetTracerAttachment();

	switch ( iTracerType )
	{
	case TRACER_LINE:
		UTIL_Tracer( vNewSrc, tr.endpos, iEntIndex, iAttachment, 0.0f, true, pszTracerName );
		break;

	case TRACER_LINE_AND_WHIZ:
		UTIL_Tracer( vNewSrc, tr.endpos, iEntIndex, iAttachment, 0.0f, true, pszTracerName );
		break;
	}
}

void CBaseCombatWeapon::GiveTo( CBaseEntity *pOther )
{
	DefaultTouch( pOther );
}

//-----------------------------------------------------------------------------
// Purpose: Default Touch function for player picking up a weapon (not AI)
// Input  : pOther - the entity that touched me
// Output :
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::DefaultTouch( CBaseEntity *pOther )
{
}

void CBaseCombatWeapon::SetPickupTouch( void )
{
#if !defined( CLIENT_DLL )
	SetTouch(&CBaseCombatWeapon::DefaultTouch);

	if ( gpGlobals->maxClients > 1 )
	{
		if ( GetSpawnFlags() & SF_NORESPAWN )
		{
			SetThink( &CBaseEntity::SUB_Remove );
			SetNextThink( gpGlobals->curtime + 30.0f );
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Become a child of the owner (MOVETYPE_FOLLOW)
//			disables collisions, touch functions, thinking
// Input  : *pOwner - new owner/operator
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Equip( CBaseCombatCharacter *pOwner )
{
	// Attach the weapon to an owner
	SetAbsVelocity( vec3_origin );
	RemoveSolidFlags( FSOLID_TRIGGER );
	FollowEntity( pOwner );
	SetOwner( pOwner );
	SetOwnerEntity( pOwner );

	// Break any constraint I might have to the world.
	RemoveEffects( EF_ITEM_BLINK );

#if !defined( CLIENT_DLL )
	if ( m_pConstraint != NULL )
	{
		RemoveSpawnFlags( SF_WEAPON_START_CONSTRAINED );
		physenv->DestroyConstraint( m_pConstraint );
		m_pConstraint = NULL;
	}
#endif


	m_flNextPrimaryAttack		= gpGlobals->curtime;
	m_flNextSecondaryAttack		= gpGlobals->curtime;
	SetTouch( NULL );
	SetThink( NULL );
#if !defined( CLIENT_DLL )
	VPhysicsDestroyObject();
#endif

	if ( pOwner->IsPlayer() )
	{
		SetModel( GetViewModel() );
	}
	else
	{
		// Make the weapon ready as soon as any NPC picks it up.
		m_flNextPrimaryAttack = gpGlobals->curtime;
		m_flNextSecondaryAttack = gpGlobals->curtime;
		SetModel( GetWorldModel() );
	}
}

void CBaseCombatWeapon::SetActivity( Activity act, float duration ) 
{ 
	//Adrian: Oh man...
#if !defined( CLIENT_DLL ) && (defined( HL2MP ) || defined( PORTAL ))
	SetModel( GetWorldModel() );
#endif

	int sequence = SelectWeightedSequence( act ); 

	// FORCE IDLE on sequences we don't have (which should be many)
	if ( sequence == ACTIVITY_NOT_AVAILABLE )
		sequence = SelectWeightedSequence( ACT_VM_IDLE );

	//Adrian: Oh man again...
#if !defined( CLIENT_DLL ) && (defined( HL2MP ) || defined( PORTAL ))
	SetModel( GetViewModel() );
#endif

	if ( sequence != ACTIVITY_NOT_AVAILABLE )
	{
		SetSequence( sequence );
		SetActivity( act ); 
		SetCycle( 0 );
		ResetSequenceInfo( );

		if ( duration > 0 )
		{
			// FIXME: does this even make sense in non-shoot animations?
			m_flPlaybackRate = SequenceDuration( sequence ) / duration;
			m_flPlaybackRate = MIN( m_flPlaybackRate, 12.0);  // FIXME; magic number!, network encoding range
		}
		else
		{
			m_flPlaybackRate = 1.0;
		}
	}
}

//====================================================================================
// WEAPON CLIENT HANDLING
//====================================================================================
int CBaseCombatWeapon::UpdateClientData( CBasePlayer *pPlayer )
{
	int iNewState = WEAPON_IS_CARRIED_BY_PLAYER;

	if ( pPlayer->GetActiveWeapon() == this )
	{
		if ( pPlayer->m_fOnTarget ) 
		{
			iNewState = WEAPON_IS_ONTARGET;
		}
		else
		{
			iNewState = WEAPON_IS_ACTIVE;
		}
	}
	else
	{
		iNewState = WEAPON_IS_CARRIED_BY_PLAYER;
	}

	if ( m_iState != iNewState )
	{
		int iOldState = m_iState;
		m_iState = iNewState;
		OnActiveStateChanged( iOldState );
	}
	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : index - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::SetViewModelIndex( int index )
{
	Assert( index >= 0 && index < MAX_VIEWMODELS );
	m_nViewModelIndex = index;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iActivity - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::SendViewModelAnim( int nSequence )
{
#if defined( CLIENT_DLL )
	if ( !IsPredicted() )
		return;
#endif

	if ( nSequence < 0 )
		return;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	CBaseViewModel *vm = pOwner->GetViewModel( m_nViewModelIndex, false );

	if ( vm == NULL )
		return;

	SetViewModel();
	Assert( vm->ViewModelIndex() == m_nViewModelIndex );
	vm->SendViewModelMatchingSequence( nSequence );
}

float CBaseCombatWeapon::GetViewModelSequenceDuration()
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner == NULL )
	{
		Assert( false );
		return 0;
	}

	CBaseViewModel *vm = pOwner->GetViewModel( m_nViewModelIndex );
	if ( vm == NULL )
	{
		Assert( false );
		return 0;
	}

	SetViewModel();
	Assert( vm->ViewModelIndex() == m_nViewModelIndex );
	return vm->SequenceDuration();
}

bool CBaseCombatWeapon::IsViewModelSequenceFinished( void )
{
	// These are not valid activities and always complete immediately
	if ( GetActivity() == ACT_RESET || GetActivity() == ACT_INVALID )
		return true;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner == NULL )
	{
		Assert( false );
		return false;
	}

	CBaseViewModel *vm = pOwner->GetViewModel( m_nViewModelIndex );
	if ( vm == NULL )
	{
		Assert( false );
		return false;
	}

	return vm->IsSequenceFinished();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::SetViewModel()
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner == NULL )
		return;
	CBaseViewModel *vm = pOwner->GetViewModel( m_nViewModelIndex, false );
	if ( vm == NULL )
		return;
	Assert( vm->ViewModelIndex() == m_nViewModelIndex );
	vm->SetWeaponModel( GetViewModel( m_nViewModelIndex ), this );
}

//-----------------------------------------------------------------------------
// Purpose: Set the desired activity for the weapon and its viewmodel counterpart
// Input  : iActivity - activity to play
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::SendWeaponAnim( int iActivity )
{
#ifdef USES_ECON_ITEMS
	iActivity = TranslateViewmodelHandActivity( (Activity)iActivity );
#endif		
	// NVNT notify the haptics system of this weapons new activity
#ifdef WIN32
#ifdef CLIENT_DLL
	if ( prediction->InPrediction() && prediction->IsFirstTimePredicted() )
#endif
#ifndef _X360
		HapticSendWeaponAnim(this,iActivity);
#endif
#endif
	//For now, just set the ideal activity and be done with it
	return SetIdealActivity( (Activity) iActivity );
}

//====================================================================================
// WEAPON SELECTION
//====================================================================================

//-----------------------------------------------------------------------------
// Purpose: Returns true if the weapon currently has ammo or doesn't need ammo
// Output :
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::HasAnyAmmo( void )
{
	// If I don't use ammo of any kind, I can always fire
	if ( !UsesPrimaryAmmo() && !UsesSecondaryAmmo() )
		return true;

	// Otherwise, I need ammo of either type
	return ( HasPrimaryAmmo() || HasSecondaryAmmo() );
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the weapon currently has ammo or doesn't need ammo
// Output :
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::HasPrimaryAmmo( void )
{
	// If I use a clip, and have some ammo in it, then I have ammo
	if ( UsesClipsForAmmo1() )
	{
		if ( m_iClip1 > 0 )
			return true;
	}

	// Otherwise, I have ammo if I have some in my ammo counts
	CBaseCombatCharacter		*pOwner = GetOwner();
	if ( pOwner )
	{
		if ( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) > 0 ) 
			return true;
	}
	else
	{
		// No owner, so return how much primary ammo I have along with me.
		if( GetPrimaryAmmoCount() > 0 )
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the weapon currently has ammo or doesn't need ammo
// Output :
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::HasSecondaryAmmo( void )
{
	// If I use a clip, and have some ammo in it, then I have ammo
	if ( UsesClipsForAmmo2() )
	{
		if ( m_iClip2 > 0 )
			return true;
	}

	// Otherwise, I have ammo if I have some in my ammo counts
	CBaseCombatCharacter		*pOwner = GetOwner();
	if ( pOwner )
	{
		if ( pOwner->GetAmmoCount( m_iSecondaryAmmoType ) > 0 ) 
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the weapon actually uses primary ammo
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::UsesPrimaryAmmo( void )
{
	if ( m_iPrimaryAmmoType < 0 )
		return false;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the weapon actually uses secondary ammo
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::UsesSecondaryAmmo( void )
{
	if ( m_iSecondaryAmmoType < 0 )
		return false;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Show/hide weapon and corresponding view model if any
// Input  : visible - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::SetWeaponVisible( bool visible )
{
	CBaseViewModel *vm = NULL;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner )
	{
		vm = pOwner->GetViewModel( m_nViewModelIndex );
	}

	if ( visible )
	{
		RemoveEffects( EF_NODRAW );
		if ( vm )
		{
			vm->RemoveEffects( EF_NODRAW );
		}
	}
	else
	{
		AddEffects( EF_NODRAW );
		if ( vm )
		{
			vm->AddEffects( EF_NODRAW );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::IsWeaponVisible( void )
{
	CBaseViewModel *vm = NULL;
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner )
	{
		vm = pOwner->GetViewModel( m_nViewModelIndex );
		if ( vm )
			return ( !vm->IsEffectActive(EF_NODRAW) );
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: If the current weapon has more ammo, reload it. Otherwise, switch 
//			to the next best weapon we've got. Returns true if it took any action.
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::ReloadOrSwitchWeapons( void )
{
	m_bFireOnEmpty = false;

	if ( HasAnyAmmo() )
	{
		// Weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
		if ( UsesClipsForAmmo1() && !AutoFiresFullClip() && 
			(m_iClip1 == 0) && 
			(GetWeaponFlags() & ITEM_FLAG_NOAUTORELOAD) == false && 
			m_flNextPrimaryAttack < gpGlobals->curtime && 
			m_flNextSecondaryAttack < gpGlobals->curtime )
		{
			// if we're successfully reloading, we're done
			if ( Reload() )
				return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *szViewModel - 
//			*szWeaponModel - 
//			iActivity - 
//			*szAnimExt - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::DefaultDeploy( char *szViewModel, char *szWeaponModel, int iActivity, char *szAnimExt )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner )
	{
		// Dead men deploy no weapons
		if ( pOwner->IsAlive() == false )
			return false;

		pOwner->SetAnimationExtension( szAnimExt );

		SetViewModel();
		SendWeaponAnim( iActivity );

		pOwner->SetNextAttack( gpGlobals->curtime + SequenceDuration() );
	}

	// Can't shoot again until we've finished deploying
	m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration();
	m_flNextSecondaryAttack	= gpGlobals->curtime + SequenceDuration();

	// Holstering
	m_bWantsHolster = false;
	m_bNoSwitch = false;

	WeaponSound( DEPLOY );

	SetWeaponVisible( true );

	SetContextThink( NULL, 0, HIDEWEAPON_THINK_CONTEXT );

	m_iShotsFired = 0;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::Deploy( )
{
	MDLCACHE_CRITICAL_SECTION();
	return DefaultDeploy( (char*)GetViewModel(), (char*)GetWorldModel(), GetDrawActivity(), (char*)GetAnimPrefix() );
}

Activity CBaseCombatWeapon::GetDrawActivity( void )
{
	return ACT_VM_DRAW;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::Holster( CBaseCombatWeapon *pSwitchingTo )
{ 
	MDLCACHE_CRITICAL_SECTION();

	// kill any think functions
	SetThink(NULL);

	SetWeaponVisible( false );

	if (!m_bNoSwitch)
	{
		CBasePlayer *pOwner = ToBasePlayer(GetOwner());
		if (pOwner)
			pOwner->Weapon_DeployNextWeapon();
	}
	else
		m_bNoSwitch = false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Check if you can holster or deploy!
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::CanHolster( void )
{
	CBasePlayer *pClient = ToBasePlayer(GetOwner());
	if ( pClient ) 
	{
		if (!pClient->IsAlive() || m_bInReload || !pClient->GetGroundEntity() || !pClient->m_bCanDoMeleeAttack)
			return false;
	}

	return true;
}

#ifdef CLIENT_DLL

void CBaseCombatWeapon::BoneMergeFastCullBloat( Vector &localMins, Vector &localMaxs, const Vector &thisEntityMins, const Vector &thisEntityMaxs ) const
{
	// The default behavior pushes it out by BONEMERGE_FASTCULL_BBOX_EXPAND in all directions, but we can do better
	// since we know the weapon will never point behind him.

	localMaxs.x += 20;	// Leaves some space in front for long weapons.

	localMins.y -= 20;	// Fatten it to his left and right since he can rotate that way.
	localMaxs.y += 20;	

	localMaxs.z += 15;	// Leave some space at the top.
}

#else
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::InputHideWeapon( inputdata_t &inputdata )
{
	// Only hide if we're still the active weapon. If we're not the active weapon
	if ( GetOwner() && GetOwner()->GetActiveWeapon() == this )
	{
		SetWeaponVisible( false );
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::HideThink( void )
{
	// Only hide if we're still the active weapon. If we're not the active weapon
	if ( GetOwner() && GetOwner()->GetActiveWeapon() == this )
	{
		SetWeaponVisible( false );
	}
}

bool CBaseCombatWeapon::CanReload( void )
{
	if ( AutoFiresFullClip() && m_bFiringWholeClip )
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::ItemPreFrame( void )
{
	MaintainIdealActivity();
}

//====================================================================================
// TFO - Clear Particles from Torch and Reset Sounds on Lantern/Torch, called on Holster
//====================================================================================
void CBaseCombatWeapon::ClearParticles( void )
{
	// Disable Ironsight when holstering/changing weapon
	if ( m_bIsIronsighted )
	{
		DisableIronsights();
	}

	StopParticleEffects( this );

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if (!pOwner)
		return;	

	pOwner->StopSound( "Torch.On" );
	pOwner->StopSound( "Lantern.On" );
	pOwner->StopSound( "Player.Swim" );

#if defined( CLIENT_DLL )
	pOwner->ParticleProp()->StopParticlesNamed( "tfo" );
#endif

	CBaseViewModel *vm = pOwner->GetViewModel();
	if ( vm )
	{
		StopParticleEffects( vm );

#if defined( CLIENT_DLL )
		vm->ParticleProp()->StopParticlesNamed( "tfo" );
#endif
	}

	CBaseCombatWeapon *pWep = pOwner->GetActiveWeapon();
	if ( pWep )
	{
		StopParticleEffects( pWep );

#if defined( CLIENT_DLL )
		pWep->ParticleProp()->StopParticlesNamed( "tfo" );
#endif
	}
}

//====================================================================================
// Weapon Effects will be handled here... ~Bernt.
//====================================================================================
void CBaseCombatWeapon::DoWeaponFX( void )
{
	if( m_iShotsFired > GetOverloadCapacity() )
	{
		// Dispatch smoke... Overload smoke.
		CBasePlayer *pPlayer = ToBasePlayer( this->GetOwner() );
		if ( pPlayer )
		{
			DispatchParticleEffect( "weapon_muzzle_smoke_b", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), "muzzle", true);
		}
		else
		{
			// Confirm that this one works for NPC's...
			DispatchParticleEffect( "weapon_muzzle_smoke_b", PATTACH_POINT_FOLLOW, (CBaseEntity*)this->GetWorldModel(), "muzzle", true);
		}

		m_iShotsFired = 0;
	}
}

//====================================================================================
// Proper Holster
//====================================================================================
void CBaseCombatWeapon::StartHolsterSequence(bool bNoSwitch)
{
	DisableIronsights();
	m_bWantsHolster = true;
	m_bNoSwitch = bNoSwitch;
	m_bInReload = false;
	m_bFiringWholeClip = false;

	// Send holster animation
	SendWeaponAnim(ACT_VM_HOLSTER);

	// Remove the fire particle on the torch and other 'long' lasting particles.
	ClearParticles();

	m_flHolsteredTime = gpGlobals->curtime + GetViewModelSequenceDuration();
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration() + 0.4f;
	m_flNextSecondaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration() + 0.4f;

	// We will stay active in the pre frame until we're done so we don't override the visibility state or animation state as we change to the new active weapon. (VITAL, IF YOU REMOVE THIS THEN YOUR VIEWMODEL MIGHT GO INVISIBLE FROM TIME TO TIME)
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner)
		pOwner->m_flNextAttack = gpGlobals->curtime + GetViewModelSequenceDuration() + 0.4f;
}

//====================================================================================
// Whenever we holster (want to holster) we start a timer which will let us fully holster then we do the holster to the new wep.
//====================================================================================
void CBaseCombatWeapon::HandleWeaponSelectionTime(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	if (m_bWantsHolster)
	{
		if (m_flHolsteredTime <= gpGlobals->curtime)
		{
			m_bWantsHolster = false;
			Holster();
		}
	}
}

//====================================================================================
// WEAPON BEHAVIOUR
//====================================================================================
void CBaseCombatWeapon::ItemPostFrame( void )
{
	AdjustWeaponFOV();

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if (!pOwner)
		return;

	DoWeaponFX();

	if ( m_bFireOnEmpty )
		DisableIronsights();

	//// IRONSIGHT DISABLED BY JUMP, FALL, LADDER, NOCLIP, FLOAT, ROAMING, ETC...
	bool bShouldDisableSights = ( (pOwner->GetMoveType() == MOVETYPE_LADDER) || !pOwner->GetGroundEntity() || (pOwner->m_nButtons & IN_JUMP) || (pOwner->m_nButtons & IN_RELOAD) || (m_iClip1 == 0) || (pOwner->m_nButtons & IN_USE) );

	if ( bShouldDisableSights )
		DisableIronsights();

#if !defined( CLIENT_DLL )
	CHL2_Player *pHL2Client = dynamic_cast < CHL2_Player * > ( pOwner );
	if ( pHL2Client )
	{
		if ( pHL2Client->ItemTakeAnim )
			return;
	}
#endif

	if ( pOwner->m_bShouldLowerWeapon )
		return;

	// Ironsight Idle code - if applicable
	if (!(pOwner->m_nButtons & IN_ATTACK) &&
		!m_bInReload &&
		pOwner->GetGroundEntity() &&
		!pOwner->m_bIsRunning &&
		m_bIsIronsighted &&
		UseIronsightAnims())
	{
		if (gpGlobals->curtime > m_fIronsightIN)
		{
			SendWeaponAnim(ACT_VM_AIM_IDLE);
		}
	}

	//No Weapons on ladders 
	if( pOwner->GetMoveType() == MOVETYPE_LADDER )
		return; 

	UpdateAutoFire();

	//Track the duration of the fire
	//FIXME: Check for IN_ATTACK2 as well?
	//FIXME: What if we're calling ItemBusyFrame?
	m_fFireDuration = ( pOwner->m_nButtons & IN_ATTACK ) ? ( m_fFireDuration + gpGlobals->frametime ) : 0.0f;

	if ( UsesClipsForAmmo1() )
	{
		CheckReload();
	}

	bool bFired = false;

	// Secondary attack has priority
	if ((pOwner->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= gpGlobals->curtime))
	{
		if (UsesSecondaryAmmo() && pOwner->GetAmmoCount(m_iSecondaryAmmoType)<=0 )
		{
			if (m_flNextEmptySoundTime < gpGlobals->curtime)
			{
				WeaponSound(EMPTY);
				m_flNextSecondaryAttack = m_flNextEmptySoundTime = gpGlobals->curtime + 0.5;
			}
		}
		else if (pOwner->GetWaterLevel() == 3 && m_bAltFiresUnderwater == false)
		{
			// This weapon doesn't fire underwater
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
		else
		{
			// FIXME: This isn't necessarily true if the weapon doesn't have a secondary fire!
			// For instance, the crossbow doesn't have a 'real' secondary fire, but it still 
			// stops the crossbow from firing on the 360 if the player chooses to hold down their
			// zoom button. (sjb) Orange Box 7/25/2007
			bFired = ShouldBlockPrimaryFire();

			SecondaryAttack();

			// Secondary ammo doesn't have a reload animation
			if ( UsesClipsForAmmo2() )
			{
				// reload clip2 if empty
				if (m_iClip2 < 1)
				{
					pOwner->RemoveAmmo( 1, m_iSecondaryAmmoType );
					m_iClip2 = m_iClip2 + 1;
				}
			}
		}
	}

	if ( !bFired && (pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		// Clip empty? Or out of ammo on a no-clip weapon?
		if ( !IsMeleeWeapon() &&  
			( ( UsesClipsForAmmo1() && m_iClip1 <= 0) || ( !UsesClipsForAmmo1() && pOwner->GetAmmoCount(m_iPrimaryAmmoType)<=0 ) ) )
		{
			HandleFireOnEmpty();
		}
		else if (pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
		{
			// This weapon doesn't fire underwater
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
		else
		{
			//NOTENOTE: There is a bug with this code with regards to the way machine guns catch the leading edge trigger
			//			on the player hitting the attack key.  It relies on the gun catching that case in the same frame.
			//			However, because the player can also be doing a secondary attack, the edge trigger may be missed.
			//			We really need to hold onto the edge trigger and only clear the condition when the gun has fired its
			//			first shot.  Right now that's too much of an architecture change -- jdw

			// If the firing button was just pressed, or the alt-fire just released, reset the firing time
			if ( ( pOwner->m_afButtonPressed & IN_ATTACK ) || ( pOwner->m_afButtonReleased & IN_ATTACK2 ) )
			{
				m_flNextPrimaryAttack = gpGlobals->curtime;
			}

			PrimaryAttack();

			if ( AutoFiresFullClip() )
			{
				m_bFiringWholeClip = true;
			}

#ifdef CLIENT_DLL
			pOwner->SetFiredWeapon( true );
#endif
		}
	}

	// -----------------------
	//  Reload pressed / Clip Empty
	// -----------------------
	if ( ( pOwner->m_nButtons & IN_RELOAD ) && UsesClipsForAmmo1() && !m_bInReload && !pOwner->m_bIsRunning ) 
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
		m_fFireDuration = 0.0f;
	}

	// -----------------------
	//  No buttons down
	// -----------------------
	if (!((pOwner->m_nButtons & IN_ATTACK) || (pOwner->m_nButtons & IN_ATTACK2) || (CanReload() && pOwner->m_nButtons & IN_RELOAD)))
	{
		// no fire buttons down or reloading
		if ( !ReloadOrSwitchWeapons() && !m_bInReload && !pOwner->m_bIsRunning && pOwner->GetGroundEntity() )
		{
			WeaponIdle();
		}
	}
}

void CBaseCombatWeapon::HandleFireOnEmpty()
{
	// If we're already firing on empty, reload if we can
	if ( m_bFireOnEmpty )
	{
		ReloadOrSwitchWeapons();
		m_fFireDuration = 0.0f;
	}
	else
	{
		if (m_flNextEmptySoundTime < gpGlobals->curtime)
		{
			WeaponSound(EMPTY);
			m_flNextEmptySoundTime = gpGlobals->curtime + 0.5;
		}
		m_bFireOnEmpty = true;
	}
}

void CBaseCombatWeapon::AdjustWeaponFOV(void)
{
	if (view_model_fov == NULL)
		view_model_fov = cvar->FindVar("viewmodel_fov");
	view_model_fov->SetValue(GetWpnData().flViewFov);
}

//-----------------------------------------------------------------------------
// Purpose: Called each frame by the player PostThink, if the player's not ready to attack yet
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::ItemBusyFrame(void)
{
	AdjustWeaponFOV();
	UpdateAutoFire();
	HandleWeaponSelectionTime();
}

//-----------------------------------------------------------------------------
// Purpose: Base class default for getting bullet type
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetBulletType( void )
{
	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: Base class default for getting spread
// Input  :
// Output :
//-----------------------------------------------------------------------------
const Vector& CBaseCombatWeapon::GetBulletSpread( void )
{
	static Vector cone = VECTOR_CONE_15DEGREES;
	return cone;
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CBaseCombatWeapon::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t defaultWeaponProficiencyTable[] =
	{
		{ 1.0, 1.0	},
		{ 1.0, 1.0	},
		{ 1.0, 1.0	},
		{ 1.0, 1.0	},
		{ 1.0, 1.0	},
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE(defaultWeaponProficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);
	return defaultWeaponProficiencyTable;
}

//-----------------------------------------------------------------------------
// Purpose: Base class default for getting firerate
// Input  :
// Output :
//-----------------------------------------------------------------------------
float CBaseCombatWeapon::GetFireRate( void )
{
	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: Base class default for playing shoot sound
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::WeaponSound( WeaponSound_t sound_type, float soundtime /* = 0.0f */ )
{
	// If we have some sounds from the weapon classname.txt file, play a random one of them
	const char *shootsound = GetShootSound( sound_type );
	if ( !shootsound || !shootsound[0] )
		return;

	CSoundParameters params;

	if ( !GetParametersForSound( shootsound, params, NULL ) )
		return;

	if ( params.play_to_owner_only )
	{
		// Am I only to play to my owner?
		if ( GetOwner() && GetOwner()->IsPlayer() )
		{
			CSingleUserRecipientFilter filter( ToBasePlayer( GetOwner() ) );
			if ( IsPredicted() && CBaseEntity::GetPredictionPlayer() )
			{
				filter.UsePredictionRules();
			}
			EmitSound( filter, GetOwner()->entindex(), shootsound, NULL, soundtime );
		}
	}
	else
	{
		// Play weapon sound from the owner
		if ( GetOwner() )
		{
			CPASAttenuationFilter filter( GetOwner(), params.soundlevel );
			if ( IsPredicted() && CBaseEntity::GetPredictionPlayer() )
			{
				filter.UsePredictionRules();
			}
			EmitSound( filter, GetOwner()->entindex(), shootsound, NULL, soundtime ); 

#if !defined( CLIENT_DLL )
			if( sound_type == EMPTY )
			{
				CSoundEnt::InsertSound( SOUND_COMBAT, GetOwner()->GetAbsOrigin(), SOUNDENT_VOLUME_EMPTY, 0.2, GetOwner() );
			}
#endif
		}
		// If no owner play from the weapon (this is used for thrown items)
		else
		{
			CPASAttenuationFilter filter( this, params.soundlevel );
			if ( IsPredicted() && CBaseEntity::GetPredictionPlayer() )
			{
				filter.UsePredictionRules();
			}
			EmitSound( filter, entindex(), shootsound, NULL, soundtime ); 
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Stop a sound played by this weapon.
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::StopWeaponSound( WeaponSound_t sound_type )
{
	//if ( IsPredicted() )
	//	return;

	// If we have some sounds from the weapon classname.txt file, play a random one of them
	const char *shootsound = GetShootSound( sound_type );
	if ( !shootsound || !shootsound[0] )
		return;

	CSoundParameters params;
	if ( !GetParametersForSound( shootsound, params, NULL ) )
		return;

	// Am I only to play to my owner?
	if ( params.play_to_owner_only )
	{
		if ( GetOwner() )
		{
			StopSound( GetOwner()->entindex(), shootsound );
		}
	}
	else
	{
		// Play weapon sound from the owner
		if ( GetOwner() )
		{
			StopSound( GetOwner()->entindex(), shootsound );
		}
		// If no owner play from the weapon (this is used for thrown items)
		else
		{
			StopSound( entindex(), shootsound );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::DefaultReload( int iClipSize1, int iClipSize2, int iActivity )
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
		return false;

	// If I don't have any spare ammo, I can't reload
	if ( pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0 )
		return false;

	bool bReload = false;

	// If you don't have clips, then don't try to reload them.
	if ( UsesClipsForAmmo1() )
	{
		// need to reload primary clip?
		int primary	= MIN(iClipSize1 - m_iClip1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));
		if ( primary != 0 )
		{
			bReload = true;
			DisableIronsights();
		}
	}

	if ( UsesClipsForAmmo2() )
	{
		// need to reload secondary clip?
		int secondary = MIN(iClipSize2 - m_iClip2, pOwner->GetAmmoCount(m_iSecondaryAmmoType));
		if ( secondary != 0 )
		{
			bReload = true;
			DisableIronsights();
		}
	}

	if ( !bReload )
		return false;

#ifdef CLIENT_DLL
	// Play reload
	WeaponSound( RELOAD );
#endif
	SendWeaponAnim( iActivity );

	// Play the player's reload animation
	if ( pOwner->IsPlayer() )
	{
		( ( CBasePlayer * )pOwner)->SetAnimation( PLAYER_RELOAD );
	}

	MDLCACHE_CRITICAL_SECTION();
	float flSequenceEndTime = gpGlobals->curtime + SequenceDuration();
	pOwner->SetNextAttack( flSequenceEndTime );
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = flSequenceEndTime;

	m_bInReload = true;
	DisableIronsights();

	return true;
}

bool CBaseCombatWeapon::ReloadsSingly( void ) const
{
	return m_bReloadsSingly;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::Reload( void )
{
	DisableIronsights();
	return DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
}

//=========================================================
void CBaseCombatWeapon::WeaponIdle( void )
{
	//Idle again if we've finished
	if ( HasWeaponIdleTimeElapsed() )
	{
		SendWeaponAnim( ACT_VM_IDLE );
	}
}


//=========================================================
Activity CBaseCombatWeapon::GetPrimaryAttackActivity( void )
{
	return ACT_VM_PRIMARYATTACK;
}

//=========================================================
Activity CBaseCombatWeapon::GetSecondaryAttackActivity( void )
{
	return ACT_VM_SECONDARYATTACK;
}

//-----------------------------------------------------------------------------
// Purpose: Adds in view kick and weapon accuracy degradation effect
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::AddViewKick( void )
{
	//NOTENOTE: By default, weapon will not kick up (defined per weapon)
}

//-----------------------------------------------------------------------------
// Purpose: Get the string to print death notices with
//-----------------------------------------------------------------------------
char *CBaseCombatWeapon::GetDeathNoticeName( void )
{
#if !defined( CLIENT_DLL )
	return (char*)STRING( m_iszName );
#else
	return "GetDeathNoticeName not implemented on client yet";
#endif
}

//====================================================================================
// WEAPON RELOAD TYPES
//====================================================================================
void CBaseCombatWeapon::CheckReload( void )
{
	if ( m_bReloadsSingly )
	{
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		if ( !pOwner )
			return;

		if ((m_bInReload) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
		{
			if ( pOwner->m_nButtons & (IN_ATTACK | IN_ATTACK2) && m_iClip1 > 0 )
			{
				m_bInReload = false;
				return;
			}

			// If out of ammo end reload
			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <=0)
			{
				FinishReload();
				return;
			}
			// If clip not full reload again
			else if (m_iClip1 < GetMaxClip1())
			{
				// Add them to the clip
				m_iClip1 += 1;
				pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );

				Reload();
				return;
			}
			// Clip full, stop reloading
			else
			{
				FinishReload();
				m_flNextPrimaryAttack	= gpGlobals->curtime;
				m_flNextSecondaryAttack = gpGlobals->curtime;
				return;
			}
		}
	}
	else
	{
		if ( (m_bInReload) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
		{
			FinishReload();
			m_flNextPrimaryAttack	= gpGlobals->curtime;
			m_flNextSecondaryAttack = gpGlobals->curtime;
			m_bInReload = false;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Reload has finished.
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::FinishReload( void )
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner)
	{
		// If I use primary clips, reload primary
		// TFO - Changed so that we lose remaining bullets on reload and we throw the clip away. Check clipmodel in weapon_* scripts for clip model to throw away.
		if ( UsesClipsForAmmo1() )
		{
			int primary = min( GetMaxClip1() - m_iClip1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));	
			if( pOwner->GetAmmoCount(m_iPrimaryAmmoType) >= GetMaxClip1() )
			{
				m_iClip1 = m_bMagazineStyleReloads ? GetMaxClip1() : m_iClip1 + primary;
				pOwner->RemoveAmmo( m_bMagazineStyleReloads ? GetMaxClip1() : primary, m_iPrimaryAmmoType);
			}
			else
			{
				m_iClip1 = pOwner->GetAmmoCount(m_iPrimaryAmmoType);
				pOwner->RemoveAmmo( GetMaxClip1(), m_iPrimaryAmmoType);
			}
		}

		// If I use secondary clips, reload secondary
		if ( UsesClipsForAmmo2() )
		{
			int secondary = MIN( GetMaxClip2() - m_iClip2, pOwner->GetAmmoCount(m_iSecondaryAmmoType));
			m_iClip2 += secondary;
			pOwner->RemoveAmmo( secondary, m_iSecondaryAmmoType );
		}

		if ( m_bReloadsSingly )
		{
			m_bInReload = false;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Abort any reload we have in progress
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::AbortReload( void )
{
#ifdef CLIENT_DLL
	StopWeaponSound( RELOAD ); 
#endif
	m_bInReload = false;
}

void CBaseCombatWeapon::UpdateAutoFire( void )
{
	if ( !AutoFiresFullClip() )
		return;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( m_iClip1 == 0 )
	{
		// Ready to reload again
		m_bFiringWholeClip = false;
	}

	if ( m_bFiringWholeClip )
	{
		// If it's firing the clip don't let them repress attack to reload
		pOwner->m_nButtons &= ~IN_ATTACK;
	}

	// Don't use the regular reload key
	if ( pOwner->m_nButtons & IN_RELOAD )
	{
		pOwner->m_nButtons &= ~IN_RELOAD;
	}

	// Try to fire if there's ammo in the clip and we're not holding the button
	bool bReleaseClip = m_iClip1 > 0 && !( pOwner->m_nButtons & IN_ATTACK );

	if ( !bReleaseClip )
	{
		if ( CanReload() && ( pOwner->m_nButtons & IN_ATTACK ) )
		{
			// Convert the attack key into the reload key
			pOwner->m_nButtons |= IN_RELOAD;
		}

		// Don't allow attack button if we're not attacking
		pOwner->m_nButtons &= ~IN_ATTACK;
	}
	else
	{
		// Fake the attack key
		pOwner->m_nButtons |= IN_ATTACK;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Primary fire button attack
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::PrimaryAttack( void )
{
	// If my clip is empty (and I use clips) start reload
	if ( UsesClipsForAmmo1() && !m_iClip1 ) 
	{
		Reload();
		return;
	}

	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
	{
		return;
	}

	pPlayer->DoMuzzleFlash();

	// TFO - Since we use PRIMARYATTACK for all our weapons attack methods we can change this to something more reliable.
	// We also use the P38 double hand change-over here.
	if ( m_bIsIronsighted )
	{
		SendWeaponAnim(UseIronsightAnims() ? ACT_VM_AIM_SHOOT : ACT_VM_PRIMARYATTACK);
	}
	else
	{
		SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	}

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	FireBulletsInfo_t info;
	info.m_vecSrc	 = pPlayer->Weapon_ShootPosition( );

	info.m_vecDirShooting = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	info.m_iShots = 0;
	float fireRate = GetFireRate();

	while ( m_flNextPrimaryAttack <= gpGlobals->curtime )
	{
		// MUST call sound before removing a round from the clip of a CMachineGun
		WeaponSound(SINGLE, m_flNextPrimaryAttack);
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
		info.m_iShots++;
		if ( !fireRate )
			break;
	}

	// Make sure we don't fire more than the amount in the clip
	if ( UsesClipsForAmmo1() )
	{
		info.m_iShots = MIN( info.m_iShots, m_iClip1 );
		m_iClip1 -= info.m_iShots;
	}
	else
	{
		info.m_iShots = MIN( info.m_iShots, pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) );
		pPlayer->RemoveAmmo( info.m_iShots, m_iPrimaryAmmoType );
	}

	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 2;

#if !defined( CLIENT_DLL )
	// Fire the bullets
	info.m_vecSpread = pPlayer->GetAttackSpread( this );
#else
	//!!!HACKHACK - what does the client want this function for? 
	info.m_vecSpread = GetActiveWeapon()->GetBulletSpread();
#endif // CLIENT_DLL

	pPlayer->FireBullets( info );

	//Add our view kick in
	AddViewKick();

	m_iShotsFired++;
}

#define BLUDGEON_HULL_DIM		16

static const Vector g_bludgeonMins(-BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM);
static const Vector g_bludgeonMaxs(BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM);

void CBaseCombatWeapon::MeleeAttack(void)
{
	// Try a ray
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	if (pOwner->m_bCanDoMeleeAttack)
		return;

	trace_t traceHit;

	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;

	pOwner->EyeVectors(&forward, NULL, NULL);

	Vector swingEnd = swingStart + forward * tfo_push_range.GetFloat();
	UTIL_TraceLine(swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);

#ifndef CLIENT_DLL
	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo(GetOwner(), GetOwner(), GetBashDamage(), DMG_SLASH);
	TraceAttackToTriggers(triggerInfo, traceHit.startpos, traceHit.endpos, vec3_origin);
#endif

	if (traceHit.fraction == 1.0)
	{
		float bludgeonHullRadius = 1.732f * 16;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

		// Back off by hull "radius"
		swingEnd -= forward * bludgeonHullRadius;

		UTIL_TraceHull(swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
		if (traceHit.fraction < 1.0 && traceHit.m_pEnt)
		{
			Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
			VectorNormalize(vecToTarget);

			float dot = vecToTarget.Dot(forward);

			// YWB:  Make sure they are sort of facing the guy at least...
			if (dot < 0.70721f)
			{
				// Force amiss
				traceHit.fraction = 1.0f;
			}
		}
	}

	Hit(traceHit);
}

void CBaseCombatWeapon::Hit(trace_t &traceHit)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	CBaseEntity	*pHitEntity = traceHit.m_pEnt;

	//Apply damage to a hit target
	if (pHitEntity != NULL)
	{
		Vector hitDirection;
		pPlayer->EyeVectors(&hitDirection, NULL, NULL);
		VectorNormalize(hitDirection);

#ifndef CLIENT_DLL
		CTakeDamageInfo info(GetOwner(), GetOwner(), GetBashDamage(), DMG_SLASH);

		if (pPlayer && pHitEntity->IsNPC())
		{
			// If bonking an NPC, adjust damage.
			info.AdjustPlayerDamageInflictedForSkillLevel();
		}

		CalculateMeleeDamageForce(&info, hitDirection, traceHit.endpos);

		pHitEntity->DispatchTraceAttack(info, hitDirection, &traceHit);
		ApplyMultiDamage();

		// Now hit all triggers along the ray that... 
		TraceAttackToTriggers(info, traceHit.startpos, traceHit.endpos, hitDirection);
#endif

		UTIL_ImpactTrace(&traceHit, DMG_SLASH);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called every frame to check if the weapon is going through transition animations
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::MaintainIdealActivity( void )
{
	// Must be transitioning
	if ( GetActivity() != ACT_TRANSITION )
		return;

	// Must not be at our ideal already 
	if ( ( GetActivity() == m_IdealActivity ) && ( GetSequence() == m_nIdealSequence ) )
		return;

	// Must be finished with the current animation
	if ( IsViewModelSequenceFinished() == false )
		return;

	// Move to the next animation towards our ideal
	SendWeaponAnim( m_IdealActivity );
}

//-----------------------------------------------------------------------------
// Purpose: Sets the ideal activity for the weapon to be in, allowing for transitional animations inbetween
// Input  : ideal - activity to end up at, ideally
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::SetIdealActivity( Activity ideal )
{
	MDLCACHE_CRITICAL_SECTION();
	int	idealSequence = SelectWeightedSequence( ideal );

	if ( idealSequence == -1 )
		return false;

	//Take the new activity
	m_IdealActivity	 = ideal;
	m_nIdealSequence = idealSequence;

	//Find the next sequence in the potential chain of sequences leading to our ideal one
	int nextSequence = FindTransitionSequence( GetSequence(), m_nIdealSequence, NULL );

	// Don't use transitions when we're deploying
	if ( ideal != ACT_VM_DRAW && IsWeaponVisible() && nextSequence != m_nIdealSequence )
	{
		//Set our activity to the next transitional animation
		SetActivity( ACT_TRANSITION );
		SetSequence( nextSequence );	
		SendViewModelAnim( nextSequence );
	}
	else
	{
		//Set our activity to the ideal
		SetActivity( m_IdealActivity );
		SetSequence( m_nIdealSequence );	
		SendViewModelAnim( m_nIdealSequence );
	}

	//Set the next time the weapon will idle
	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
	return true;
}

//-----------------------------------------------------------------------------
// Returns information about the various control panels
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::GetControlPanelInfo( int nPanelIndex, const char *&pPanelName )
{
	pPanelName = NULL;
}

//-----------------------------------------------------------------------------
// Returns information about the various control panels
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::GetControlPanelClassName( int nPanelIndex, const char *&pPanelName )
{
	pPanelName = "vgui_screen";
}


//-----------------------------------------------------------------------------
// Locking a weapon is an exclusive action. If you lock a weapon, that means 
// you are preventing others from doing so for themselves.
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Lock( float lockTime, CBaseEntity *pLocker )
{
	m_flUnlockTime = gpGlobals->curtime + lockTime;
	m_hLocker.Set( pLocker );
}

//-----------------------------------------------------------------------------
// If I'm still locked for a period of time, tell everyone except the person
// that locked me that I'm not available. 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::IsLocked( CBaseEntity *pAsker )
{
	return ( m_flUnlockTime > gpGlobals->curtime && m_hLocker != pAsker );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
Activity CBaseCombatWeapon::ActivityOverride( Activity baseAct, bool *pRequired )
{
	acttable_t *pTable = ActivityList();
	int actCount = ActivityListCount();

	for ( int i = 0; i < actCount; i++, pTable++ )
	{
		if ( baseAct == pTable->baseAct )
		{
			if (pRequired)
			{
				*pRequired = pTable->required;
			}
			return (Activity)pTable->weaponAct;
		}
	}
	return baseAct;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CDmgAccumulator::CDmgAccumulator( void )
{
#ifdef GAME_DLL
	SetDefLessFunc( m_TargetsDmgInfo );
#endif // GAME_DLL

	m_bActive = false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CDmgAccumulator::~CDmgAccumulator()
{
	// Did a weapon get deleted while aggregating CTakeDamageInfo events?
	Assert( !m_bActive );
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Collect trace attacks for weapons that fire multiple bullets per attack that also penetrate
//-----------------------------------------------------------------------------
void CDmgAccumulator::AccumulateMultiDamage( const CTakeDamageInfo &info, CBaseEntity *pEntity )
{
	if ( !pEntity )
		return;

	Assert( m_bActive );

#if defined( GAME_DLL )
	int iIndex = m_TargetsDmgInfo.Find( pEntity->entindex() );
	if ( iIndex == m_TargetsDmgInfo.InvalidIndex() )
	{
		m_TargetsDmgInfo.Insert( pEntity->entindex(), info );
	}
	else
	{
		CTakeDamageInfo *pInfo = &m_TargetsDmgInfo[iIndex];
		if ( pInfo )
		{
			// Update
			m_TargetsDmgInfo[iIndex].AddDamageType( info.GetDamageType() );
			m_TargetsDmgInfo[iIndex].SetDamage( pInfo->GetDamage() + info.GetDamage() );
			m_TargetsDmgInfo[iIndex].SetDamageForce( pInfo->GetDamageForce() + info.GetDamageForce() );
			m_TargetsDmgInfo[iIndex].SetDamagePosition( info.GetDamagePosition() );
			m_TargetsDmgInfo[iIndex].SetReportedPosition( info.GetReportedPosition() );
			m_TargetsDmgInfo[iIndex].SetMaxDamage( MAX( pInfo->GetMaxDamage(), info.GetDamage() ) );
			m_TargetsDmgInfo[iIndex].SetAmmoType( info.GetAmmoType() );
		}

	}
#endif	// GAME_DLL
}

//-----------------------------------------------------------------------------
// Purpose: Send aggregate info
//-----------------------------------------------------------------------------
void CDmgAccumulator::Process( void )
{
	FOR_EACH_MAP( m_TargetsDmgInfo, i )
	{
		CBaseEntity *pEntity = UTIL_EntityByIndex( m_TargetsDmgInfo.Key( i ) );
		if ( pEntity )
		{
			AddMultiDamage( m_TargetsDmgInfo[i], pEntity );
		}
	}

	m_bActive = false;
	m_TargetsDmgInfo.Purge();
}
#endif // GAME_DLL

#if defined( CLIENT_DLL )

// Ironsight proxy TFO
void RecvProxy_ToggleSights( const CRecvProxyData* pData, void* pStruct, void* pOut )
{
	CBaseCombatWeapon *pWeapon = (CBaseCombatWeapon*)pStruct;
	pData->m_Value.m_Int ? pWeapon->EnableIronsights() : pWeapon->DisableIronsights();
}

BEGIN_PREDICTION_DATA( CBaseCombatWeapon )

	DEFINE_PRED_FIELD( m_nNextThinkTick, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	// Networked
	DEFINE_PRED_FIELD( m_hOwner, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
	// DEFINE_FIELD( m_hWeaponFileInfo, FIELD_SHORT ),
	DEFINE_PRED_FIELD( m_iState, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),			 
	DEFINE_PRED_FIELD( m_iViewModelIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_MODELINDEX ),
	DEFINE_PRED_FIELD( m_iWorldModelIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_MODELINDEX ),
	DEFINE_PRED_FIELD_TOL( m_flNextPrimaryAttack, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ),	
	DEFINE_PRED_FIELD_TOL( m_flNextSecondaryAttack, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ),
	DEFINE_PRED_FIELD_TOL( m_flTimeWeaponIdle, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ),
	
	// TFO Holster Sequence
	DEFINE_PRED_FIELD_TOL(m_flHolsteredTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),
	DEFINE_PRED_FIELD( m_bWantsHolster, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bNoSwitch, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),

	DEFINE_PRED_FIELD( m_iPrimaryAmmoType, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_iSecondaryAmmoType, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_iClip1, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),			
	DEFINE_PRED_FIELD( m_iClip2, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),			

	DEFINE_PRED_FIELD( m_nViewModelIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	// Ironsight
	DEFINE_PRED_FIELD( m_bIsIronsighted, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flIronsightedTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),


	// Not networked

	DEFINE_PRED_FIELD( m_flTimeWeaponIdle, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_FIELD( m_bInReload, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bFireOnEmpty, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bFiringWholeClip, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flNextEmptySoundTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_Activity, FIELD_INTEGER ),
	DEFINE_FIELD( m_fFireDuration, FIELD_FLOAT ),
	DEFINE_FIELD( m_iszName, FIELD_INTEGER ),		
	// TFO Clip Reloading.
	DEFINE_FIELD( m_bMagazineStyleReloads, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bFiresUnderwater, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bAltFiresUnderwater, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fMinRange1, FIELD_FLOAT ),		
	DEFINE_FIELD( m_fMinRange2, FIELD_FLOAT ),		
	DEFINE_FIELD( m_fMaxRange1, FIELD_FLOAT ),		
	DEFINE_FIELD( m_fMaxRange2, FIELD_FLOAT ),		
	DEFINE_FIELD( m_bReloadsSingly, FIELD_BOOLEAN ),	
	DEFINE_FIELD( m_bRemoveable, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iPrimaryAmmoCount, FIELD_INTEGER ),
	DEFINE_FIELD( m_iSecondaryAmmoCount, FIELD_INTEGER ),

	//DEFINE_PHYSPTR( m_pConstraint ),

	// DEFINE_FIELD( m_iOldState, FIELD_INTEGER ),
	// DEFINE_FIELD( m_bJustRestored, FIELD_BOOLEAN ),

	// DEFINE_FIELD( m_OnPlayerPickup, COutputEvent ),
	// DEFINE_FIELD( m_pConstraint, FIELD_INTEGER ),

	END_PREDICTION_DATA()

#endif	// ! CLIENT_DLL

	// Special hack since we're aliasing the name C_BaseCombatWeapon with a macro on the client
	IMPLEMENT_NETWORKCLASS_ALIASED( BaseCombatWeapon, DT_BaseCombatWeapon )

#if !defined( CLIENT_DLL )
	//-----------------------------------------------------------------------------
	// Purpose: Save Data for Base Weapon object
	//-----------------------------------------------------------------------------// 
	BEGIN_DATADESC( CBaseCombatWeapon )

	DEFINE_FIELD( m_flNextPrimaryAttack, FIELD_TIME ),
	DEFINE_FIELD( m_flNextSecondaryAttack, FIELD_TIME ),
	DEFINE_FIELD( m_flTimeWeaponIdle, FIELD_TIME ),

	DEFINE_FIELD( m_bInReload, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bFireOnEmpty, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_hOwner, FIELD_EHANDLE ),

	DEFINE_FIELD( m_iState, FIELD_INTEGER ),
	DEFINE_FIELD( m_iszName, FIELD_STRING ),
	DEFINE_FIELD( m_iPrimaryAmmoType, FIELD_INTEGER ),
	DEFINE_FIELD( m_iSecondaryAmmoType, FIELD_INTEGER ),
	DEFINE_FIELD( m_iClip1, FIELD_INTEGER ),
	DEFINE_FIELD( m_iClip2, FIELD_INTEGER ),
	// TFO Clip Reloading.
	DEFINE_FIELD( m_bMagazineStyleReloads, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bFiresUnderwater, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bAltFiresUnderwater, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fMinRange1, FIELD_FLOAT ),
	DEFINE_FIELD( m_fMinRange2, FIELD_FLOAT ),
	DEFINE_FIELD( m_fMaxRange1, FIELD_FLOAT ),
	DEFINE_FIELD( m_fMaxRange2, FIELD_FLOAT ),

	DEFINE_FIELD( m_iPrimaryAmmoCount, FIELD_INTEGER ),
	DEFINE_FIELD( m_iSecondaryAmmoCount, FIELD_INTEGER ),

	DEFINE_FIELD( m_nViewModelIndex, FIELD_INTEGER ),

	// don't save these, init to 0 and regenerate
	//	DEFINE_FIELD( m_flNextEmptySoundTime, FIELD_TIME ),
	//	DEFINE_FIELD( m_Activity, FIELD_INTEGER ),
	DEFINE_FIELD( m_nIdealSequence, FIELD_INTEGER ),
	DEFINE_FIELD( m_IdealActivity, FIELD_INTEGER ),

	DEFINE_FIELD( m_fFireDuration, FIELD_FLOAT ),

	DEFINE_FIELD( m_bReloadsSingly, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iSubType, FIELD_INTEGER ),
	DEFINE_FIELD( m_bRemoveable, FIELD_BOOLEAN ),

	DEFINE_FIELD( m_flUnlockTime,		FIELD_TIME ),
	DEFINE_FIELD( m_hLocker,			FIELD_EHANDLE ),

	//	DEFINE_FIELD( m_iViewModelIndex, FIELD_INTEGER ),
	//	DEFINE_FIELD( m_iWorldModelIndex, FIELD_INTEGER ),
	//  DEFINE_FIELD( m_hWeaponFileInfo, ???? ),

	DEFINE_PHYSPTR( m_pConstraint ),

	// Just to quiet classcheck.. this field exists only on the client
	//	DEFINE_FIELD( m_iOldState, FIELD_INTEGER ),
	//	DEFINE_FIELD( m_bJustRestored, FIELD_BOOLEAN ),

	// Function pointers
	DEFINE_ENTITYFUNC( DefaultTouch ),
	DEFINE_THINKFUNC( FallThink ),
	DEFINE_THINKFUNC( Materialize ),
	DEFINE_THINKFUNC( AttemptToMaterialize ),
	DEFINE_THINKFUNC( DestroyItem ),
	DEFINE_THINKFUNC( SetPickupTouch ),

	DEFINE_THINKFUNC( HideThink ),
	DEFINE_INPUTFUNC( FIELD_VOID, "HideWeapon", InputHideWeapon ),

	// Outputs
	DEFINE_OUTPUT( m_OnPlayerUse, "OnPlayerUse"),
	DEFINE_OUTPUT( m_OnPlayerPickup, "OnPlayerPickup"),
	DEFINE_OUTPUT( m_OnNPCPickup, "OnNPCPickup"),
	DEFINE_OUTPUT( m_OnCacheInteraction, "OnCacheInteraction" ),

	END_DATADESC()

	//-----------------------------------------------------------------------------
	// Purpose: Only send to local player if this weapon is the active weapon
	// Input  : *pStruct - 
	//			*pVarData - 
	//			*pRecipients - 
	//			objectID - 
	// Output : void*
	//-----------------------------------------------------------------------------
	void* SendProxy_SendActiveLocalWeaponDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
{
	// Get the weapon entity
	CBaseCombatWeapon *pWeapon = (CBaseCombatWeapon*)pVarData;
	if ( pWeapon )
	{
		// Only send this chunk of data to the player carrying this weapon
		CBasePlayer *pPlayer = ToBasePlayer( pWeapon->GetOwner() );
		if ( pPlayer /*&& pPlayer->GetActiveWeapon() == pWeapon*/ )
		{
			pRecipients->SetOnly( pPlayer->GetClientIndex() );
			return (void*)pVarData;
		}
	}

	return NULL;
}
REGISTER_SEND_PROXY_NON_MODIFIED_POINTER( SendProxy_SendActiveLocalWeaponDataTable );

//-----------------------------------------------------------------------------
// Purpose: Only send the LocalWeaponData to the player carrying the weapon
//-----------------------------------------------------------------------------
void* SendProxy_SendLocalWeaponDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
{
	// Get the weapon entity
	CBaseCombatWeapon *pWeapon = (CBaseCombatWeapon*)pVarData;
	if ( pWeapon )
	{
		// Only send this chunk of data to the player carrying this weapon
		CBasePlayer *pPlayer = ToBasePlayer( pWeapon->GetOwner() );
		if ( pPlayer )
		{
			pRecipients->SetOnly( pPlayer->GetClientIndex() );
			return (void*)pVarData;
		}
	}

	return NULL;
}
REGISTER_SEND_PROXY_NON_MODIFIED_POINTER( SendProxy_SendLocalWeaponDataTable );

//-----------------------------------------------------------------------------
// Purpose: Only send to non-local players
//-----------------------------------------------------------------------------
void* SendProxy_SendNonLocalWeaponDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
{
	pRecipients->SetAllRecipients();

	CBaseCombatWeapon *pWeapon = (CBaseCombatWeapon*)pVarData;
	if ( pWeapon )
	{
		CBasePlayer *pPlayer = ToBasePlayer( pWeapon->GetOwner() );
		if ( pPlayer )
		{
			pRecipients->ClearRecipient( pPlayer->GetClientIndex() );
			return ( void * )pVarData;
		}
	}

	return NULL;
}
REGISTER_SEND_PROXY_NON_MODIFIED_POINTER( SendProxy_SendNonLocalWeaponDataTable );

#endif

#if PREDICTION_ERROR_CHECK_LEVEL > 1
#define SendPropTime SendPropFloat
#define RecvPropTime RecvPropFloat
#endif

//-----------------------------------------------------------------------------
// Purpose: Propagation data for weapons. Only sent when a player's holding it.
//-----------------------------------------------------------------------------
BEGIN_NETWORK_TABLE_NOBASE( CBaseCombatWeapon, DT_LocalActiveWeaponData )
#if !defined( CLIENT_DLL )
	SendPropTime( SENDINFO( m_flNextPrimaryAttack ) ),
	SendPropTime( SENDINFO( m_flNextSecondaryAttack ) ),
	SendPropInt( SENDINFO( m_nNextThinkTick ) ),
	SendPropTime( SENDINFO( m_flTimeWeaponIdle ) ),
#else
	RecvPropTime( RECVINFO( m_flNextPrimaryAttack ) ),
	RecvPropTime( RECVINFO( m_flNextSecondaryAttack ) ),
	RecvPropInt( RECVINFO( m_nNextThinkTick ) ),
	RecvPropTime( RECVINFO( m_flTimeWeaponIdle ) ),
#endif
	END_NETWORK_TABLE()

	//-----------------------------------------------------------------------------
	// Purpose: Propagation data for weapons. Only sent when a player's holding it.
	//-----------------------------------------------------------------------------
	BEGIN_NETWORK_TABLE_NOBASE( CBaseCombatWeapon, DT_LocalWeaponData )
#if !defined( CLIENT_DLL )
	SendPropIntWithMinusOneFlag( SENDINFO(m_iClip1 ), 8 ),
	SendPropIntWithMinusOneFlag( SENDINFO(m_iClip2 ), 8 ),
	SendPropInt( SENDINFO(m_iPrimaryAmmoType ), 8 ),
	SendPropInt( SENDINFO(m_iSecondaryAmmoType ), 8 ),
	SendPropInt( SENDINFO( m_nViewModelIndex ), VIEWMODEL_INDEX_BITS, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_bFlipViewModel ) ),
#else
	RecvPropIntWithMinusOneFlag( RECVINFO(m_iClip1 )),
	RecvPropIntWithMinusOneFlag( RECVINFO(m_iClip2 )),
	RecvPropInt( RECVINFO(m_iPrimaryAmmoType )),
	RecvPropInt( RECVINFO(m_iSecondaryAmmoType )),
	RecvPropInt( RECVINFO( m_nViewModelIndex ) ),
	RecvPropBool( RECVINFO( m_bFlipViewModel ) ),
#endif
	END_NETWORK_TABLE()

	BEGIN_NETWORK_TABLE(CBaseCombatWeapon, DT_BaseCombatWeapon)
#if !defined( CLIENT_DLL )
	SendPropDataTable("LocalWeaponData", 0, &REFERENCE_SEND_TABLE(DT_LocalWeaponData), SendProxy_SendLocalWeaponDataTable ),
	SendPropDataTable("LocalActiveWeaponData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponData), SendProxy_SendActiveLocalWeaponDataTable ),
	SendPropModelIndex( SENDINFO(m_iViewModelIndex) ),
	SendPropModelIndex( SENDINFO(m_iWorldModelIndex) ),
	SendPropInt( SENDINFO(m_iState ), 8, SPROP_UNSIGNED ),
	SendPropEHandle( SENDINFO(m_hOwner) ),
	// Ironsight
	SendPropBool( SENDINFO( m_bIsIronsighted ) ),
	SendPropFloat( SENDINFO( m_flIronsightedTime ) ),
	// Holstering
	SendPropBool( SENDINFO( m_bWantsHolster ) ),
	SendPropBool(SENDINFO(m_bNoSwitch)),
	SendPropTime( SENDINFO( m_flHolsteredTime ) ),
#else
	RecvPropDataTable("LocalWeaponData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalWeaponData)),
	RecvPropDataTable("LocalActiveWeaponData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponData)),
	RecvPropInt( RECVINFO(m_iViewModelIndex)),
	RecvPropInt( RECVINFO(m_iWorldModelIndex)),
	RecvPropInt( RECVINFO(m_iState )),
	RecvPropEHandle( RECVINFO(m_hOwner ) ),
	// Ironsight + proxy!
	RecvPropInt( RECVINFO( m_bIsIronsighted ), 0, RecvProxy_ToggleSights ), //note: RecvPropBool is actually RecvPropInt (see its implementation), but we need a proxy
	RecvPropFloat( RECVINFO( m_flIronsightedTime ) ),
	// Holstering
	RecvPropBool(RECVINFO(m_bWantsHolster)),
	RecvPropBool(RECVINFO(m_bNoSwitch)),
	RecvPropTime(RECVINFO(m_flHolsteredTime)),
#endif
	END_NETWORK_TABLE()
