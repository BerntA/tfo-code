//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "c_basehlcombatweapon.h"
#include "iviewrender_beams.h"
#include "beam_shared.h"
#include "materialsystem/imaterial.h"
#include "clienteffectprecachesystem.h"
#include "beamdraw.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CLIENTEFFECT_REGISTER_BEGIN(PrecacheEffectStunstick)
CLIENTEFFECT_MATERIAL("effects/stunstick")
CLIENTEFFECT_REGISTER_END()