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
#include "usermessages.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
//
// Client-side Episodic Jeep (Jalopy) Class
//
class C_PropJeepEpisodic : public C_PropJeep
{
	DECLARE_CLASS(C_PropJeepEpisodic, C_PropJeep);

public:
	DECLARE_CLIENTCLASS();

public:
	C_PropJeepEpisodic();
};

IMPLEMENT_CLIENTCLASS_DT(C_PropJeepEpisodic, DT_CPropJeepEpisodic, CPropJeepEpisodic)
END_RECV_TABLE()

C_PropJeepEpisodic::C_PropJeepEpisodic()
{
}