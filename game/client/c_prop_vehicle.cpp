//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#include "cbase.h"
#include "c_prop_vehicle.h"
#include "hud.h"		
#include <vgui_controls/Controls.h>
#include <Color.h>
#include "view.h"
#include "engine/ivdebugoverlay.h"
#include "movevars_shared.h"
#include "iviewrender.h"
#include "vgui/ISurface.h"

// TFO
#include "iinput.h"
#include "input.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

int ScreenTransform( const Vector& point, Vector& screen );

extern ConVar default_fov;
extern ConVar joy_response_move_vehicle;

extern ConVar cl_thirdperson;
extern ConVar cam_idealdist;
extern ConVar cam_idealdistright;

IMPLEMENT_CLIENTCLASS_DT(C_PropVehicleDriveable, DT_PropVehicleDriveable, CPropVehicleDriveable)
	RecvPropEHandle( RECVINFO(m_hPlayer) ),
	RecvPropInt( RECVINFO( m_nSpeed ) ),
	RecvPropInt( RECVINFO( m_nRPM ) ),
	RecvPropFloat( RECVINFO( m_flThrottle ) ),
	RecvPropInt( RECVINFO( m_nBoostTimeLeft ) ),
	RecvPropInt( RECVINFO( m_nHasBoost ) ),
	RecvPropInt( RECVINFO( m_bEnterAnimOn ) ),
	RecvPropInt( RECVINFO( m_bExitAnimOn ) ),
	RecvPropInt( RECVINFO( m_bUnableToFire ) ),
	RecvPropVector( RECVINFO( m_vecEyeExitEndpoint ) ),
	RecvPropBool( RECVINFO( m_bHasGun ) ),
	RecvPropVector( RECVINFO( m_vecGunCrosshair ) ),
END_RECV_TABLE()


BEGIN_DATADESC( C_PropVehicleDriveable )
	DEFINE_EMBEDDED( m_ViewSmoothingData ),
END_DATADESC()

ConVar r_VehicleViewClamp( "r_VehicleViewClamp", "1", FCVAR_CHEAT );

#define ROLL_CURVE_ZERO		20		// roll less than this is clamped to zero
#define ROLL_CURVE_LINEAR	90		// roll greater than this is copied out

#define PITCH_CURVE_ZERO		10	// pitch less than this is clamped to zero
#define PITCH_CURVE_LINEAR		45	// pitch greater than this is copied out
									// spline in between


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_PropVehicleDriveable::C_PropVehicleDriveable() :
	m_iv_vecGunCrosshair( "C_PropVehicleDriveable::m_iv_vecGunCrosshair" )

{
	m_hPrevPlayer = NULL;

	// TFO Vehicle stuff
	DidEnter = false;

	memset( &m_ViewSmoothingData, 0, sizeof( m_ViewSmoothingData ) );

	m_ViewSmoothingData.pVehicle = this;
	m_ViewSmoothingData.bClampEyeAngles = true;
	m_ViewSmoothingData.bDampenEyePosition = true;

	m_ViewSmoothingData.flPitchCurveZero = PITCH_CURVE_ZERO;
	m_ViewSmoothingData.flPitchCurveLinear = PITCH_CURVE_LINEAR;
	m_ViewSmoothingData.flRollCurveZero = ROLL_CURVE_ZERO;
	m_ViewSmoothingData.flRollCurveLinear = ROLL_CURVE_LINEAR;

	m_ViewSmoothingData.flFOV = m_flFOV = default_fov.GetFloat();

	AddVar( &m_vecGunCrosshair, &m_iv_vecGunCrosshair, LATCH_SIMULATION_VAR );
}

//-----------------------------------------------------------------------------
// Purpose: De-constructor
//-----------------------------------------------------------------------------
C_PropVehicleDriveable::~C_PropVehicleDriveable()
{
}


