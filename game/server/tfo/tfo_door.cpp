//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Dynamic door transition.
//
//=============================================================================//

#include "cbase.h"
#include "baseentity.h"
#include "player.h"
#include "gamerules.h"
#include "collisionutils.h"

abstract_class CDynamicDoor
{
	DECLARE_CLASS_NOBASE(CDynamicDoor);

public:
	CDynamicDoor();
	virtual ~CDynamicDoor();

	void Attach(CBaseEntity* pOuter) { m_pOuter = pOuter; }
	void OnUse(CBasePlayer* pTarget);

protected:
	bool m_bIsLocked;

private:
	CBaseEntity* m_pOuter;
};

CDynamicDoor::CDynamicDoor()
{
	m_pOuter = NULL;
}

CDynamicDoor::~CDynamicDoor()
{
	m_pOuter = NULL;
}

void CDynamicDoor::OnUse(CBasePlayer* pTarget)
{
	if (pTarget == NULL || m_pOuter == NULL)
		return;

	const Vector& vDoor = m_pOuter->WorldSpaceCenter();
	const Vector& vDoorMins = m_pOuter->WorldAlignMins();
	const Vector& vDoorMaxs = m_pOuter->WorldAlignMaxs();
	const QAngle& qDoorAngle = m_pOuter->GetAbsAngles();
	const float depth = (vDoorMaxs.x - vDoorMins.x) / 2.0f;

	const Vector& vPlayer = pTarget->GetAbsOrigin();
	const Vector& vPlayerMins = pTarget->GetPlayerMins();
	const Vector& vPlayerMaxs = pTarget->GetPlayerMaxs();
	const QAngle& qPlayerAngle = pTarget->GetAbsAngles();

	Vector vForward;
	AngleVectors(qDoorAngle, &vForward);
	VectorNormalize(vForward);

	if (IsOBBIntersectingOBB(
		vPlayer, qPlayerAngle, vPlayerMins, vPlayerMaxs,
		vDoor + (vForward * depth), qDoorAngle, vDoorMins, vDoorMaxs
	))
	{
		// move player in opposite dir
	}
	else if (IsOBBIntersectingOBB(
		vPlayer, qPlayerAngle, vPlayerMins, vPlayerMaxs,
		vDoor - (vForward * depth), qDoorAngle, vDoorMins, vDoorMaxs
	))
	{
		// move player in opposite dir
		//pTarget->Teleport()
	}
}