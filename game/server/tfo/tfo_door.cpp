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

#define SF_DOOR_LOCKED 2048	// Door is initially locked

class CDynamicDoor
{
	DECLARE_CLASS_NOBASE(CDynamicDoor);
	DECLARE_SIMPLE_DATADESC();

public:
	explicit CDynamicDoor(CBaseEntity* pOuter);
	virtual ~CDynamicDoor();

	void Attach(CBaseEntity* pOuter) { m_pOuter = pOuter; }
	void Precache();

	void OnUse(CBasePlayer* pTarget);
	void OnLockDoor(void);
	void OnUnlockDoor(void);
	void SetLockedState(bool value) { m_bIsLocked = value; }

private:
	CBaseEntity* m_pOuter;

	bool m_bIsLocked;

	COutputEvent m_OnUse;
	COutputEvent m_OnLockedUse;

	COutputEvent m_OnLocked;
	COutputEvent m_OnUnlocked;
};

BEGIN_SIMPLE_DATADESC(CDynamicDoor)

DEFINE_FIELD(m_bIsLocked, FIELD_BOOLEAN),

DEFINE_OUTPUT(m_OnUse, "OnUse"),
DEFINE_OUTPUT(m_OnLockedUse, "OnLockedUse"),

DEFINE_OUTPUT(m_OnLocked, "OnLocked"),
DEFINE_OUTPUT(m_OnUnlocked, "OnUnlocked"),

END_DATADESC()

CDynamicDoor::CDynamicDoor(CBaseEntity* pOuter)
{
	Msg("create door obj\n");
	m_pOuter = pOuter;
	m_bIsLocked = false;
}

CDynamicDoor::~CDynamicDoor()
{
	m_pOuter = NULL;
}

void CDynamicDoor::Precache()
{
	// precache anything 
}

void CDynamicDoor::OnUse(CBasePlayer* pTarget)
{
	if (pTarget == NULL || m_pOuter == NULL)
		return;

	Msg("Open Door : %i\n", m_bIsLocked ? 1 : 0);

	if (m_bIsLocked)
	{
		m_OnLockedUse.FireOutput(pTarget, m_pOuter);
		// play the locked sound + locked text here!
		return;
	}

	m_OnUse.FireOutput(pTarget, m_pOuter);
	// play opening sound

	const Vector& vDoor = m_pOuter->GetAbsOrigin();
	const Vector& vDoorMins = m_pOuter->WorldAlignMins();
	const Vector& vDoorMaxs = m_pOuter->WorldAlignMaxs();
	const Vector& vTransitPos = m_pOuter->WorldSpaceCenter();

	const QAngle& qDoorRenderAngle = m_pOuter->GetAbsAngles();
	QAngle qDoorAngle = qDoorRenderAngle;
	qDoorAngle.y += 90;
	qDoorAngle.x = 0.0f;
	qDoorAngle.z = 0.0f;

	const float depth = (vDoorMaxs.x - vDoorMins.x) / 2.0f;

	const Vector& vPlayer = pTarget->GetAbsOrigin();
	const Vector& vPlayerMins = pTarget->GetPlayerMins();
	const Vector& vPlayerMaxs = pTarget->GetPlayerMaxs();
	const QAngle& qPlayerAngle = pTarget->GetAbsAngles();

	Vector vForward;
	AngleVectors(qDoorAngle, &vForward);
	VectorNormalize(vForward);

	debugoverlay->AddLineOverlay(vDoor, vDoor + vForward * 150.0f, 0, 0, 200, true, 3.0f);
	debugoverlay->AddLineOverlay(m_pOuter->WorldSpaceCenter(), m_pOuter->WorldSpaceCenter() + vForward * 150.0f, 0, 200, 200, true, 3.0f);

	Msg("%f %f %f\n", vDoorMins.x, vDoorMins.y, vDoorMins.z);
	Msg("%f %f %f\n", vDoorMaxs.x, vDoorMaxs.y, vDoorMaxs.z);

	debugoverlay->AddBoxOverlay(vDoor + vForward * depth, vDoorMins, vDoorMaxs, qDoorRenderAngle, 200, 0, 0, 200, 4.0f);
	debugoverlay->AddBoxOverlay(vDoor - vForward * depth, vDoorMins, vDoorMaxs, qDoorRenderAngle, 0, 200, 0, 200, 4.0f);

	// do fade + slow down player
	color32 black = { 0,0,0,255 };
	UTIL_ScreenFade(pTarget, black, 0.5f, 1.0f, FFADE_OUT);

	Vector vTargetPos = vec3_invalid;
	QAngle angles;
	VectorAngles(vForward, angles);

	if (IsOBBIntersectingOBB(
		vPlayer, qPlayerAngle, vPlayerMins, vPlayerMaxs,
		vDoor + (vForward * depth), qDoorRenderAngle, vDoorMins, vDoorMaxs
	))
	{
		angles.y += 180;
		vTargetPos = vTransitPos - (vForward * 30.0f);
	}
	else if (IsOBBIntersectingOBB(
		vPlayer, qPlayerAngle, vPlayerMins, vPlayerMaxs,
		vDoor - (vForward * depth), qDoorRenderAngle, vDoorMins, vDoorMaxs
	))
	{
		vTargetPos = vTransitPos + (vForward * 30.0f);
	}

	if (vTargetPos == vec3_invalid)
		return;

	trace_t tr;
	CTraceFilterWorldOnly filter;
	UTIL_TraceLine(vTargetPos, vTargetPos + Vector(0, 0, -1) * MAX_TRACE_LENGTH, MASK_SHOT, &filter, &tr);

	pTarget->Teleport(&tr.endpos, &angles, &vec3_origin);
}

