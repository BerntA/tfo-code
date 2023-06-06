//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "c_prop_vehicle.h"
#include "c_vehicle_jeep.h"
#include "movevars_shared.h"
#include "view.h"
#include "flashlighteffect.h"
#include "c_baseplayer.h"
#include "c_te_effect_dispatch.h"
#include "hl2_vehicle_radar.h"
#include "usermessages.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
//
// Client-side Episodic Jeep (Jalopy) Class
//
class C_PropJeepEpisodic : public C_PropJeep
{

	DECLARE_CLASS( C_PropJeepEpisodic, C_PropJeep );

public:
	DECLARE_CLIENTCLASS();

public:
	C_PropJeepEpisodic();

	void OnEnteredVehicle( C_BasePlayer *pPlayer );
	void Simulate( void );

public:
	int		m_iNumRadarContacts;
	Vector	m_vecRadarContactPos[ RADAR_MAX_CONTACTS ];
	int		m_iRadarContactType[ RADAR_MAX_CONTACTS ];
};
C_PropJeepEpisodic *g_pJalopy = NULL;

IMPLEMENT_CLIENTCLASS_DT( C_PropJeepEpisodic, DT_CPropJeepEpisodic, CPropJeepEpisodic )
	//CNetworkVar( int, m_iNumRadarContacts );
	RecvPropInt( RECVINFO(m_iNumRadarContacts) ),

	//CNetworkArray( Vector, m_vecRadarContactPos, RADAR_MAX_CONTACTS );
	RecvPropArray( RecvPropVector(RECVINFO(m_vecRadarContactPos[0])), m_vecRadarContactPos ),

	//CNetworkArray( int, m_iRadarContactType, RADAR_MAX_CONTACTS );
	RecvPropArray( RecvPropInt( RECVINFO(m_iRadarContactType[0] ) ), m_iRadarContactType ),

END_RECV_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
C_PropJeepEpisodic::C_PropJeepEpisodic()
{
	g_pJalopy = this;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_PropJeepEpisodic::Simulate( void )
{
	BaseClass::Simulate();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_PropJeepEpisodic::OnEnteredVehicle( C_BasePlayer *pPlayer )
{
	BaseClass::OnEnteredVehicle( pPlayer );
}
