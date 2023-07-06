//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: APIs for player pickup of physics objects
//
//=============================================================================//

#ifndef PLAYER_PICKUP_H
#define PLAYER_PICKUP_H

#ifdef _WIN32
#pragma once
#endif

void PlayerPickupObject(CBasePlayer* pPlayer, CBaseEntity* pObject);
void Pickup_ForcePlayerToDropThisObject(CBaseEntity* pTarget);
bool Pickup_GetPreferredCarryAngles(CBaseEntity* pObject, CBasePlayer* pPlayer, matrix3x4_t& localToWorld, QAngle& outputAnglesWorldSpace);

abstract_class IPlayerPickupVPhysics
{
public:
	virtual bool			HasPreferredCarryAnglesForPlayer(CBasePlayer * pPlayer = NULL) = 0;
	virtual QAngle			PreferredCarryAngles(void) = 0;
};

class CDefaultPlayerPickupVPhysics : public IPlayerPickupVPhysics
{
public:
	virtual bool			HasPreferredCarryAnglesForPlayer(CBasePlayer* pPlayer) { return false; }
	virtual QAngle			PreferredCarryAngles(void) { return vec3_angle; }
};

#endif // PLAYER_PICKUP_H