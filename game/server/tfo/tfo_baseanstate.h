//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Animation State fixes for thirdperson.
// SDK 2013 has server side animations, in mp this is a problem. 
//
//=============================================================================//

#ifndef TFO_BASE_PLAYERANIMSTATE_H
#define TFO_BASE_PLAYERANIMSTATE_H

#include "player.h"
#include "hl2_player.h"
#include "base_playeranimstate.h"
#include "cbase.h"

class CHL2_Player;

class TFOCUSTOMAnimState : public CBasePlayerAnimState
{
public:
	TFOCUSTOMAnimState(CBaseEntity* pOuter)
	{
		m_pOuter = pOuter;
	}
	virtual Activity CalcMainActivity() // return the current activity
	{
		CBasePlayer * pHL2Player = (CBasePlayer*) m_pOuter;
		return pHL2Player->GetActivity();
	}

	virtual int CalcAimLayerSequence( float *flCycle, float *flAimSequenceWeight, bool bForceIdle ){ return 0;}
	virtual float GetCurrentMaxGroundSpeed()
	{
		return 220;
	}

	float TFOGetYaw()
	{
		float flYaw;

		Vector est_velocity;
		est_velocity = m_pOuter->GetAbsVelocity();

		float flLength = est_velocity.Length2D();
		if ( flLength > MOVING_MINIMUM_SPEED )
		{
			m_TFO_GaitYaw = atan2( est_velocity[1], est_velocity[0] );
			m_TFO_GaitYaw = RAD2DEG( m_TFO_GaitYaw);
			m_TFO_GaitYaw = AngleNormalize( m_TFO_GaitYaw );
		}

		float ang = GetEyeYaw();
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
		return (flYaw + 90.0f );
	}
private:
	CBaseEntity* m_pOuter;
	float m_TFO_GaitYaw;
};

#endif // BASE_PLAYERANIMSTATE_H