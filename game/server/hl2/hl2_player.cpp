//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for HL2.
//
//=============================================================================//

#include "cbase.h"
#include "hl2_player.h"
#include "globalstate.h"
#include "game.h"
#include "gamerules.h"
#include "trains.h"
#include "basehlcombatweapon_shared.h"
#include "vcollide_parse.h"
#include "in_buttons.h"
#include "ai_interactions.h"
#include "ai_squad.h"
#include "igamemovement.h"
#include "ai_hull.h"
#include "hl2_shareddefs.h"
#include "info_camera_link.h"
#include "point_camera.h"
#include "engine/IEngineSound.h"
#include "ndebugoverlay.h"
#include "iservervehicle.h"
#include "IVehicle.h"
#include "globals.h"
#include "collisionutils.h"
#include "coordsize.h"
#include "effect_color_tables.h"
#include "vphysics/player_controller.h"
#include "player_pickup.h"
#include "player_pickup_controller.h"
#include "script_intro.h"
#include "effect_dispatch_data.h"
#include "te_effect_dispatch.h" 
#include "ai_basenpc.h"
#include "AI_Criteria.h"
#include "entitylist.h"
#include "env_zoom.h"
#include "hl2_gamerules.h"
#include "datacache/imdlcache.h"
#include "eventqueue.h"
#include "doors.h"
#include "buttons.h"
#include "achievement_manager.h"
#include "filters.h"
#include "tier0/icommandline.h"

#ifdef PORTAL
#include "portal_player.h"
#endif // PORTAL

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar weapon_showproficiency;

// Do not touch with without seeing me, please! (sjb)
// For consistency's sake, enemy gunfire is traced against a scaled down
// version of the player's hull, not the hitboxes for the player's model
// because the player isn't aware of his model, and can't do anything about
// preventing headshots and other such things. Also, game difficulty will
// not change if the model changes. This is the value by which to scale
// the X/Y of the player's hull to get the volume to trace bullets against.
#define PLAYER_HULL_REDUCTION	0.70

// This switches between the single primary weapon, and multiple weapons with buckets approach (jdw)
#define	HL2_SINGLE_PRIMARY_WEAPON_MODE	0

#define TIME_IGNORE_FALL_DAMAGE 10.0

#define PLAYER_MODEL "models/player.mdl"

extern int gEvilImpulse101;

ConVar sv_autojump( "sv_autojump", "0" );

ConVar hl2_walkspeed( "hl2_walkspeed", "100" );
ConVar hl2_normspeed( "hl2_normspeed", "110" );
ConVar hl2_sprintspeed( "hl2_sprintspeed", "220" );

#ifdef HL2MP
	#define	HL2_WALK_SPEED 100
	#define	HL2_NORM_SPEED 110
	#define	HL2_SPRINT_SPEED 220
#else
	#define	HL2_WALK_SPEED hl2_walkspeed.GetFloat()
	#define	HL2_NORM_SPEED hl2_normspeed.GetFloat()
	#define	HL2_SPRINT_SPEED hl2_sprintspeed.GetFloat()
#endif

ConVar player_showpredictedposition( "player_showpredictedposition", "0" );
ConVar player_showpredictedposition_timestep( "player_showpredictedposition_timestep", "1.0" );

ConVar player_squad_transient_commands( "player_squad_transient_commands", "1", FCVAR_REPLICATED );
ConVar player_squad_double_tap_time( "player_squad_double_tap_time", "0.25" );

ConVar sv_infinite_aux_power( "sv_infinite_aux_power", "0", FCVAR_CHEAT );
ConVar sv_stickysprint("sv_stickysprint", "0", FCVAR_ARCHIVE | FCVAR_ARCHIVE_XBOX);

ConVar* g_pConVarDialogueMenu = NULL;

//==============================================================================================
// CAPPED PLAYER PHYSICS DAMAGE TABLE
//==============================================================================================
static impactentry_t cappedPlayerLinearTable[] =
{
	{ 150*150, 5 },
	{ 250*250, 10 },
	{ 450*450, 20 },
	{ 550*550, 30 },
	//{ 700*700, 100 },
	//{ 1000*1000, 500 },
};

static impactentry_t cappedPlayerAngularTable[] =
{
	{ 100*100, 10 },
	{ 150*150, 20 },
	{ 200*200, 30 },
	//{ 300*300, 500 },
};

static impactdamagetable_t gCappedPlayerImpactDamageTable =
{
	cappedPlayerLinearTable,
	cappedPlayerAngularTable,

	ARRAYSIZE(cappedPlayerLinearTable),
	ARRAYSIZE(cappedPlayerAngularTable),

	24*24.0f,	// minimum linear speed
	360*360.0f,	// minimum angular speed
	2.0f,		// can't take damage from anything under 2kg

	5.0f,		// anything less than 5kg is "small"
	5.0f,		// never take more than 5 pts of damage from anything under 5kg
	36*36.0f,	// <5kg objects must go faster than 36 in/s to do damage

	0.0f,		// large mass in kg (no large mass effects)
	1.0f,		// large mass scale
	2.0f,		// large mass falling scale
	320.0f,		// min velocity for player speed to cause damage

};

//-----------------------------------------------------------------------------
// Purpose: Used to relay outputs/inputs from the player to the world and viceversa
//-----------------------------------------------------------------------------
class CLogicPlayerProxy : public CLogicalEntity
{
	DECLARE_CLASS( CLogicPlayerProxy, CLogicalEntity );

private:

	DECLARE_DATADESC();

public:

	COutputEvent m_PlayerHasAmmo;
	COutputEvent m_PlayerHasNoAmmo;
	COutputEvent m_PlayerDied;
	COutputInt m_RequestedPlayerHealth;

	void InputRequestPlayerHealth( inputdata_t &inputdata );
	void InputSetPlayerHealth( inputdata_t &inputdata );
	void InputRequestAmmoState( inputdata_t &inputdata );
	void InputLowerWeapon( inputdata_t &inputdata );
	void InputEnableCappedPhysicsDamage( inputdata_t &inputdata );
	void InputDisableCappedPhysicsDamage( inputdata_t &inputdata );

	void Activate ( void );
	bool PassesDamageFilter( const CTakeDamageInfo &info );

	EHANDLE m_hPlayer;
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void CC_ToggleZoom( void )
{
	CBasePlayer* pPlayer = UTIL_GetCommandClient();

	if( pPlayer )
	{
		CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player*>(pPlayer);

		if( pHL2Player )
		{
			pHL2Player->ToggleZoom();
		}
	}
}

static ConCommand toggle_zoom("toggle_zoom", CC_ToggleZoom, "Toggles zoom display" );

// ConVar cl_forwardspeed( "cl_forwardspeed", "400", FCVAR_CHEAT ); // Links us to the client's version
ConVar xc_crouch_range( "xc_crouch_range", "0.85", FCVAR_ARCHIVE, "Percentarge [1..0] of joystick range to allow ducking within" );	// Only 1/2 of the range is used
ConVar xc_use_crouch_limiter( "xc_use_crouch_limiter", "0", FCVAR_ARCHIVE, "Use the crouch limiting logic on the controller" );

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void CC_ToggleDuck( void )
{
	CBasePlayer* pPlayer = UTIL_GetCommandClient();
	if ( pPlayer == NULL )
		return;

	// Cannot be frozen
	if ( pPlayer->GetFlags() & FL_FROZEN )
		return;

	static bool		bChecked = false;
	static ConVar *pCVcl_forwardspeed = NULL;
	if ( !bChecked )
	{
		bChecked = true;
		pCVcl_forwardspeed = ( ConVar * )cvar->FindVar( "cl_forwardspeed" );
	}


	// If we're not ducked, do extra checking
	if ( xc_use_crouch_limiter.GetBool() )
	{
		if ( pPlayer->GetToggledDuckState() == false )
		{
			float flForwardSpeed = 400.0f;
			if ( pCVcl_forwardspeed )
			{
				flForwardSpeed = pCVcl_forwardspeed->GetFloat();
			}

			flForwardSpeed = MAX( 1.0f, flForwardSpeed );

			// Make sure we're not in the blindspot on the crouch detection
			float flStickDistPerc = ( pPlayer->GetStickDist() / flForwardSpeed ); // Speed is the magnitude
			if ( flStickDistPerc > xc_crouch_range.GetFloat() )
				return;
		}
	}

	// Toggle the duck
	pPlayer->ToggleDuck();
}

static ConCommand toggle_duck("toggle_duck", CC_ToggleDuck, "Toggles duck" );

#ifndef HL2MP
#ifndef PORTAL
LINK_ENTITY_TO_CLASS( player, CHL2_Player );
#endif
#endif

PRECACHE_REGISTER(player);

CBaseEntity *FindEntityForward( CBasePlayer *pMe, bool fHull );

BEGIN_SIMPLE_DATADESC( LadderMove_t )
	DEFINE_FIELD( m_bForceLadderMove, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bForceMount, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flStartTime, FIELD_TIME ),
	DEFINE_FIELD( m_flArrivalTime, FIELD_TIME ),
	DEFINE_FIELD( m_vecGoalPosition, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vecStartPosition, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_hForceLadder, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hReservedSpot, FIELD_EHANDLE ),
END_DATADESC()

// Global Savedata for HL2 player
BEGIN_DATADESC( CHL2_Player )

	DEFINE_FIELD( m_nControlClass, FIELD_INTEGER ),
	DEFINE_EMBEDDED( m_HL2Local ),

	DEFINE_FIELD( m_bSprintEnabled, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flTimeAllSuitDevicesOff, FIELD_TIME ),
	DEFINE_FIELD( m_fIsSprinting, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fIsWalking, FIELD_BOOLEAN ),

	DEFINE_AUTO_ARRAY( m_vecMissPositions, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_nNumMissPositions, FIELD_INTEGER ),

	DEFINE_FIELD( m_flTimeIgnoreFallDamage, FIELD_TIME ),
	DEFINE_FIELD( m_bIgnoreFallDamageResetAfterImpact, FIELD_BOOLEAN ),

	// Suit power fields
	DEFINE_FIELD( m_flSuitPowerLoad, FIELD_FLOAT ),

	DEFINE_FIELD( m_flIdleTime, FIELD_TIME ),
	DEFINE_FIELD( m_flMoveTime, FIELD_TIME ),
	DEFINE_FIELD( m_flLastDamageTime, FIELD_TIME ),
	DEFINE_FIELD( m_flTargetFindTime, FIELD_TIME ),
	DEFINE_FIELD(m_flSprintExhaustionTime, FIELD_TIME),

	DEFINE_FIELD( m_bUseCappedPhysicsDamageTable, FIELD_BOOLEAN ),

	DEFINE_EMBEDDED( m_LowerWeaponTimer ),