void CDynamicDoor::OnLockDoor(void)
{
	if (m_bIsLocked)
		return;

	SetLockedState(true);
	// play locked sound + locked text if any
}

void CDynamicDoor::OnUnlockDoor(void)
{
	if (m_bIsLocked == false)
		return;

	SetLockedState(false);
	// play unlock sound + unlocked text if any
}

class CPropDynamicDoor : public CBaseAnimating
{
public:
	DECLARE_CLASS(CPropDynamicDoor, CBaseAnimating);
	DECLARE_DATADESC();

	CPropDynamicDoor() : door(this)
	{
		Msg("init\n");
	}

	int ObjectCaps(void) { return (BaseClass::ObjectCaps() | FCAP_CONTINUOUS_USE | FCAP_USE_IN_RADIUS); }

	void Spawn();
	void Precache() { BaseClass::Precache(); PrecacheModel(STRING(GetModelName())); door.Precache(); }
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) { door.OnUse(ToBasePlayer(pActivator)); }

	void InputLockDoor(inputdata_t& inputdata) { door.OnLockDoor(); }
	void InputUnlockDoor(inputdata_t& inputdata) { door.OnUnlockDoor(); }

private:
	CDynamicDoor door;

	COutputEvent m_OnUse;
	COutputEvent m_OnLockedUse;

	COutputEvent m_OnLocked;
	COutputEvent m_OnUnlocked;

};

LINK_ENTITY_TO_CLASS(prop_dynamic_door, CPropDynamicDoor);

BEGIN_DATADESC(CPropDynamicDoor)

DEFINE_EMBEDDED(door),
DEFINE_INPUTFUNC(FIELD_VOID, "LockDoor", InputLockDoor),
DEFINE_INPUTFUNC(FIELD_VOID, "UnlockDoor", InputUnlockDoor),

END_DATADESC()

void CPropDynamicDoor::Spawn()
{
	BaseClass::Spawn();

	m_flFadeScale = 0;
	m_takedamage = DAMAGE_NO;

	Precache();
	SetModel(STRING(GetModelName()));

	if (HasSpawnFlags(SF_DOOR_LOCKED))
		door.SetLockedState(true);

	SetMoveType(MOVETYPE_NONE);
	RemoveFlag(FL_STATICPROP);

	SetSolid(SOLID_VPHYSICS);
	VPhysicsInitStatic();

	SetBlocksLOS(true);
	AddEffects(EF_NOSHADOW | EF_NORECEIVESHADOW);
}