//-----------------------------------------------------------------------------
// By default all driveable vehicles use the curve defined by the convar.
//-----------------------------------------------------------------------------
int C_PropVehicleDriveable::GetJoystickResponseCurve() const
{
	return joy_response_move_vehicle.GetInt();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_BaseCombatCharacter *C_PropVehicleDriveable::GetPassenger( int nRole )
{
	if ( nRole == VEHICLE_ROLE_DRIVER )
		return m_hPlayer.Get();

	return NULL;
}

//-----------------------------------------------------------------------------
// Returns the role of the passenger
//-----------------------------------------------------------------------------
int	C_PropVehicleDriveable::GetPassengerRole( C_BaseCombatCharacter *pPassenger )
{
	if ( m_hPlayer.Get() == pPassenger )
		return VEHICLE_ROLE_DRIVER;

	return VEHICLE_ROLE_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_PropVehicleDriveable::OnPreDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnPreDataChanged( updateType );

	m_hPrevPlayer = m_hPlayer;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_PropVehicleDriveable::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged(updateType);

	if (m_hPlayer && !m_hPrevPlayer)
	{
		OnEnteredVehicle(m_hPlayer);
		SetNextClientThink(CLIENT_THINK_ALWAYS);

		if (!DidEnter)
		{
			DidEnter = true;
			cam_idealdist.SetValue(155);
			cam_idealdistright.SetValue(0);
			cl_thirdperson.SetValue(1);
		}
	}
	else if (!m_hPlayer && m_hPrevPlayer)
	{
		OnExitedVehicle(m_hPrevPlayer);
		// They have just exited the vehicle.
		// Sometimes we never reach the end of our exit anim, such as if the
		// animation doesn't have fadeout 0 specified in the QC, so we fail to
		// catch it in VehicleViewSmoothing. Catch it here instead.
		m_ViewSmoothingData.bWasRunningAnim = false;
		SetNextClientThink(CLIENT_THINK_NEVER);

		if (DidEnter)
		{
			DidEnter = false;
			cl_thirdperson.SetValue(0);
			SetViewOffset(VEC_VIEW_SCALED(this));
		}
	}
}

//-----------------------------------------------------------------------------
// Should this object cast render-to-texture shadows?
//-----------------------------------------------------------------------------
ShadowType_t C_PropVehicleDriveable::ShadowCastType()
{
	CStudioHdr *pStudioHdr = GetModelPtr();
	if ( !pStudioHdr )
		return SHADOWS_NONE;

	if ( IsEffectActive(EF_NODRAW | EF_NOSHADOW) )
		return SHADOWS_NONE;

	// Always use render-to-texture. We'll always the dirty bits in our think function
	return SHADOWS_RENDER_TO_TEXTURE;
}


//-----------------------------------------------------------------------------
// Mark the shadow as dirty while the vehicle is being driven
//-----------------------------------------------------------------------------
void C_PropVehicleDriveable::ClientThink( void )
{
	// The vehicle is always dirty owing to pose parameters while it's being driven.
	g_pClientShadowMgr->MarkRenderToTextureShadowDirty( GetShadowHandle() );
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_PropVehicleDriveable::DampenEyePosition( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles )
{
}

//-----------------------------------------------------------------------------
// Purpose: Modify the player view/camera while in a vehicle
//-----------------------------------------------------------------------------
void C_PropVehicleDriveable::GetVehicleViewPosition( int nRole, Vector *pAbsOrigin, QAngle *pAbsAngles, float *pFOV /*=NULL*/ )
{
	SharedVehicleViewSmoothing( m_hPlayer,
								pAbsOrigin, pAbsAngles,
								m_bEnterAnimOn, m_bExitAnimOn,
								m_vecEyeExitEndpoint, 
								&m_ViewSmoothingData,
								pFOV );
}


//-----------------------------------------------------------------------------
// Futzes with the clip planes
//-----------------------------------------------------------------------------
void C_PropVehicleDriveable::GetVehicleClipPlanes( float &flZNear, float &flZFar ) const
{
	// FIXME: Need something a better long-term, this fixes the buggy.
	flZNear = 6;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_PropVehicleDriveable::RestrictView( float *pYawBounds, float *pPitchBounds,
										   float *pRollBounds, QAngle &vecViewAngles )
{
	int eyeAttachmentIndex = LookupAttachment( "vehicle_driver_eyes" );
	Vector vehicleEyeOrigin;
	QAngle vehicleEyeAngles;
	GetAttachmentLocal( eyeAttachmentIndex, vehicleEyeOrigin, vehicleEyeAngles );

	// Limit the yaw.
	if ( pYawBounds )
	{
		float flAngleDiff = AngleDiff( vecViewAngles.y, vehicleEyeAngles.y );
		flAngleDiff = clamp( flAngleDiff, pYawBounds[0], pYawBounds[1] );
		vecViewAngles.y = vehicleEyeAngles.y + flAngleDiff;
	}

	// Limit the pitch.
	if ( pPitchBounds )
	{
		float flAngleDiff = AngleDiff( vecViewAngles.x, vehicleEyeAngles.x );
		flAngleDiff = clamp( flAngleDiff, pPitchBounds[0], pPitchBounds[1] );
		vecViewAngles.x = vehicleEyeAngles.x + flAngleDiff;
	}

	// Limit the roll.
	if ( pRollBounds )
	{
		float flAngleDiff = AngleDiff( vecViewAngles.z, vehicleEyeAngles.z );
		flAngleDiff = clamp( flAngleDiff, pRollBounds[0], pRollBounds[1] );
		vecViewAngles.z = vehicleEyeAngles.z + flAngleDiff;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_PropVehicleDriveable::UpdateViewAngles( C_BasePlayer *pLocalPlayer, CUserCmd *pCmd )
{
	/*
	if ( r_VehicleViewClamp.GetInt() )
	{
		float pitchBounds[2] = { -85.0f, 25.0f };
		RestrictView( NULL, pitchBounds, NULL, pCmd->viewangles );
	}
	*/
}

void C_PropVehicleDriveable::OnEnteredVehicle( C_BaseCombatCharacter *pPassenger )
{
}

void C_PropVehicleDriveable::OnExitedVehicle( C_BaseCombatCharacter *pPassenger )
{
}