	DEFINE_INPUTFUNC( FIELD_FLOAT, "IgnoreFallDamage", InputIgnoreFallDamage ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "IgnoreFallDamageWithoutReset", InputIgnoreFallDamageWithoutReset ),
	DEFINE_INPUTFUNC( FIELD_VOID, "OnSquadMemberKilled", OnSquadMemberKilled ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ForceDropPhysObjects", InputForceDropPhysObjects ),

	DEFINE_SOUNDPATCH( m_sndLeeches ),
	DEFINE_SOUNDPATCH( m_sndWaterSplashes ),

	DEFINE_FIELD( m_flArmorReductionTime, FIELD_TIME ),
	DEFINE_FIELD( m_iArmorReductionFrom, FIELD_INTEGER ),

	DEFINE_FIELD( m_flTimeUseSuspended, FIELD_TIME ),

	//DEFINE_FIELD( m_hPlayerProxy, FIELD_EHANDLE ), //Shut up class check!

END_DATADESC()

CHL2_Player::CHL2_Player()
{
	// TFO
	ItemTakeAnim = false;
	NextShoots = 0;
	IsZoomin = false;
	m_bCanSearchForEnts = true;
	m_flCheckForItems = 0.0f;
	m_flSprintExhaustionTime = 0.0f;

	m_nNumMissPositions	= 0;
	m_pPlayerAISquad = 0;
	m_bSprintEnabled = true;

	m_flArmorReductionTime = 0.0f;
	m_iArmorReductionFrom = 0;
}

//
// SUIT POWER DEVICES
//
#define SUITPOWER_CHARGE_RATE	8.5											// 100 units in 8 seconds

CSuitPowerDevice SuitDeviceSprint(bits_SUIT_DEVICE_SPRINT, 8.5f);				// 100 units in 8 seconds
CSuitPowerDevice SuitDeviceBreather( bits_SUIT_DEVICE_BREATHER, 6.7f );		// 100 units in 15 seconds (plus three padded seconds)

IMPLEMENT_SERVERCLASS_ST(CHL2_Player, DT_HL2_Player)
	SendPropDataTable(SENDINFO_DT(m_HL2Local), &REFERENCE_SEND_TABLE(DT_HL2Local), SendProxy_SendLocalDataTable),
	SendPropBool( SENDINFO(m_fIsSprinting) ),
END_SEND_TABLE()

void CHL2_Player::Precache( void )
{
	BaseClass::Precache();

	PrecacheModel(PLAYER_MODEL);
	PrecacheScriptSound( "HL2Player.FlashLightOn" );
	PrecacheScriptSound( "HL2Player.FlashLightOff" );
	PrecacheScriptSound( "HL2Player.PickupWeapon" );
	PrecacheScriptSound( "HL2Player.BurnPain" );
	PrecacheScriptSound( "PlayerSprinty.Regen" );
	PrecacheScriptSound( "TFO.Paper" );
	PrecacheScriptSound( "Weapon.Lower" );
	PrecacheScriptSound( "Weapon.Raise" );
	PrecacheScriptSound("HealthKit.Touch");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHL2_Player::CheckSuitZoom( void )
{
	if (g_pConVarDialogueMenu == NULL)
		g_pConVarDialogueMenu = cvar->FindVar("cl_dialoguepanel");

	if (m_bIsInCamView || !g_pConVarDialogueMenu)
		return;

	if (!g_pConVarDialogueMenu->GetBool() && IsZoomin)
	{
		IsZoomin = false;
		StopZooming();
	}
	else if (g_pConVarDialogueMenu->GetBool() && !IsZoomin)
	{
		IsZoomin = true;
		StartZooming();
	}
}

void CHL2_Player::EquipSuit()
{
	MDLCACHE_CRITICAL_SECTION();
	BaseClass::EquipSuit();
}

void CHL2_Player::RemoveSuit( void )
{
	BaseClass::RemoveSuit();
}

void CHL2_Player::HandleSpeedChanges( void )
{
	int buttonsChanged = m_afButtonPressed | m_afButtonReleased;

	bool bCanSprint = CanSprint();
	bool bIsSprinting = IsSprinting();
	bool bWantSprint = ( bCanSprint && (m_nButtons & IN_SPEED) );
	if ( bIsSprinting != bWantSprint && (buttonsChanged & IN_SPEED) )
	{
		// If someone wants to sprint, make sure they've pressed the button to do so. We want to prevent the
		// case where a player can hold down the sprint key and burn tiny bursts of sprint as the suit recharges
		// We want a full debounce of the key to resume sprinting after the suit is completely drained
		if ( bWantSprint )
		{
			if ( sv_stickysprint.GetBool() )
			{
				StartAutoSprint();
			}
			else
			{
				StartSprinting();
			}
		}
		else
		{
			if ( !sv_stickysprint.GetBool() )
			{
				StopSprinting();
			}
			// Reset key, so it will be activated post whatever is suppressing it.
			m_nButtons &= ~IN_SPEED;
		}
	}

	bool bIsWalking = IsWalking();
	// have suit, pressing button, not sprinting or ducking
	bool bWantWalking = bWantWalking = (m_nButtons & IN_WALK) && !IsSprinting() && !(m_nButtons & IN_DUCK);
	
	if( bIsWalking != bWantWalking )
	{
		if ( bWantWalking )
		{
			StartWalking();
		}
		else
		{
			StopWalking();
		}
	}
}

//-----------------------------------------------------------------------------
// This happens when we powerdown from the mega physcannon to the regular one
//-----------------------------------------------------------------------------
void CHL2_Player::HandleArmorReduction( void )
{
	if ( m_flArmorReductionTime < gpGlobals->curtime )
		return;

	if ( ArmorValue() <= 0 )
		return;

	float flPercent = 1.0f - (( m_flArmorReductionTime - gpGlobals->curtime ) / ARMOR_DECAY_TIME );

	int iArmor = Lerp( flPercent, m_iArmorReductionFrom, 0 );

	SetArmorValue( iArmor );
}

//-----------------------------------------------------------------------------
// Purpose: Allow pre-frame adjustments on the player
//-----------------------------------------------------------------------------
void CHL2_Player::PreThink(void)
{
	if ( player_showpredictedposition.GetBool() )
	{
		Vector	predPos;

		UTIL_PredictedPosition( this, player_showpredictedposition_timestep.GetFloat(), &predPos );

		NDebugOverlay::Box( predPos, NAI_Hull::Mins( GetHullType() ), NAI_Hull::Maxs( GetHullType() ), 0, 255, 0, 0, 0.01f );
		NDebugOverlay::Line( GetAbsOrigin(), predPos, 0, 255, 0, 0, 0.01f );
	}

	// Riding a vehicle?
	if ( IsInAVehicle() )	
	{
		VPROF( "CHL2_Player::PreThink-Vehicle" );
		// make sure we update the client, check for timed damage and update suit even if we are in a vehicle
		UpdateClientData();		
		CheckTimeBasedDamage();

		// Allow the suit to recharge when in the vehicle.
		SuitPower_Update();
		CheckSuitZoom();

		WaterMove();	
		return;
	}

	// This is an experiment of mine- autojumping! 
	// only affects you if sv_autojump is nonzero.
	if( (GetFlags() & FL_ONGROUND) && sv_autojump.GetFloat() != 0 )
	{
		VPROF( "CHL2_Player::PreThink-Autojump" );
		// check autojump
		Vector vecCheckDir;

		vecCheckDir = GetAbsVelocity();

		float flVelocity = VectorNormalize( vecCheckDir );

		if( flVelocity > 200 )
		{
			// Going fast enough to autojump
			vecCheckDir = WorldSpaceCenter() + vecCheckDir * 34 - Vector( 0, 0, 16 );

			trace_t tr;

			UTIL_TraceHull( WorldSpaceCenter() - Vector( 0, 0, 16 ), vecCheckDir, NAI_Hull::Mins(HULL_TINY_CENTERED),NAI_Hull::Maxs(HULL_TINY_CENTERED), MASK_PLAYERSOLID, this, COLLISION_GROUP_PLAYER, &tr );
			
			//NDebugOverlay::Line( tr.startpos, tr.endpos, 0,255,0, true, 10 );

			if( tr.fraction == 1.0 && !tr.startsolid )
			{
				// Now trace down!
				UTIL_TraceLine( vecCheckDir, vecCheckDir - Vector( 0, 0, 64 ), MASK_PLAYERSOLID, this, COLLISION_GROUP_NONE, &tr );

				//NDebugOverlay::Line( tr.startpos, tr.endpos, 0,255,0, true, 10 );

				if( tr.fraction == 1.0 && !tr.startsolid )
				{
					// !!!HACKHACK
					// I KNOW, I KNOW, this is definitely not the right way to do this,
					// but I'm prototyping! (sjb)
					Vector vecNewVelocity = GetAbsVelocity();
					vecNewVelocity.z += 250;
					SetAbsVelocity( vecNewVelocity );
				}
			}
		}
	}

	VPROF_SCOPE_BEGIN( "CHL2_Player::PreThink-Speed" );
	HandleSpeedChanges();
#ifdef HL2_EPISODIC
	HandleArmorReduction();
#endif

	if( sv_stickysprint.GetBool() && m_bIsAutoSprinting )
	{
		// If we're ducked and not in the air
		if( (this->m_nButtons & IN_BACK) || (IsDucked() && GetGroundEntity() != NULL) )
		{
			StopSprinting();
		}
		// Stop sprinting if the player lets off the stick for a moment.
		else if( GetStickDist() == 0.0f )
		{
			if( gpGlobals->curtime > m_fAutoSprintMinTime )
			{
				StopSprinting();
			}
		}
		else
		{
			// Stop sprinting one half second after the player stops inputting with the move stick.
			m_fAutoSprintMinTime = gpGlobals->curtime + 0.5f;
		}
	}
	else if ( IsSprinting() )
	{
		// Disable sprint while ducked unless we're in the air (jumping) 
		// Don't allow sprinting backwards... wut?
		if ( (this->m_nButtons & IN_BACK) || (IsDucked() && ( GetGroundEntity() != NULL )) )
		{
			StopSprinting();
		}
	}

	VPROF_SCOPE_END();

	if ( g_fGameOver || IsPlayerLockedInPlace() )
		return;         // finale

	VPROF_SCOPE_BEGIN( "CHL2_Player::PreThink-ItemPreFrame" );
	ItemPreFrame( );
	VPROF_SCOPE_END();

	VPROF_SCOPE_BEGIN( "CHL2_Player::PreThink-WaterMove" );
	WaterMove();
	VPROF_SCOPE_END();

	// Operate suit accessories and manage power consumption/charge
	VPROF_SCOPE_BEGIN( "CHL2_Player::PreThink-SuitPower_Update" );
	SuitPower_Update();
	VPROF_SCOPE_END();

	// checks if new client data (for HUD and view control) needs to be sent to the client
	VPROF_SCOPE_BEGIN( "CHL2_Player::PreThink-UpdateClientData" );
	UpdateClientData();
	VPROF_SCOPE_END();
	
	VPROF_SCOPE_BEGIN( "CHL2_Player::PreThink-CheckTimeBasedDamage" );
	CheckTimeBasedDamage();
	VPROF_SCOPE_END();

	VPROF_SCOPE_BEGIN( "CHL2_Player::PreThink-CheckSuitZoom" );
	CheckSuitZoom();
	VPROF_SCOPE_END();

	if (m_lifeState >= LIFE_DYING)
	{
		PlayerDeathThink();
		return;
	}

	// So the correct flags get sent to client asap.
	//
	if ( m_afPhysicsFlags & PFLAG_DIROVERRIDE )
		AddFlag( FL_ONTRAIN );
	else 
		RemoveFlag( FL_ONTRAIN );

	// Train speed control
	if ( m_afPhysicsFlags & PFLAG_DIROVERRIDE )
	{
		CBaseEntity *pTrain = GetGroundEntity();
		float vel;

		if ( pTrain )
		{
			if ( !(pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) )
				pTrain = NULL;
		}
		
		if ( !pTrain )
		{
			if ( GetActiveWeapon() && (GetActiveWeapon()->ObjectCaps() & FCAP_DIRECTIONAL_USE) )
			{
				m_iTrain = TRAIN_ACTIVE | TRAIN_NEW;

				if ( m_nButtons & IN_FORWARD )
				{
					m_iTrain |= TRAIN_FAST;
				}
				else if ( m_nButtons & IN_BACK )
				{
					m_iTrain |= TRAIN_BACK;
				}
				else
				{
					m_iTrain |= TRAIN_NEUTRAL;
				}
				return;
			}
			else
			{
				trace_t trainTrace;
				// Maybe this is on the other side of a level transition
				UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + Vector(0,0,-38), 
					MASK_PLAYERSOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trainTrace );

				if ( trainTrace.fraction != 1.0 && trainTrace.m_pEnt )
					pTrain = trainTrace.m_pEnt;


				if ( !pTrain || !(pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) || !pTrain->OnControls(this) )
				{
//					Warning( "In train mode with no train!\n" );
					m_afPhysicsFlags &= ~PFLAG_DIROVERRIDE;
					m_iTrain = TRAIN_NEW|TRAIN_OFF;
					return;
				}
			}
		}
		else if ( !( GetFlags() & FL_ONGROUND ) || pTrain->HasSpawnFlags( SF_TRACKTRAIN_NOCONTROL ) || (m_nButtons & (IN_MOVELEFT|IN_MOVERIGHT) ) )
		{
			// Turn off the train if you jump, strafe, or the train controls go dead
			m_afPhysicsFlags &= ~PFLAG_DIROVERRIDE;
			m_iTrain = TRAIN_NEW|TRAIN_OFF;
			return;
		}

		SetAbsVelocity( vec3_origin );
		vel = 0;
		if ( m_afButtonPressed & IN_FORWARD )
		{
			vel = 1;
			pTrain->Use( this, this, USE_SET, (float)vel );
		}
		else if ( m_afButtonPressed & IN_BACK )
		{
			vel = -1;
			pTrain->Use( this, this, USE_SET, (float)vel );
		}

		if (vel)
		{
			m_iTrain = TrainSpeed(pTrain->m_flSpeed, ((CFuncTrackTrain*)pTrain)->GetMaxSpeed());
			m_iTrain |= TRAIN_ACTIVE|TRAIN_NEW;
		}
	} 
	else if (m_iTrain & TRAIN_ACTIVE)
	{
		m_iTrain = TRAIN_NEW; // turn off train
	}

	//
	// If we're not on the ground, we're falling. Update our falling velocity.
	//
	if ( !( GetFlags() & FL_ONGROUND ) )
	{
		m_Local.m_flFallVelocity = -GetAbsVelocity().z;
	}

	// StudioFrameAdvance( );//!!!HACKHACK!!! Can't be hit by traceline when not animating?

	// Update weapon's ready status
	UpdateWeaponPosture();

	// Disallow shooting while zooming
	if ( IsX360() )
	{
		if ( IsZooming() )
		{
			if( GetActiveWeapon() && !GetActiveWeapon()->IsWeaponZoomed() )
			{
				// If not zoomed because of the weapon itself, do not attack.
				m_nButtons &= ~(IN_ATTACK|IN_ATTACK2);
			}
		}
	}
	else
	{
		if ( m_nButtons & IN_ZOOM )
		{
			//FIXME: Held weapons like the grenade get sad when this happens
	#ifdef HL2_EPISODIC
			// Episodic allows players to zoom while using a func_tank
			CBaseCombatWeapon* pWep = GetActiveWeapon();
			if ( !m_hUseEntity || ( pWep && pWep->IsWeaponVisible() ) )
	#endif
			m_nButtons &= ~(IN_ATTACK|IN_ATTACK2);
		}
	}
}

// Removes a certain ID from our glow ent list:
void CHL2_Player::RemoveGlowItemFromList( int iEntID )
{
	for (int i = (pPlrGlowEntities.Count() - 1); i >= 0; i--)
	{
		CBaseEntity *pEntity = UTIL_EntityByIndex(pPlrGlowEntities[i]);
		if (pEntity)
		{
			if (pEntity->entindex() == iEntID)
			{
				pEntity->RemoveGlowEffect();
				pPlrGlowEntities.Remove(i);
				break;
			}
		}
	}
}

// Clear Up Glow Item List (memory)
void CHL2_Player::ReleaseGlowItemList( bool WantsToEnable )
{
	m_bIsInCamView = false;
	m_bCanSearchForEnts = false;

	// De-glow old items before "resetting"...
	for (int i = (pPlrGlowEntities.Count() - 1); i >= 0; i--)
	{
		CBaseEntity *pEntity = UTIL_EntityByIndex(pPlrGlowEntities[i]);
		if (pEntity)
			pEntity->RemoveGlowEffect();
	}

	pPlrGlowEntities.Purge();
	m_bCanSearchForEnts = WantsToEnable;
}

void CHL2_Player::PostThink( void )
{
	BaseClass::PostThink();

	// TFO Warn
	// Radius Dynamically Change
	int GlowRad = 150;

	// Watch over our entities:
	if ( m_bCanSearchForEnts )
	{
		for (int i = (pPlrGlowEntities.Count() - 1); i >= 0; i--)
		{
			CBaseEntity *pEntity = UTIL_EntityByIndex(pPlrGlowEntities[i]);
			if (pEntity)
			{
				bool bCanGlow = pEntity->CanGlow() && !pEntity->GetOwnerEntity() && (pEntity->IsItem() || pEntity->IsWeapon());

				if (IsAlive() && bCanGlow && (pEntity->GetAbsOrigin().DistTo(this->GetAbsOrigin()) <= GlowRad) /*&& pEntity->FVisible(this, MASK_SOLID)*/)
					pEntity->AddGlowEffect();
				else
				{
					pEntity->RemoveGlowEffect();
					pPlrGlowEntities.Remove(i);
				}
			}
			else
				pPlrGlowEntities.Remove(i);
		}
	}

	if (IsAlive())
	{
		// We search for entities within the radius:
		CBaseEntity *glow_ents = gEntList.FindEntityInSphere(NULL, GetAbsOrigin(), GlowRad);
		while (glow_ents)
		{
			if (m_bCanSearchForEnts && !glow_ents->IsMarkedForDeletion() && glow_ents->CanGlow() && (glow_ents->IsItem() || glow_ents->IsWeapon()) && !glow_ents->GetOwnerEntity())
			{
				if (pPlrGlowEntities.Find(glow_ents->entindex()) == -1)
					pPlrGlowEntities.AddToTail(glow_ents->entindex());
			}

			glow_ents = gEntList.FindEntityInSphere(glow_ents, GetAbsOrigin(), GlowRad);
		}
	}
}

#define HL2PLAYER_RELOADGAME_ATTACK_DELAY 1.0f

void CHL2_Player::Activate( void )
{
	BaseClass::Activate();
	InitSprinting();

#ifdef HL2_EPISODIC

	// Delay attacks by 1 second after loading a game.
	if ( GetActiveWeapon() )
	{
		float flRemaining = GetActiveWeapon()->m_flNextPrimaryAttack - gpGlobals->curtime;

		if ( flRemaining < HL2PLAYER_RELOADGAME_ATTACK_DELAY )
		{
			GetActiveWeapon()->m_flNextPrimaryAttack = gpGlobals->curtime + HL2PLAYER_RELOADGAME_ATTACK_DELAY;
		}

		flRemaining = GetActiveWeapon()->m_flNextSecondaryAttack - gpGlobals->curtime;

		if ( flRemaining < HL2PLAYER_RELOADGAME_ATTACK_DELAY )
		{
			GetActiveWeapon()->m_flNextSecondaryAttack = gpGlobals->curtime + HL2PLAYER_RELOADGAME_ATTACK_DELAY;
		}
	}

#endif

	GetPlayerProxy();
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
Class_T  CHL2_Player::Classify ( void )
{
	// If player controlling another entity?  If so, return this class
	if (m_nControlClass != CLASS_NONE)
	{
		return m_nControlClass;
	}
	else
	{
		if(IsInAVehicle())
		{
			IServerVehicle *pVehicle = GetVehicle();
			return pVehicle->ClassifyPassenger( this, CLASS_PLAYER );
		}
		else
		{
			return CLASS_PLAYER;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:  This is a generic function (to be implemented by sub-classes) to
//			 handle specific interactions between different types of characters
//			 (For example the barnacle grabbing an NPC)
// Input  :  Constant for the type of interaction
// Output :	 true  - if sub-class has a response for the interaction
//			 false - if sub-class has no response
//-----------------------------------------------------------------------------
bool CHL2_Player::HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt)
{
	return false;
}

void CHL2_Player::PlayerRunCommand(CUserCmd *ucmd, IMoveHelper *moveHelper)
{
	// Handle FL_FROZEN.
	if ( m_afPhysicsFlags & PFLAG_ONBARNACLE )
	{
		ucmd->forwardmove = 0;
		ucmd->sidemove = 0;
		ucmd->upmove = 0;
		ucmd->buttons &= ~IN_USE;
	}

	// Can't use stuff while dead
	if ( IsDead() )
	{
		ucmd->buttons &= ~IN_USE;
	}

	//Update our movement information
	if ( ( ucmd->forwardmove != 0 ) || ( ucmd->sidemove != 0 ) || ( ucmd->upmove != 0 ) )
	{
		m_flIdleTime -= TICK_INTERVAL * 2.0f;
		
		if ( m_flIdleTime < 0.0f )
		{
			m_flIdleTime = 0.0f;
		}

		m_flMoveTime += TICK_INTERVAL;

		if ( m_flMoveTime > 4.0f )
		{
			m_flMoveTime = 4.0f;
		}
	}
	else
	{
		m_flIdleTime += TICK_INTERVAL;
		
		if ( m_flIdleTime > 4.0f )
		{
			m_flIdleTime = 4.0f;
		}

		m_flMoveTime -= TICK_INTERVAL * 2.0f;
		
		if ( m_flMoveTime < 0.0f )
		{
			m_flMoveTime = 0.0f;
		}
	}

	//Msg("Player time: [ACTIVE: %f]\t[IDLE: %f]\n", m_flMoveTime, m_flIdleTime );

	BaseClass::PlayerRunCommand( ucmd, moveHelper );
}

//-----------------------------------------------------------------------------
// Purpose: Sets HL2 specific defaults.
//-----------------------------------------------------------------------------
void CHL2_Player::Spawn(void)
{
#ifndef HL2MP
#ifndef PORTAL
	SetModel( PLAYER_MODEL );
#endif
#endif

	BaseClass::Spawn();

	//
	// Our player movement speed is set once here. This will override the cl_xxxx
	// cvars unless they are set to be lower than this.
	//
	//m_flMaxspeed = 320;

	SuitPower_SetCharge( 100 );

	m_Local.m_iHideHUD |= HIDEHUD_CHAT;

	m_pPlayerAISquad = g_AI_SquadManager.FindCreateSquad(AllocPooledString(PLAYER_SQUADNAME));

	InitSprinting();

	GetPlayerProxy();

	m_flCheckForItems = 0.0f;

	GiveNamedItem("weapon_stiel", true); // Give the actual wep without ammo.
}

// TFO Animstate
float CHL2_Player::TFOGetYaw()
{
	float flYaw;

	Vector est_velocity;
	est_velocity = GetAbsVelocity();

	float flLength = est_velocity.Length2D();
	if ( flLength > MOVING_MINIMUM_SPEED )
	{
		m_TFO_GaitYaw = atan2( est_velocity[1], est_velocity[0] );
		m_TFO_GaitYaw = RAD2DEG( m_TFO_GaitYaw);
		m_TFO_GaitYaw = AngleNormalize( m_TFO_GaitYaw );
	}

	float ang = pl.v_angle.y;
	if ( ang > 180.0f )
	{
		ang -= 360.0f;
	}
	else if ( ang < -180.0f )
	{
		ang += 360.0f;
	}

	// calc side to side turning
	flYaw = ang - m_TFO_GaitYaw;
	// Invert for mapping into 8way blend
	flYaw = -flYaw;
	flYaw = flYaw - (int)(flYaw / 360) * 360;

	if (flYaw < -180)
	{
		flYaw = flYaw + 360;
	}
	else if (flYaw > 180)
	{
		flYaw = flYaw - 360;
	}

	return flYaw;
}

void CHL2_Player::SetAnimation( PLAYER_ANIM playerAnim )
{
	CBaseCombatWeapon *pWeapon = GetActiveWeapon();
	bool bHasWeapon = (pWeapon && !IsZoomin);

	m_poseMove_Yaw = LookupPoseParameter( "move_yaw"  ); ; 
	m_poseAim_Pitch = LookupPoseParameter( "aim_pitch" ) ;
	m_poseHead_Pitch = LookupPoseParameter( "head_pitch" ) ;

	int animDesired;
 
	float speed;
 
	speed = GetAbsVelocity().Length2D();
 
	if ( GetFlags() & ( FL_FROZEN | FL_ATCONTROLS ) )
	{
		speed = 0;
		playerAnim = PLAYER_IDLE;
	}
 
	Activity idealActivity = ACT_HL2MP_RUN;
 
	if ( playerAnim == PLAYER_JUMP )
		idealActivity = bHasWeapon ? ACT_HL2MP_JUMP : ACT_JUMP;
	else if ( playerAnim == PLAYER_DIE )
	{
		if ( m_lifeState == LIFE_ALIVE )
		{
			return;
		}
	}
	else if ( playerAnim == PLAYER_ATTACK1 )
	{
		if ( GetActivity( ) == ACT_HOVER ||
			GetActivity( ) == ACT_SWIM ||
			GetActivity( ) == ACT_HOP ||
			GetActivity( ) == ACT_LEAP ||
			GetActivity( ) == ACT_DIESIMPLE )
		{
			idealActivity = GetActivity();
		}
		else
		{
			idealActivity = ACT_HL2MP_GESTURE_RANGE_ATTACK;
		}
	}
	else if ( playerAnim == PLAYER_RELOAD )
	{
		idealActivity = ACT_HL2MP_GESTURE_RELOAD;
	}
	else if (playerAnim == PLAYER_BASH) // Bash
	{
		idealActivity = ACT_MELEE_ATTACK1;
	}
	else
	{
		if ( !( GetFlags() & FL_ONGROUND ) && GetActivity( ) == ACT_JUMP ) // Still jumping
		{
			idealActivity = GetActivity( );
		}
		else
		{
			if ( GetFlags() & FL_DUCKING )
			{
				if ( speed > 0 )
					idealActivity = bHasWeapon ? ACT_HL2MP_WALK_CROUCH : ACT_WALK_CROUCH;
				else
					idealActivity = bHasWeapon ? ACT_HL2MP_IDLE_CROUCH : ACT_HL2MP_IDLE_CROUCH_SLAM; //ACT_IDLE_CROUCH;

				SetPoseParameter(m_poseMove_Yaw,TFOGetYaw());
			}
			else
			{				
				if ( speed > 0 )
				{
					SetPoseParameter(m_poseMove_Yaw,TFOGetYaw());

					if (bHasWeapon)
					{
						if(!m_HL2Local.m_bWeaponLowered)
						{
							if(pWeapon->IsMeleeWeapon() ) // is melee
							{
								idealActivity = ACT_HL2MP_RUN;
								SetPoseParameter(m_poseHead_Pitch,pl.v_angle.x );
							}
							else // any other....
								idealActivity = ACT_HL2MP_RUN;

							SetPoseParameter(m_poseAim_Pitch, pl.v_angle.x );
						}
						else
						{
							SetPoseParameter(m_poseHead_Pitch,pl.v_angle.x );

							if(speed < HL2_WALK_SPEED+25)
								idealActivity = ACT_WALK;
							else
								idealActivity = ACT_RUN;
						}
					}
					else if(speed < HL2_WALK_SPEED+25)
					{
						SetPoseParameter(m_poseHead_Pitch,pl.v_angle.x );
						idealActivity = ACT_WALK;
					}
					else
					{
						SetPoseParameter(m_poseHead_Pitch,pl.v_angle.x );
						idealActivity = ACT_RUN;
					}
				}
				else
				{
					if (bHasWeapon)
					{
						if(m_HL2Local.m_bWeaponLowered)
						{
							idealActivity = pWeapon->IsMeleeWeapon() ? ACT_IDLE_SUITCASE : ACT_IDLE;
							SetPoseParameter(m_poseHead_Pitch,pl.v_angle.x );
						}
						else
						{
							if(pWeapon->IsMeleeWeapon() ) // is melee
							{
								idealActivity = ACT_HL2MP_IDLE;
								SetPoseParameter(m_poseHead_Pitch,pl.v_angle.x );
							}
							else
								idealActivity = ACT_HL2MP_IDLE;

							SetPoseParameter(m_poseAim_Pitch,pl.v_angle.x );
						}
					}
					else
					{
						SetPoseParameter(m_poseHead_Pitch,pl.v_angle.x );
						idealActivity = ACT_IDLE_SUITCASE;
					}
				}
			}
		}
	}

	if ( idealActivity == ACT_HL2MP_GESTURE_RANGE_ATTACK )
	{
		RestartGesture( Weapon_TranslateActivity( idealActivity ) );
 
		// FIXME: this seems a bit wacked
		Weapon_SetActivity( Weapon_TranslateActivity( ACT_RANGE_ATTACK1 ), 0 );		
 
		return;
	}
	else if ( idealActivity == ACT_HL2MP_GESTURE_RELOAD )
	{
		RestartGesture( Weapon_TranslateActivity( idealActivity ) );
		return;
	}
	else if( idealActivity == ACT_MELEE_ATTACK_SWING )
	{
		RestartGesture( ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE);
		return;
	}
	else if (idealActivity == ACT_MELEE_ATTACK1)
	{
		RestartGesture(ACT_MELEE_ATTACK1);
		return;
	}
	else
	{
		SetActivity( idealActivity );

		animDesired = SelectWeightedSequence( Weapon_TranslateActivity ( idealActivity ) );
 
		if (animDesired == -1)
		{
			animDesired = SelectWeightedSequence( idealActivity );
 
			if ( animDesired == -1 )
			{
				animDesired = 0;
			}
		}
 
		// Already using the desired animation?
		if ( GetSequence() == animDesired )
			return;
 
		m_flPlaybackRate = 1.0;
		ResetSequence( animDesired );
		SetCycle( 0 );
		return;
	}

	// Already using the desired animation?
	if ( GetSequence() == animDesired )
		return;
	
	// Reset to first frame of desired animation
	ResetSequence( animDesired );
	SetCycle( 0 );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::InitSprinting( void )
{
	StopSprinting();
}


//-----------------------------------------------------------------------------
// Purpose: Returns whether or not we are allowed to sprint now.
//-----------------------------------------------------------------------------
bool CHL2_Player::CanSprint()
{
	if ( (this->m_nButtons & IN_BACK) )
		return false;

	return ( m_bSprintEnabled &&										// Only if sprint is enabled 
			!IsWalking() &&												// Not if we're walking
			!( m_Local.m_bDucked && !m_Local.m_bDucking ) &&			// Nor if we're ducking
			(GetWaterLevel() != 3) &&									// Certainly not underwater
			(GlobalEntity_GetState("suit_no_sprint") != GLOBAL_ON) );	// Out of the question without the sprint module
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::StartAutoSprint() 
{
	if( IsSprinting() )
	{
		StopSprinting();
	}
	else
	{
		StartSprinting();
		m_bIsAutoSprinting = true;
		m_fAutoSprintMinTime = gpGlobals->curtime + 1.5f;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::StartSprinting( void )
{
	if( m_HL2Local.m_flSuitPower < 10 )
	{
		// Don't sprint unless there's a reasonable
		// amount of suit power.
		
		// debounce the button for sound playing
		if ( m_afButtonPressed & IN_SPEED )
		{
			CPASAttenuationFilter filter( this );
			filter.UsePredictionRules();
			//EmitSound( filter, entindex(), "HL2Player.SprintNoPower" );
		}
		return;
	}

	if( !SuitPower_AddDevice( SuitDeviceSprint ) )
		return;

	CPASAttenuationFilter filter( this );
	filter.UsePredictionRules();
	//EmitSound( filter, entindex(), "HL2Player.SprintStart" );

	SetMaxSpeed( HL2_SPRINT_SPEED );
	m_fIsSprinting = true;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::StopSprinting( void )
{
	if ( m_HL2Local.m_bitsActiveDevices & SuitDeviceSprint.GetDeviceID() )
	{
		SuitPower_RemoveDevice( SuitDeviceSprint );
	}

	SetMaxSpeed( HL2_NORM_SPEED );

	m_fIsSprinting = false;

	if ( sv_stickysprint.GetBool() )
	{
		m_bIsAutoSprinting = false;
		m_fAutoSprintMinTime = 0.0f;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Called to disable and enable sprint due to temporary circumstances:
//			- Carrying a heavy object with the physcannon
//-----------------------------------------------------------------------------
void CHL2_Player::EnableSprint( bool bEnable )
{
	if ( !bEnable && IsSprinting() )
	{
		StopSprinting();
	}

	m_bSprintEnabled = bEnable;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::StartWalking( void )
{
	SetMaxSpeed( HL2_WALK_SPEED );
	m_fIsWalking = true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::StopWalking( void )
{
	SetMaxSpeed( HL2_NORM_SPEED );
	m_fIsWalking = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHL2_Player::CanZoom( CBaseEntity *pRequester )
{
	if ( IsZooming() )
		return false;

	//Check our weapon

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::ToggleZoom(void)
{
	if( IsZooming() )
	{
		StopZooming();
	}
	else
	{
		StartZooming();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Dialogue Zoom : START
//-----------------------------------------------------------------------------
void CHL2_Player::StartZooming( void )
{
	int iFOV = GetDefaultFOV() - 30;

	if ( SetFOV( this, iFOV, 0.6f ) )
	{
		m_HL2Local.m_bZooming = true;
	}

	CBaseCombatWeapon *pWep = GetActiveWeapon();
	if (!pWep)
		return;

	if ( pWep->m_bIsIronsighted )
		pWep->DisableIronsights();

	pWep->StartHolsterSequence(true);
}

//-----------------------------------------------------------------------------
// Purpose: Dialogue Zoom : STOP
//-----------------------------------------------------------------------------
void CHL2_Player::StopZooming( void )
{
	int iFOV = GetZoomOwnerDesiredFOV( m_hZoomOwner );

	if ( SetFOV( this, iFOV, 0.4f ) )
	{
		m_HL2Local.m_bZooming = false;
	}

	CBaseCombatWeapon *pWep = GetActiveWeapon();
	if (!pWep)
		return;

	pWep->Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHL2_Player::IsZooming( void )
{
	if ( m_hZoomOwner != NULL )
		return true;

	return false;
}

class CPhysicsPlayerCallback : public IPhysicsPlayerControllerEvent
{
public:
	int ShouldMoveTo( IPhysicsObject *pObject, const Vector &position )
	{
		CHL2_Player *pPlayer = (CHL2_Player *)pObject->GetGameData();
		if ( pPlayer )
		{
			if ( pPlayer->TouchedPhysics() )
			{
				return 0;
			}
		}
		return 1;
	}
};

static CPhysicsPlayerCallback playerCallback;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHL2_Player::InitVCollision( const Vector &vecAbsOrigin, const Vector &vecAbsVelocity )
{
	BaseClass::InitVCollision( vecAbsOrigin, vecAbsVelocity );

	// Setup the HL2 specific callback.
	IPhysicsPlayerController *pPlayerController = GetPhysicsController();
	if ( pPlayerController )
	{
		pPlayerController->SetEventHandler( &playerCallback );
	}
}

CHL2_Player::~CHL2_Player( void )
{
	for (int i = (pPlrGlowEntities.Count() - 1); i >= 0; i--)
	{
		CBaseEntity *pEntity = UTIL_EntityByIndex(pPlrGlowEntities[i]);
		if (pEntity)
			pEntity->RemoveGlowEffect();
	}

	pPlrGlowEntities.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iImpulse - 
//-----------------------------------------------------------------------------
void CHL2_Player::CheatImpulseCommands( int iImpulse )
{
	switch( iImpulse )
	{

	case 52:
	{
		// Rangefinder
		trace_t tr;
		UTIL_TraceLine( EyePosition(), EyePosition() + EyeDirection3D() * MAX_COORD_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

		if( tr.fraction != 1.0 )
		{
			float flDist = (tr.startpos - tr.endpos).Length();
			float flDist2D = (tr.startpos - tr.endpos).Length2D();
			DevMsg( 1,"\nStartPos: %.4f %.4f %.4f --- EndPos: %.4f %.4f %.4f\n", tr.startpos.x,tr.startpos.y,tr.startpos.z,tr.endpos.x,tr.endpos.y,tr.endpos.z );
			DevMsg( 1,"3D Distance: %.4f units  (%.2f feet) --- 2D Distance: %.4f units  (%.2f feet)\n", flDist, flDist / 12.0, flDist2D, flDist2D / 12.0 );
		}

		break;
	}

	default:
		BaseClass::CheatImpulseCommands(iImpulse);

	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHL2_Player::SetupVisibility( CBaseEntity *pViewEntity, unsigned char *pvs, int pvssize )
{
	BaseClass::SetupVisibility( pViewEntity, pvs, pvssize );

	int area = pViewEntity ? pViewEntity->NetworkProp()->AreaNum() : NetworkProp()->AreaNum();
	PointCameraSetupVisibility( this, area, pvs, pvssize );

	// If the intro script is playing, we want to get it's visibility points
	if ( g_hIntroScript )
	{
		Vector vecOrigin;
		CBaseEntity *pCamera;
		if ( g_hIntroScript->GetIncludedPVSOrigin( &vecOrigin, &pCamera ) )
		{
			// If it's a point camera, turn it on
			CPointCamera *pPointCamera = dynamic_cast< CPointCamera* >(pCamera); 
			if ( pPointCamera )
			{
				pPointCamera->SetActive( true );
			}
			engine->AddOriginToPVS( vecOrigin );
		}
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::SuitPower_Update( void )
{
	if( SuitPower_ShouldRecharge() )
	{
		SuitPower_Charge( SUITPOWER_CHARGE_RATE * gpGlobals->frametime );
	}
	else if( m_HL2Local.m_bitsActiveDevices )
	{
		float flPowerLoad = m_flSuitPowerLoad;

		//Since stickysprint quickly shuts off sprint if it isn't being used, this isn't an issue.
		if ( !sv_stickysprint.GetBool() )
		{
			if( SuitPower_IsDeviceActive(SuitDeviceSprint) )
			{
				if( !fabs(GetAbsVelocity().x) && !fabs(GetAbsVelocity().y) )
				{
					// If player's not moving, don't drain sprint juice.
					flPowerLoad -= SuitDeviceSprint.GetDeviceDrainRate();
				}
			}
		}

		if( !SuitPower_Drain( flPowerLoad * gpGlobals->frametime ) )
		{
			// TURN OFF ALL DEVICES!!
			if( IsSprinting() )
			{
				StopSprinting();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Charge battery fully, turn off all devices.
//-----------------------------------------------------------------------------
void CHL2_Player::SuitPower_Initialize( void )
{
	m_HL2Local.m_bitsActiveDevices = 0x00000000;
	m_HL2Local.m_flSuitPower = 100.0;
	m_flSuitPowerLoad = 0.0;
}

//-----------------------------------------------------------------------------
// Purpose: Interface to drain power from the suit's power supply.
// Input:	Amount of charge to remove (expressed as percentage of full charge)
// Output:	Returns TRUE if successful, FALSE if not enough power available.
//-----------------------------------------------------------------------------
bool CHL2_Player::SuitPower_Drain( float flPower )
{
	// Suitpower cheat on?
	if ( sv_infinite_aux_power.GetBool() )
		return true;

	m_HL2Local.m_flSuitPower -= flPower;

	if( m_HL2Local.m_flSuitPower < 0.0 )
	{
		// Power is depleted!
		// Clamp and fail
		m_HL2Local.m_flSuitPower = 0.0;
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Interface to add power to the suit's power supply
// Input:	Amount of charge to add
//-----------------------------------------------------------------------------
void CHL2_Player::SuitPower_Charge( float flPower )
{
	m_HL2Local.m_flSuitPower += flPower;

	if( m_HL2Local.m_flSuitPower > 100.0 )
	{
		// Full charge, clamp.
		m_HL2Local.m_flSuitPower = 100.0;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CHL2_Player::SuitPower_IsDeviceActive( const CSuitPowerDevice &device )
{
	return (m_HL2Local.m_bitsActiveDevices & device.GetDeviceID()) != 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CHL2_Player::SuitPower_AddDevice( const CSuitPowerDevice &device )
{
	// Make sure this device is NOT active!!
	if( m_HL2Local.m_bitsActiveDevices & device.GetDeviceID() )
		return false;

	m_HL2Local.m_bitsActiveDevices |= device.GetDeviceID();
	m_flSuitPowerLoad += device.GetDeviceDrainRate();
	return true;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CHL2_Player::SuitPower_RemoveDevice( const CSuitPowerDevice &device )
{
	// Make sure this device is active!!
	if( ! (m_HL2Local.m_bitsActiveDevices & device.GetDeviceID()) )
		return false;

	// Take a little bit of suit power when you disable a device. If the device is shutting off
	// because the battery is drained, no harm done, the battery charge cannot go below 0. 
	// This code in combination with the delay before the suit can start recharging are a defense
	// against exploits where the player could rapidly tap sprint and never run out of power.
	SuitPower_Drain( device.GetDeviceDrainRate() * 0.1f );

	m_HL2Local.m_bitsActiveDevices &= ~device.GetDeviceID();
	m_flSuitPowerLoad -= device.GetDeviceDrainRate();

	if( m_HL2Local.m_bitsActiveDevices == 0x00000000 )
	{
		// With this device turned off, we can set this timer which tells us when the
		// suit power system entered a no-load state.
		m_flTimeAllSuitDevicesOff = gpGlobals->curtime;
	}

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define SUITPOWER_BEGIN_RECHARGE_DELAY	0.5f
bool CHL2_Player::SuitPower_ShouldRecharge( void )
{
	// Make sure all devices are off.
	if( m_HL2Local.m_bitsActiveDevices != 0x00000000 )
		return false;

	// Is the system fully charged?
	if( m_HL2Local.m_flSuitPower >= 100.0f )
		return false; 

	// Has the system been in a no-load state for long enough
	// to begin recharging?
	if( gpGlobals->curtime < m_flTimeAllSuitDevicesOff + SUITPOWER_BEGIN_RECHARGE_DELAY )
		return false;

	return true;
}		

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::SetPlayerUnderwater( bool state )
{
	if ( state )
	{
		SuitPower_AddDevice( SuitDeviceBreather );
	}
	else
	{
  		SuitPower_RemoveDevice( SuitDeviceBreather );
	}

	BaseClass::SetPlayerUnderwater( state );
}

//-----------------------------------------------------------------------------
bool CHL2_Player::PassesDamageFilter( const CTakeDamageInfo &info )
{
	CBaseEntity *pAttacker = info.GetAttacker();
	if( pAttacker && pAttacker->MyNPCPointer() && pAttacker->MyNPCPointer()->IsPlayerAlly() )
	{
		return false;
	}

	if( m_hPlayerProxy && !m_hPlayerProxy->PassesDamageFilter( info ) )
	{
		return false;
	}

	return BaseClass::PassesDamageFilter( info );
}

//-----------------------------------------------------------------------------
// Purpose: Prevent the player from taking fall damage for [n] seconds, but
// reset back to taking fall damage after the first impact (so players will be
// hurt if they bounce off what they hit). This is the original behavior.
//-----------------------------------------------------------------------------
void CHL2_Player::InputIgnoreFallDamage( inputdata_t &inputdata )
{
	float timeToIgnore = inputdata.value.Float();

	if ( timeToIgnore <= 0.0 )
		timeToIgnore = TIME_IGNORE_FALL_DAMAGE;

	m_flTimeIgnoreFallDamage = gpGlobals->curtime + timeToIgnore;
	m_bIgnoreFallDamageResetAfterImpact = true;
}

//-----------------------------------------------------------------------------
// Purpose: Absolutely prevent the player from taking fall damage for [n] seconds. 
//-----------------------------------------------------------------------------
void CHL2_Player::InputIgnoreFallDamageWithoutReset( inputdata_t &inputdata )
{
	float timeToIgnore = inputdata.value.Float();

	if ( timeToIgnore <= 0.0 )
		timeToIgnore = TIME_IGNORE_FALL_DAMAGE;

	m_flTimeIgnoreFallDamage = gpGlobals->curtime + timeToIgnore;
	m_bIgnoreFallDamageResetAfterImpact = false;
}

//-----------------------------------------------------------------------------
// Purpose: Notification of a player's npc ally in the players squad being killed
//-----------------------------------------------------------------------------
void CHL2_Player::OnSquadMemberKilled( inputdata_t &data )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHL2_Player::NotifyFriendsOfDamage( CBaseEntity *pAttackerEntity )
{
	CAI_BaseNPC *pAttacker = pAttackerEntity->MyNPCPointer();
	if ( pAttacker )
	{
		const Vector &origin = GetAbsOrigin();
		for ( int i = 0; i < g_AI_Manager.NumAIs(); i++ )
		{
			const float NEAR_Z = 12*12;
			const float NEAR_XY_SQ = Square( 50*12 );
			CAI_BaseNPC *pNpc = g_AI_Manager.AccessAIs()[i];
			if ( pNpc->IsPlayerAlly() )
			{
				const Vector &originNpc = pNpc->GetAbsOrigin();
				if ( fabsf( originNpc.z - origin.z ) < NEAR_Z )
				{
					if ( (originNpc.AsVector2D() - origin.AsVector2D()).LengthSqr() < NEAR_XY_SQ )
					{
						pNpc->OnFriendDamaged( this, pAttacker );
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ConVar test_massive_dmg("test_massive_dmg", "30" );
ConVar test_massive_dmg_clip("test_massive_dmg_clip", "0.5" );
int	CHL2_Player::OnTakeDamage( const CTakeDamageInfo &info )
{
	if ( GlobalEntity_GetState( "gordon_invulnerable" ) == GLOBAL_ON )
		return 0;

	// ignore fall damage if instructed to do so by input
	if ( ( info.GetDamageType() & DMG_FALL ) && m_flTimeIgnoreFallDamage > gpGlobals->curtime )
	{
		// usually, we will reset the input flag after the first impact. However there is another input that
		// prevents this behavior.
		if ( m_bIgnoreFallDamageResetAfterImpact )
		{
			m_flTimeIgnoreFallDamage = 0;
		}
		return 0;
	}

	if( info.GetDamageType() & DMG_BLAST_SURFACE )
	{
		if( GetWaterLevel() > 2 )
		{
			// Don't take blast damage from anything above the surface.
			if( info.GetInflictor()->GetWaterLevel() == 0 )
			{
				return 0;
			}
		}
	}

	if ( info.GetDamage() > 0.0f )
	{
		m_flLastDamageTime = gpGlobals->curtime;

		if ( info.GetAttacker() )
			NotifyFriendsOfDamage( info.GetAttacker() );
	}
	
	// Modify the amount of damage the player takes, based on skill.
	CTakeDamageInfo playerDamage = info;

	if ( GetVehicleEntity() != NULL && GlobalEntity_GetState("gordon_protect_driver") == GLOBAL_ON )
	{
		if( playerDamage.GetDamage() > test_massive_dmg.GetFloat() && playerDamage.GetInflictor() == GetVehicleEntity() && (playerDamage.GetDamageType() & DMG_CRUSH) )
		{
			playerDamage.ScaleDamage( test_massive_dmg_clip.GetFloat() / playerDamage.GetDamage() );
		}
	}

	return BaseClass::OnTakeDamage( playerDamage );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
//-----------------------------------------------------------------------------
int CHL2_Player::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	// Drown
	if( info.GetDamageType() & DMG_DROWN )
	{
		if( m_idrowndmg == m_idrownrestored )
		{
			EmitSound( "Player.DrownStart" );
		}
		else
		{
			EmitSound( "Player.DrownContinue" );
		}
	}

	// Burnt
	if ( info.GetDamageType() & DMG_BURN )
	{
		EmitSound( "HL2Player.BurnPain" );
	}

	if( (info.GetDamageType() & DMG_SLASH) && hl2_episodic.GetBool() )
	{
		if( m_afPhysicsFlags & PFLAG_USING )
		{
			// Stop the player using a rotating button for a short time if hit by a creature's melee attack.
			// This is for the antlion burrow-corking training in EP1 (sjb).
			SuspendUse( 0.5f );
		}
	}


	// Call the base class implementation
	return BaseClass::OnTakeDamage_Alive( info );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::OnDamagedByExplosion( const CTakeDamageInfo &info )
{
	if ( info.GetInflictor() && info.GetInflictor()->ClassMatches( "mortarshell" ) )
	{
		// No ear ringing for mortar
		UTIL_ScreenShake( info.GetInflictor()->GetAbsOrigin(), 4.0, 1.0, 0.5, 1000, SHAKE_START, false );
		return;
	}
	BaseClass::OnDamagedByExplosion( info );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CHL2_Player::ShouldShootMissTarget( CBaseCombatCharacter *pAttacker )
{
	if( gpGlobals->curtime > m_flTargetFindTime )
	{
		// Put this off into the future again.
		m_flTargetFindTime = gpGlobals->curtime + random->RandomFloat( 3, 5 );
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::Event_KilledOther( CBaseEntity *pVictim, const CTakeDamageInfo &info )
{
	BaseClass::Event_KilledOther( pVictim, info );

#ifdef HL2_EPISODIC

	CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();

	for ( int i = 0; i < g_AI_Manager.NumAIs(); i++ )
	{
		if ( ppAIs[i] && ppAIs[i]->IRelationType(this) == D_LI )
		{
			ppAIs[i]->OnPlayerKilledOther( pVictim, info );
		}
	}

#endif
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::Event_Killed( const CTakeDamageInfo &info )
{
	BaseClass::Event_Killed( info );

	FirePlayerProxyOutput( "PlayerDied", variant_t(), this, this );
	NotifyScriptsOfDeath();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::NotifyScriptsOfDeath( void )
{
	CBaseEntity *pEnt =	gEntList.FindEntityByClassname( NULL, "scripted_sequence" );

	while( pEnt )
	{
		variant_t emptyVariant;
		pEnt->AcceptInput( "ScriptPlayerDeath", NULL, NULL, emptyVariant, 0 );

		pEnt = gEntList.FindEntityByClassname( pEnt, "scripted_sequence" );
	}

	pEnt =	gEntList.FindEntityByClassname( NULL, "logic_choreographed_scene" );

	while( pEnt )
	{
		variant_t emptyVariant;
		pEnt->AcceptInput( "ScriptPlayerDeath", NULL, NULL, emptyVariant, 0 );

		pEnt = gEntList.FindEntityByClassname( pEnt, "logic_choreographed_scene" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iCount - 
//			iAmmoIndex - 
//			bSuppressSound - 
// Output : int
//-----------------------------------------------------------------------------
int CHL2_Player::GiveAmmo( int nCount, int nAmmoIndex, bool bSuppressSound)
{
	// Don't try to give the player invalid ammo indices.
	if (nAmmoIndex < 0)
		return 0;

	int nAdd = BaseClass::GiveAmmo(nCount, nAmmoIndex, bSuppressSound);

	if ( nCount > 0 && nAdd == 0 )
	{
		// we've been denied the pickup, display a hud icon to show that
		CSingleUserRecipientFilter user( this );
		user.MakeReliable();
		UserMessageBegin( user, "AmmoDenied" );
			WRITE_SHORT( nAmmoIndex );
		MessageEnd();
	}

	return nAdd;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CHL2_Player::Weapon_CanUse( CBaseCombatWeapon *pWeapon )
{
	return BaseClass::Weapon_CanUse( pWeapon );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pWeapon - 
//-----------------------------------------------------------------------------
void CHL2_Player::Weapon_Equip( CBaseCombatWeapon *pWeapon )
{
	if ( GetActiveWeapon() && ( Weapon_GetSlot( pWeapon->GetSlot() ) != NULL ) )
	{
		Weapon_DropSlot( pWeapon->GetSlot() );
	}

	if( GetActiveWeapon() == NULL )
	{
		m_HL2Local.m_bWeaponLowered = false;
	}

	BaseClass::Weapon_Equip( pWeapon );
}

//-----------------------------------------------------------------------------
// Purpose: Player reacts to bumping a weapon. 
// Input  : pWeapon - the weapon that the player bumped into.
// Output : Returns true if player picked up the weapon
//-----------------------------------------------------------------------------
bool CHL2_Player::BumpWeapon(CBaseCombatWeapon *pWeapon)
{
	CBaseCombatCharacter *pOwner = pWeapon->GetOwner();

	// Can I have this weapon type?
	if (pOwner || !Weapon_CanUse(pWeapon) || !g_pGameRules->CanHavePlayerItem(this, pWeapon) || IsWeaponBusy())
		return false;

	// Don't let the player fetch weapons through walls (use MASK_SOLID so that you can't pickup through windows)
	if (!pWeapon->FVisible(this, MASK_SOLID) && !(GetFlags() & FL_NOTARGET))
		return false;

	if (!m_bCanPickupRewards && FClassnameIs(pWeapon, "weapon_fg42"))
		return false;

	// Is the new weapon the same as our active weapon ?
	if (GetActiveWeapon() && FClassnameIs(GetActiveWeapon(), pWeapon->GetClassname()))
		return false;

	// We can't pickup a weapon which we already carry:
	CBaseCombatWeapon *pHasWeapon = Weapon_GetSlot(pWeapon->GetSlot());
	if (pHasWeapon && FClassnameIs(pHasWeapon, pWeapon->GetClassname()))
		return false;

	// Set weapon pickup achievements here:
	if (FClassnameIs(pWeapon, "weapon_fg42"))
		CAchievementManager::SendAchievement("ACH_WEAPON_SPECIAL_2");
	else if (FClassnameIs(pWeapon, "weapon_svt40"))
		CAchievementManager::SendAchievement("ACH_WEAPON_SPECIAL_3");
	else if (FClassnameIs(pWeapon, "weapon_panzer"))
		CAchievementManager::SendAchievement("ACH_WEAPON_SPECIAL_1");

	pWeapon->CheckRespawn();
	pWeapon->AddSolidFlags(FSOLID_NOT_SOLID);
	pWeapon->AddEffects(EF_NODRAW);
	Weapon_Equip(pWeapon);

	if (IsInAVehicle())
		pWeapon->SetWeaponVisible(false);
	else
	{
		CBaseCombatWeapon *pActiveWep = GetActiveWeapon();
		if (pActiveWep)
		{
			bool bCantDeploy = (pActiveWep->m_bIsIronsighted || !m_bCanDoMeleeAttack || !pActiveWep->CanHolster() || pActiveWep->m_bWantsHolster || !pWeapon->HasAnyAmmo());
			bool bNewWepIsOfSameSlot = (pActiveWep->GetSlot() == pWeapon->GetSlot());

			if (bNewWepIsOfSameSlot && pActiveWep->m_bInReload)
				pActiveWep->m_bInReload = false;

			if (!bCantDeploy)
				Weapon_Switch(pWeapon, bNewWepIsOfSameSlot);
		}
		else
			Weapon_Switch(pWeapon, true);
	}

	EmitSound("HL2Player.PickupWeapon");

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *cmd - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHL2_Player::ClientCommand( const CCommand &args )
{
	// Drop Code
	if (!Q_stricmp(args[0], "DropPrimary"))
	{
		if (!IsAlive())
			return true;

		CBaseCombatWeapon* pWeapon = GetActiveWeapon();
		if (!pWeapon || pWeapon->IsHands() || pWeapon->IsGrenade())
			return true;

		// Can't drop while:
		if (pWeapon->m_bInReload || m_bIsRunning || !GetGroundEntity() || m_bShouldSwim || m_bIsInCamView || !pWeapon->CanHolster() || pWeapon->m_bWantsHolster)
			return true;

		pWeapon->DisableIronsights();

		if (pWeapon->IsLightSource())
			pWeapon->ClearParticles();

		Weapon_DropSlot(pWeapon->GetSlot());
		return true;
	}

	if (!Q_stricmp(args[0], "DropWeaponAtSlot"))
	{
		if (args.ArgC() != 2)
			return true;

		int iSlot = atoi(args[1]);
		if (!iSlot)
			return true;

		if (!IsAlive())
			return true;

		CBaseCombatWeapon* pWeapon = Weapon_GetSlot(iSlot);
		if (pWeapon != NULL && (iSlot < MAX_WEAPON_SLOTS))
			Weapon_DropSlot(iSlot);
	}

	if ( !Q_stricmp( args[0], "emit" ) )
	{
		CSingleUserRecipientFilter filter( this );
		if ( args.ArgC() > 1 )
		{
			EmitSound( filter, entindex(), args[ 1 ] );
		}
		else
		{
			EmitSound( filter, entindex(), "Test.Sound" );
		}
		return true;
	}

	return BaseClass::ClientCommand( args );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : void CBasePlayer::PlayerUse
//-----------------------------------------------------------------------------
void CHL2_Player::PlayerUse ( void )
{
	// Was use pressed or released?
	if ( ! ((m_nButtons | m_afButtonPressed | m_afButtonReleased) & IN_USE) )
		return;

	if ( m_afButtonPressed & IN_USE )
	{
		// Currently using a latched entity?
		if ( ClearUseEntity() )
		{
			return;
		}
		else
		{
			if ( m_afPhysicsFlags & PFLAG_DIROVERRIDE )
			{
				m_afPhysicsFlags &= ~PFLAG_DIROVERRIDE;
				m_iTrain = TRAIN_NEW|TRAIN_OFF;
				return;
			}
			else
			{	// Start controlling the train!
				CBaseEntity *pTrain = GetGroundEntity();
				if ( pTrain && !(m_nButtons & IN_JUMP) && (GetFlags() & FL_ONGROUND) && (pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) && pTrain->OnControls(this) )
				{
					m_afPhysicsFlags |= PFLAG_DIROVERRIDE;
					m_iTrain = TrainSpeed(pTrain->m_flSpeed, ((CFuncTrackTrain*)pTrain)->GetMaxSpeed());
					m_iTrain |= TRAIN_NEW;
					return;
				}
			}
		}

		// Tracker 3926:  We can't +USE something if we're climbing a ladder
		if ( GetMoveType() == MOVETYPE_LADDER )
		{
			return;
		}
	}

	if( m_flTimeUseSuspended > gpGlobals->curtime )
	{
		// Something has temporarily stopped us being able to USE things.
		// Obviously, this should be used very carefully.(sjb)
		return;
	}

	CBaseEntity *pUseEntity = FindUseEntity();

	bool usedSomething = false;

	// Found an object
	if ( pUseEntity )
	{
		//!!!UNDONE: traceline here to prevent +USEing buttons through walls			
		int caps = pUseEntity->ObjectCaps();
		variant_t emptyVariant;

		if ( ( (m_nButtons & IN_USE) && (caps & FCAP_CONTINUOUS_USE) ) ||
			 ( (m_afButtonPressed & IN_USE) && (caps & (FCAP_IMPULSE_USE|FCAP_ONOFF_USE)) ) )
		{
			if ( caps & FCAP_CONTINUOUS_USE )
				m_afPhysicsFlags |= PFLAG_USING;

			pUseEntity->AcceptInput( "Use", this, this, emptyVariant, USE_TOGGLE );

			usedSomething = true;
		}
		// UNDONE: Send different USE codes for ON/OFF.  Cache last ONOFF_USE object to send 'off' if you turn away
		else if ( (m_afButtonReleased & IN_USE) && (pUseEntity->ObjectCaps() & FCAP_ONOFF_USE) )	// BUGBUG This is an "off" use
		{
			pUseEntity->AcceptInput( "Use", this, this, emptyVariant, USE_TOGGLE );

			usedSomething = true;
		}
	}

	// Debounce the use key
	if ( usedSomething && pUseEntity )
	{
		m_Local.m_nOldButtons |= IN_USE;
		m_afButtonPressed &= ~IN_USE;
	}
}

ConVar	sv_show_crosshair_target( "sv_show_crosshair_target", "0" );

//-----------------------------------------------------------------------------
// Purpose: Updates the posture of the weapon from lowered to ready
//-----------------------------------------------------------------------------
void CHL2_Player::UpdateWeaponPosture( void )
{
	CBaseCombatWeapon *pWeapon = dynamic_cast<CBaseCombatWeapon *>(GetActiveWeapon());

	if ( pWeapon && m_LowerWeaponTimer.Expired() && pWeapon->CanLower() )
	{
		m_LowerWeaponTimer.Set( .3 );
		VPROF( "CHL2_Player::UpdateWeaponPosture-CheckLower" );
		Vector vecAim = BaseClass::GetAutoaimVector();

		const float CHECK_FRIENDLY_RANGE = 50 * 12;
		trace_t	tr;
		UTIL_TraceLine( EyePosition(), EyePosition() + vecAim * CHECK_FRIENDLY_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

		CBaseEntity *aimTarget = tr.m_pEnt;

		//If we're over something
		if (  aimTarget && !tr.DidHitWorld() )
		{
			if ( !aimTarget->IsNPC() || aimTarget->MyNPCPointer()->GetState() != NPC_STATE_COMBAT )
			{
				Disposition_t dis = IRelationType( aimTarget );

				//Debug info for seeing what an object "cons" as
				if ( sv_show_crosshair_target.GetBool() )
				{
					int text_offset = BaseClass::DrawDebugTextOverlays();

					char tempstr[255];	

					switch ( dis )
					{
					case D_LI:
						Q_snprintf( tempstr, sizeof(tempstr), "Disposition: Like" );
						break;

					case D_HT:
						Q_snprintf( tempstr, sizeof(tempstr), "Disposition: Hate" );
						break;

					case D_FR:
						Q_snprintf( tempstr, sizeof(tempstr), "Disposition: Fear" );
						break;

					case D_NU:
						Q_snprintf( tempstr, sizeof(tempstr), "Disposition: Neutral" );
						break;

					default:
					case D_ER:
						Q_snprintf( tempstr, sizeof(tempstr), "Disposition: !!!ERROR!!!" );
						break;
					}

					//Draw the text
					NDebugOverlay::EntityText( aimTarget->entindex(), text_offset, tempstr, 0 );
				}

				//See if we hates it
				if ( dis == D_LI  )
				{
					//We're over a friendly, drop our weapon
					if ( Weapon_Lower() == false )
					{
						//FIXME: We couldn't lower our weapon!
					}

					return;
				}
			}
		}

		if ( Weapon_Ready() == false )
		{
			//FIXME: We couldn't raise our weapon!
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Lowers the weapon posture (for hovering over friendlies)
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHL2_Player::Weapon_Lower( void )
{
	VPROF( "CHL2_Player::Weapon_Lower" );
	// Already lowered?
	if ( m_HL2Local.m_bWeaponLowered )
		return true;

	m_HL2Local.m_bWeaponLowered = true;

	CBaseCombatWeapon *pWeapon = dynamic_cast<CBaseCombatWeapon *>(GetActiveWeapon());

	if ( pWeapon == NULL )
		return false;

	pWeapon->DisableIronsights();

	return pWeapon->Lower();
}

//-----------------------------------------------------------------------------
// Purpose: Returns the weapon posture to normal
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHL2_Player::Weapon_Ready( void )
{
	VPROF( "CHL2_Player::Weapon_Ready" );

	// Already ready?
	if ( m_HL2Local.m_bWeaponLowered == false )
		return true;

	m_HL2Local.m_bWeaponLowered = false;

	CBaseCombatWeapon *pWeapon = dynamic_cast<CBaseCombatWeapon *>(GetActiveWeapon());

	if ( pWeapon == NULL )
		return false;

	return pWeapon->Ready();
}

//-----------------------------------------------------------------------------
// Purpose: Returns whether or not we can switch to the given weapon.
// Input  : pWeapon - 
//-----------------------------------------------------------------------------
bool CHL2_Player::Weapon_CanSwitchTo( CBaseCombatWeapon *pWeapon )
{
	CBasePlayer *pPlayer = (CBasePlayer *)this;
#if !defined( CLIENT_DLL )
	IServerVehicle *pVehicle = pPlayer->GetVehicle();
#else
	IClientVehicle *pVehicle = pPlayer->GetVehicle();
#endif
	if (pVehicle && !pPlayer->UsingStandardWeaponsInVehicle())
		return false;

	if (GetActiveWeapon())
	{
		if (!GetActiveWeapon()->CanHolster())
			return false;
	}

	return true;
}

void CHL2_Player::PickupObject( CBaseEntity *pObject, bool bLimitMassAndSize )
{
	// can't pick up what you're standing on
	if (GetGroundEntity() == pObject || (pObject->IsItem() || pObject->IsWeapon()) || IsWeaponBusy())
		return;
	
	if ( bLimitMassAndSize == true )
	{
		if ( CBasePlayer::CanPickupObject( pObject, 35, 128 ) == false )
			 return;
	}

	// Can't be picked up if NPCs are on me
	if ( pObject->HasNPCsOnIt() )
		return;

	PlayerPickupObject( this, pObject );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : CBaseEntity
//-----------------------------------------------------------------------------
bool CHL2_Player::IsHoldingEntity( CBaseEntity *pEnt )
{
	return PlayerPickupControllerIsHoldingEntity(m_hUseEntity, pEnt);
}

float CHL2_Player::GetHeldObjectMass( IPhysicsObject *pHeldObject )
{
	return PlayerPickupGetHeldObjectMass(m_hUseEntity, pHeldObject);
}

//-----------------------------------------------------------------------------
// Purpose: Force the player to drop any physics objects he's carrying
//-----------------------------------------------------------------------------
void CHL2_Player::ForceDropOfCarriedPhysObjects( CBaseEntity *pOnlyIfHoldingThis )
{
	if ( PhysIsInCallback() )
	{
		variant_t value;
		g_EventQueue.AddEvent( this, "ForceDropPhysObjects", value, 0.01f, pOnlyIfHoldingThis, this );
		return;
	}

	// Drop any objects being handheld.
	ClearUseEntity();
}

void CHL2_Player::InputForceDropPhysObjects( inputdata_t &data )
{
	ForceDropOfCarriedPhysObjects( data.pActivator );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHL2_Player::UpdateClientData( void )
{
	if (m_DmgTake || m_DmgSave || m_bitsHUDDamage != m_bitsDamageType)
	{
		// Comes from inside me if not set
		Vector damageOrigin = GetLocalOrigin();
		// send "damage" message
		// causes screen to flash, and pain compass to show direction of damage
		damageOrigin = m_DmgOrigin;

		// only send down damage type that have hud art
		int iShowHudDamage = g_pGameRules->Damage_GetShowOnHud();
		int visibleDamageBits = m_bitsDamageType & iShowHudDamage;

		m_DmgTake = clamp( m_DmgTake, 0, 255 );
		m_DmgSave = clamp( m_DmgSave, 0, 255 );

		// If we're poisoned, but it wasn't this frame, don't send the indicator
		// Without this check, any damage that occured to the player while they were
		// recovering from a poison bite would register as poisonous as well and flash
		// the whole screen! -- jdw
		if ( visibleDamageBits & DMG_POISON )
		{
			float flLastPoisonedDelta = gpGlobals->curtime - m_tbdPrev;
			if ( flLastPoisonedDelta > 0.1f )
			{
				visibleDamageBits &= ~DMG_POISON;
			}
		}

		CSingleUserRecipientFilter user( this );
		user.MakeReliable();
		UserMessageBegin( user, "Damage" );
			WRITE_BYTE( m_DmgSave );
			WRITE_BYTE( m_DmgTake );
			WRITE_LONG( visibleDamageBits );
			WRITE_FLOAT( damageOrigin.x );	//BUG: Should be fixed point (to hud) not floats
			WRITE_FLOAT( damageOrigin.y );	//BUG: However, the HUD does _not_ implement bitfield messages (yet)
			WRITE_FLOAT( damageOrigin.z );	//BUG: We use WRITE_VEC3COORD for everything else
		MessageEnd();
	
		m_DmgTake = 0;
		m_DmgSave = 0;
		m_bitsHUDDamage = m_bitsDamageType;
		
		// Clear off non-time-based damage indicators
		int iTimeBasedDamage = g_pGameRules->Damage_GetTimeBased();
		m_bitsDamageType &= iTimeBasedDamage;
	}

	BaseClass::UpdateClientData();
}

//---------------------------------------------------------
//---------------------------------------------------------
void CHL2_Player::OnRestore()
{
	BaseClass::OnRestore();
	m_pPlayerAISquad = g_AI_SquadManager.FindCreateSquad(AllocPooledString(PLAYER_SQUADNAME));
}

//---------------------------------------------------------
//---------------------------------------------------------
Vector CHL2_Player::EyeDirection2D( void )
{
	Vector vecReturn = EyeDirection3D();
	vecReturn.z = 0;
	vecReturn.AsVector2D().NormalizeInPlace();

	return vecReturn;
}

//---------------------------------------------------------
//---------------------------------------------------------
Vector CHL2_Player::EyeDirection3D( void )
{
	Vector vecForward;

	// Return the vehicle angles if we request them
	if ( GetVehicle() != NULL )
	{
		CacheVehicleView();
		EyeVectors( &vecForward );
		return vecForward;
	}
	
	AngleVectors( EyeAngles(), &vecForward );
	return vecForward;
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CHL2_Player::Weapon_Switch(CBaseCombatWeapon* pWeapon, bool bWantDraw)
{
	MDLCACHE_CRITICAL_SECTION();

	if (pWeapon && pWeapon->IsGrenade())
	{
		if (!pWeapon->HasAnyAmmo() && !GetAmmoCount(pWeapon->m_iPrimaryAmmoType))
			return false;
	}

	bool bRet = BaseClass::Weapon_Switch(pWeapon, bWantDraw);
	if (bRet)
	{
		// Recalculate proficiency!
		SetCurrentWeaponProficiency(CalcWeaponProficiency(pWeapon));

		// Can we bash or not?
		CBaseViewModel *pvm = GetViewModel();
		m_bCanBash = (pvm ? (pvm->LookupActivity("ACT_VM_BASH") != ACT_INVALID) : false);
		ItemTakeAnim = false;
		m_bCanDoMeleeAttack = true;
	}

	return bRet;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
WeaponProficiency_t CHL2_Player::CalcWeaponProficiency( CBaseCombatWeapon *pWeapon )
{
	WeaponProficiency_t proficiency;

	proficiency = WEAPON_PROFICIENCY_PERFECT;

	if( weapon_showproficiency.GetBool() != 0 )
	{
		Msg("Player switched to %s, proficiency is %s\n", pWeapon->GetClassname(), GetWeaponProficiencyName( proficiency ) );
	}

	return proficiency;
}

//-----------------------------------------------------------------------------
// Purpose: override how single player rays hit the player
//-----------------------------------------------------------------------------

bool LineCircleIntersection(
	const Vector2D &center,
	const float radius,
	const Vector2D &vLinePt,
	const Vector2D &vLineDir,
	float *fIntersection1,
	float *fIntersection2)
{
	// Line = P + Vt
	// Sphere = r (assume we've translated to origin)
	// (P + Vt)^2 = r^2
	// VVt^2 + 2PVt + (PP - r^2)
	// Solve as quadratic:  (-b  +/-  sqrt(b^2 - 4ac)) / 2a
	// If (b^2 - 4ac) is < 0 there is no solution.
	// If (b^2 - 4ac) is = 0 there is one solution (a case this function doesn't support).
	// If (b^2 - 4ac) is > 0 there are two solutions.
	Vector2D P;
	float a, b, c, sqr, insideSqr;


	// Translate circle to origin.
	P[0] = vLinePt[0] - center[0];
	P[1] = vLinePt[1] - center[1];
	
	a = vLineDir.Dot(vLineDir);
	b = 2.0f * P.Dot(vLineDir);
	c = P.Dot(P) - (radius * radius);

	insideSqr = b*b - 4*a*c;
	if(insideSqr <= 0.000001f)
		return false;

	// Ok, two solutions.
	sqr = (float)FastSqrt(insideSqr);

	float denom = 1.0 / (2.0f * a);
	
	*fIntersection1 = (-b - sqr) * denom;
	*fIntersection2 = (-b + sqr) * denom;

	return true;
}

static void Collision_ClearTrace( const Vector &vecRayStart, const Vector &vecRayDelta, CBaseTrace *pTrace )
{
	pTrace->startpos = vecRayStart;
	pTrace->endpos = vecRayStart;
	pTrace->endpos += vecRayDelta;
	pTrace->startsolid = false;
	pTrace->allsolid = false;
	pTrace->fraction = 1.0f;
	pTrace->contents = 0;
}


bool IntersectRayWithAACylinder( const Ray_t &ray, 
	const Vector &center, float radius, float height, CBaseTrace *pTrace )
{
	Assert( ray.m_IsRay );
	Collision_ClearTrace( ray.m_Start, ray.m_Delta, pTrace );

	// First intersect the ray with the top + bottom planes
	float halfHeight = height * 0.5;

	// Handle parallel case
	Vector vStart = ray.m_Start - center;
	Vector vEnd = vStart + ray.m_Delta;

	float flEnterFrac, flLeaveFrac;
	if (FloatMakePositive(ray.m_Delta.z) < 1e-8)
	{
		if ( (vStart.z < -halfHeight) || (vStart.z > halfHeight) )
		{
			return false; // no hit
		}
		flEnterFrac = 0.0f; flLeaveFrac = 1.0f;
	}
	else
	{
		// Clip the ray to the top and bottom of box
		flEnterFrac = IntersectRayWithAAPlane( vStart, vEnd, 2, 1, halfHeight);
		flLeaveFrac = IntersectRayWithAAPlane( vStart, vEnd, 2, 1, -halfHeight);

		if ( flLeaveFrac < flEnterFrac )
		{
			float temp = flLeaveFrac;
			flLeaveFrac = flEnterFrac;
			flEnterFrac = temp;
		}

		if ( flLeaveFrac < 0 || flEnterFrac > 1)
		{
			return false;
		}
	}

	// Intersect with circle
	float flCircleEnterFrac, flCircleLeaveFrac;
	if ( !LineCircleIntersection( vec3_origin.AsVector2D(), radius,
		vStart.AsVector2D(), ray.m_Delta.AsVector2D(), &flCircleEnterFrac, &flCircleLeaveFrac ) )
	{
		return false; // no hit
	}

	Assert( flCircleEnterFrac <= flCircleLeaveFrac );
	if ( flCircleLeaveFrac < 0 || flCircleEnterFrac > 1)
	{
		return false;
	}

	if ( flEnterFrac < flCircleEnterFrac )
		flEnterFrac = flCircleEnterFrac;
	if ( flLeaveFrac > flCircleLeaveFrac )
		flLeaveFrac = flCircleLeaveFrac;

	if ( flLeaveFrac < flEnterFrac )
		return false;

	VectorMA( ray.m_Start, flEnterFrac , ray.m_Delta, pTrace->endpos );
	pTrace->fraction = flEnterFrac;
	pTrace->contents = CONTENTS_SOLID;

	// Calculate the point on our center line where we're nearest the intersection point
	Vector collisionCenter;
	CalcClosestPointOnLineSegment( pTrace->endpos, center + Vector( 0, 0, halfHeight ), center - Vector( 0, 0, halfHeight ), collisionCenter );
	
	// Our normal is the direction from that center point to the intersection point
	pTrace->plane.normal = pTrace->endpos - collisionCenter;
	VectorNormalize( pTrace->plane.normal );

	return true;
}

bool CHL2_Player::TestHitboxes( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr )
{
	if( g_pGameRules->IsMultiplayer() )
	{
		return BaseClass::TestHitboxes( ray, fContentsMask, tr );
	}
	else
	{
		Assert( ray.m_IsRay );

		Vector mins, maxs;

		mins = WorldAlignMins();
		maxs = WorldAlignMaxs();

		if ( IntersectRayWithAACylinder( ray, WorldSpaceCenter(), maxs.x * PLAYER_HULL_REDUCTION, maxs.z - mins.z, &tr ) )
		{
			tr.hitbox = 0;
			CStudioHdr *pStudioHdr = GetModelPtr( );
			if (!pStudioHdr)
				return false;

			mstudiohitboxset_t *set = pStudioHdr->pHitboxSet( m_nHitboxSet );
			if ( !set || !set->numhitboxes )
				return false;

			mstudiobbox_t *pbox = set->pHitbox( tr.hitbox );
			mstudiobone_t *pBone = pStudioHdr->pBone(pbox->bone);
			tr.surface.name = "**studio**";
			tr.surface.flags = SURF_HITBOX;
			tr.surface.surfaceProps = physprops->GetSurfaceIndex( pBone->pszSurfaceProp() );
		}
		
		return true;
	}
}

//---------------------------------------------------------
// Show the player's scaled down bbox that we use for
// bullet impacts.
//---------------------------------------------------------
void CHL2_Player::DrawDebugGeometryOverlays(void) 
{
	BaseClass::DrawDebugGeometryOverlays();

	if (m_debugOverlays & OVERLAY_BBOX_BIT) 
	{	
		Vector mins, maxs;

		mins = WorldAlignMins();
		maxs = WorldAlignMaxs();

		mins.x *= PLAYER_HULL_REDUCTION;
		mins.y *= PLAYER_HULL_REDUCTION;

		maxs.x *= PLAYER_HULL_REDUCTION;
		maxs.y *= PLAYER_HULL_REDUCTION;

		NDebugOverlay::Box( GetAbsOrigin(), mins, maxs, 255, 0, 0, 100, 0 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Helper to remove from ladder
//-----------------------------------------------------------------------------
void CHL2_Player::ExitLadder()
{
	if ( MOVETYPE_LADDER != GetMoveType() )
		return;
	
	SetMoveType( MOVETYPE_WALK );
	SetMoveCollide( MOVECOLLIDE_DEFAULT );
	// Remove from ladder
	m_HL2Local.m_hLadder.Set( NULL );
}

surfacedata_t *CHL2_Player::GetLadderSurface( const Vector &origin )
{
	extern const char *FuncLadder_GetSurfaceprops(CBaseEntity *pLadderEntity);

	CBaseEntity *pLadder = m_HL2Local.m_hLadder.Get();
	if ( pLadder )
	{
		const char *pSurfaceprops = FuncLadder_GetSurfaceprops(pLadder);
		// get ladder material from func_ladder
		return physprops->GetSurfaceData( physprops->GetSurfaceIndex( pSurfaceprops ) );

	}
	return BaseClass::GetLadderSurface(origin);
}

void CHL2_Player::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

	if ((m_afButtonPressed & IN_HEALTHKIT) && m_bHasHealthkit && (GetHealth() <= 30))
		CAchievementManager::SendAchievement("ACH_HEALTHKIT");

	if ((m_afButtonPressed & IN_OBJECTIVE) && !IsInAVehicle() && !m_bIsInCamView && !m_bIsTransiting)
	{
		EmitSound("TFO.Paper");
		engine->ClientCommand(this->edict(), "tfo_gameui_command OpenInventoryPanel\n");
	}
	
	// Breath sick
	if ( m_HL2Local.m_flSuitPower <= 15 )
		m_bExhausted = true;
	else if ( m_HL2Local.m_flSuitPower >= 85 )
		m_bExhausted = false;

	if (m_bExhausted && gpGlobals->curtime >= m_flSprintExhaustionTime)
	{
		m_flSprintExhaustionTime = gpGlobals->curtime + 5.0f;
		EmitSound("PlayerSprinty.Regen");
	}

	CBaseCombatWeapon* pWep = (gpGlobals->curtime < m_flNextAttack) ? NULL : GetActiveWeapon();
	CBaseViewModel* pVM = GetViewModel();

	if (!pWep)
		return;

	CheckAnimationEvents(pWep);

	// Handle take anim
	// If we have a weapon we can do a hand anim, make a trace from our eyes take the endpos and use it as an origin and search the origin for entities around it.

	if ((gpGlobals->curtime < m_flCheckForItems) || pWep->m_bWantsHolster || pWep->m_bInReload || (gpGlobals->curtime < pWep->m_flNextPrimaryAttack))
		return;

	const Vector vEyePos = EyePosition();
	trace_t	tr;

	Vector vecDir;
	AngleVectors(EyeAngles(), &vecDir);
	VectorNormalize(vecDir);

	UTIL_TraceLine(vEyePos, vEyePos + (vecDir * 40.0f), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	CBaseEntity *aimTarget = NULL;
	bool bFoundEntity = false;

	for (CEntitySphereQuery sphere(tr.endpos, 15.0f); (aimTarget = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity())
	{
		Vector vecDirToTarget = aimTarget->GetLocalOrigin() - vEyePos;
		VectorNormalize(vecDirToTarget);
		if (RAD2DEG(acos(DotProduct(vecDir, vecDirToTarget))) > 30.0f) // Skip entities that we clearly are not looking towards.
			continue;

		// We check for doors, items, buttons, weapons...
		CBaseCombatWeapon* pIsWeapon = dynamic_cast<CBaseCombatWeapon*>(aimTarget);

		if ((pIsWeapon && pIsWeapon->GetOwner()) || aimTarget->GetOwnerEntity())
			continue;

		if (aimTarget->IsItem() || aimTarget->IsWeapon() || pIsWeapon)
		{
			const int iSeqActivity = (pVM ? pVM->GetSequenceActivity(pVM->GetSequence()) : ACT_IDLE);
			if ((iSeqActivity != ACT_VM_DRAW) && (iSeqActivity != ACT_VM_HOLSTER))
				bFoundEntity = true;
			break;
		}		
	}

	const bool bCancelAnim = m_bIsRunning || m_bIsInCamView || m_bShouldSwim || m_bShouldLowerWeapon || (GetGroundEntity() == NULL) || pWep->IsIronsighted();

	if (bFoundEntity && !bCancelAnim)
	{
		if (!ItemTakeAnim)
		{
			ItemTakeAnim = true;
			EmitSound("Weapon.Lower");
			pWep->SendWeaponAnim(ACT_VM_IDLE_TO_TAKE);
			NextShoots = gpGlobals->curtime + pWep->GetViewModelSequenceDuration();
			pWep->m_flNextPrimaryAttack = gpGlobals->curtime + pWep->GetViewModelSequenceDuration();
		}
		else
		{
			if (gpGlobals->curtime >= NextShoots)
			{
				pWep->SendWeaponAnim(ACT_VM_TAKE_IDLE);
				NextShoots = gpGlobals->curtime + pWep->GetViewModelSequenceDuration();
				pWep->m_flNextPrimaryAttack = gpGlobals->curtime + pWep->GetViewModelSequenceDuration();
			}
		}
	}
	else
	{
		if (ItemTakeAnim)
		{
			EmitSound("Weapon.Raise");
			ItemTakeAnim = false;
			pWep->SendWeaponAnim(ACT_VM_TAKE_TO_IDLE);
			NextShoots = gpGlobals->curtime + pWep->GetViewModelSequenceDuration();
			pWep->m_flNextPrimaryAttack = gpGlobals->curtime + pWep->GetViewModelSequenceDuration();
		}
	}

	m_flCheckForItems = gpGlobals->curtime + 0.2f;
}

void CHL2_Player::StartWaterDeathSounds( void )
{
	CPASAttenuationFilter filter( this );

	if ( m_sndLeeches == NULL )
	{
		m_sndLeeches = (CSoundEnvelopeController::GetController()).SoundCreate( filter, entindex(), CHAN_STATIC, "coast.leech_bites_loop" , ATTN_NORM );
	}

	if ( m_sndLeeches )
	{
		(CSoundEnvelopeController::GetController()).Play( m_sndLeeches, 1.0f, 100 );
	}

	if ( m_sndWaterSplashes == NULL )
	{
		m_sndWaterSplashes = (CSoundEnvelopeController::GetController()).SoundCreate( filter, entindex(), CHAN_STATIC, "coast.leech_water_churn_loop" , ATTN_NORM );
	}

	if ( m_sndWaterSplashes )
	{
		(CSoundEnvelopeController::GetController()).Play( m_sndWaterSplashes, 1.0f, 100 );
	}
}

void CHL2_Player::StopWaterDeathSounds( void )
{
	if ( m_sndLeeches )
	{
		(CSoundEnvelopeController::GetController()).SoundFadeOut( m_sndLeeches, 0.5f, true );
		m_sndLeeches = NULL;
	}

	if ( m_sndWaterSplashes )
	{
		(CSoundEnvelopeController::GetController()).SoundFadeOut( m_sndWaterSplashes, 0.5f, true );
		m_sndWaterSplashes = NULL;
	}
}

//-----------------------------------------------------------------------------
// Shuts down sounds
//-----------------------------------------------------------------------------
void CHL2_Player::StopLoopingSounds( void )
{
	if ( m_sndLeeches != NULL )
	{
		 (CSoundEnvelopeController::GetController()).SoundDestroy( m_sndLeeches );
		 m_sndLeeches = NULL;
	}

	if ( m_sndWaterSplashes != NULL )
	{
		 (CSoundEnvelopeController::GetController()).SoundDestroy( m_sndWaterSplashes );
		 m_sndWaterSplashes = NULL;
	}

	BaseClass::StopLoopingSounds();
}

//-----------------------------------------------------------------------------
void CHL2_Player::ModifyOrAppendPlayerCriteria( AI_CriteriaSet& set )
{
	BaseClass::ModifyOrAppendPlayerCriteria( set );

	if ( GlobalEntity_GetIndex( "gordon_precriminal" ) == -1 )
	{
		set.AppendCriteria( "gordon_precriminal", "0" );
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const impactdamagetable_t &CHL2_Player::GetPhysicsImpactDamageTable()
{
	if ( m_bUseCappedPhysicsDamageTable )
		return gCappedPlayerImpactDamageTable;
	
	return BaseClass::GetPhysicsImpactDamageTable();
}


//-----------------------------------------------------------------------------
// Purpose: Makes a splash when the player transitions between water states
//-----------------------------------------------------------------------------
void CHL2_Player::Splash( void )
{
	CEffectData data;
	data.m_fFlags = 0;
	data.m_vOrigin = GetAbsOrigin();
	data.m_vNormal = Vector(0,0,1);
	data.m_vAngles = QAngle( 0, 0, 0 );
	
	if ( GetWaterType() & CONTENTS_SLIME )
	{
		data.m_fFlags |= FX_WATER_IN_SLIME;
	}

	float flSpeed = GetAbsVelocity().Length();
	if ( flSpeed < 300 )
	{
		data.m_flScale = random->RandomFloat( 10, 12 );
		DispatchEffect( "waterripple", data );
	}
	else
	{
		data.m_flScale = random->RandomFloat( 6, 8 );
		DispatchEffect( "watersplash", data );
	}
}

CLogicPlayerProxy *CHL2_Player::GetPlayerProxy( void )
{
	CLogicPlayerProxy *pProxy = dynamic_cast< CLogicPlayerProxy* > ( m_hPlayerProxy.Get() );

	if ( pProxy == NULL )
	{
		pProxy = (CLogicPlayerProxy*)gEntList.FindEntityByClassname(NULL, "logic_playerproxy" );

		if ( pProxy == NULL )
			return NULL;

		pProxy->m_hPlayer = this;
		m_hPlayerProxy = pProxy;
	}

	return pProxy;
}

void CHL2_Player::FirePlayerProxyOutput( const char *pszOutputName, variant_t variant, CBaseEntity *pActivator, CBaseEntity *pCaller )
{
	if ( GetPlayerProxy() == NULL )
		return;

	GetPlayerProxy()->FireNamedOutput( pszOutputName, variant, pActivator, pCaller );
}

LINK_ENTITY_TO_CLASS( logic_playerproxy, CLogicPlayerProxy);

BEGIN_DATADESC( CLogicPlayerProxy )
	DEFINE_OUTPUT( m_RequestedPlayerHealth, "PlayerHealth" ),
	DEFINE_OUTPUT( m_PlayerHasAmmo, "PlayerHasAmmo" ),
	DEFINE_OUTPUT( m_PlayerHasNoAmmo, "PlayerHasNoAmmo" ),
	DEFINE_OUTPUT( m_PlayerDied,	"PlayerDied" ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"RequestPlayerHealth",	InputRequestPlayerHealth ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetPlayerHealth",	InputSetPlayerHealth ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"RequestAmmoState", InputRequestAmmoState ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"LowerWeapon", InputLowerWeapon ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"EnableCappedPhysicsDamage", InputEnableCappedPhysicsDamage ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"DisableCappedPhysicsDamage", InputDisableCappedPhysicsDamage ),
	DEFINE_FIELD( m_hPlayer, FIELD_EHANDLE ),
END_DATADESC()

void CLogicPlayerProxy::Activate( void )
{
	BaseClass::Activate();

	if ( m_hPlayer == NULL )
	{
		m_hPlayer = AI_GetSinglePlayer();
	}
}

bool CLogicPlayerProxy::PassesDamageFilter( const CTakeDamageInfo &info )
{
	if (m_hDamageFilter)
	{
		CBaseFilter *pFilter = (CBaseFilter *)(m_hDamageFilter.Get());
		return pFilter->PassesDamageFilter(info);
	}

	return true;
}

void CLogicPlayerProxy::InputSetPlayerHealth( inputdata_t &inputdata )
{
	if ( m_hPlayer == NULL )
		return;

	m_hPlayer->SetHealth( inputdata.value.Int() );

}

void CLogicPlayerProxy::InputRequestPlayerHealth( inputdata_t &inputdata )
{
	if ( m_hPlayer == NULL )
		return;

	m_RequestedPlayerHealth.Set( m_hPlayer->GetHealth(), inputdata.pActivator, inputdata.pCaller );
}

void CLogicPlayerProxy::InputRequestAmmoState( inputdata_t &inputdata )
{
	if( m_hPlayer == NULL )
		return;

	CHL2_Player *pPlayer = dynamic_cast<CHL2_Player*>(m_hPlayer.Get());

	for ( int i = 0 ; i < pPlayer->WeaponCount(); ++i )
	{
		CBaseCombatWeapon* pCheck = pPlayer->GetWeapon( i );

		if ( pCheck )
		{
			if ( pCheck->HasAnyAmmo() && (pCheck->UsesPrimaryAmmo() || pCheck->UsesSecondaryAmmo()))
			{
				m_PlayerHasAmmo.FireOutput( this, this, 0 );
				return;
			}
		}
	}

	m_PlayerHasNoAmmo.FireOutput( this, this, 0 );
}

void CLogicPlayerProxy::InputLowerWeapon( inputdata_t &inputdata )
{
	if( m_hPlayer == NULL )
		return;

	CHL2_Player *pPlayer = dynamic_cast<CHL2_Player*>(m_hPlayer.Get());

	pPlayer->Weapon_Lower();
}

void CLogicPlayerProxy::InputEnableCappedPhysicsDamage( inputdata_t &inputdata )
{
	if( m_hPlayer == NULL )
		return;

	CHL2_Player *pPlayer = dynamic_cast<CHL2_Player*>(m_hPlayer.Get());
	pPlayer->EnableCappedPhysicsDamage();
}

void CLogicPlayerProxy::InputDisableCappedPhysicsDamage( inputdata_t &inputdata )
{
	if( m_hPlayer == NULL )
		return;

	CHL2_Player *pPlayer = dynamic_cast<CHL2_Player*>(m_hPlayer.Get());
	pPlayer->DisableCappedPhysicsDamage();
}