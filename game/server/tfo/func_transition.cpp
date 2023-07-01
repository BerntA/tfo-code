//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Door/Portal/Teleport transition handler.
//
//=============================================================================//

#include "cbase.h"
#include "func_transition.h"
#include "player.h"
#include "props.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SF_TRANSITION_LOCKED 2048 // Start locked

ConVar func_transition_time("func_transition_time", "1.25");
ConVar func_transition_fade_time("func_transition_fade_time", "0.5");

LINK_ENTITY_TO_CLASS(func_transition, CFuncTransition);

BEGIN_DATADESC(CFuncTransition)

DEFINE_KEYFIELD(m_Door, FIELD_STRING, "TargetDoor"),
DEFINE_KEYFIELD(m_Destination, FIELD_STRING, "TargetDestination"),

DEFINE_KEYFIELD(m_OpenSound, FIELD_STRING, "OpenSound"),
DEFINE_KEYFIELD(m_CloseSound, FIELD_STRING, "CloseSound"),
DEFINE_KEYFIELD(m_LockedSound, FIELD_STRING, "LockedSound"),

DEFINE_KEYFIELD(m_UnlockedMessage, FIELD_STRING, "UnlockedMessage"),
DEFINE_KEYFIELD(m_LockedMessage, FIELD_STRING, "LockedMessage"),

// Function Pointers
DEFINE_USEFUNC(TransitionUse),
DEFINE_THINKFUNC(Transit),

// Inputs
DEFINE_INPUTFUNC(FIELD_VOID, "Lock", InputLock),
DEFINE_INPUTFUNC(FIELD_VOID, "Unlock", InputUnlock),

// Outputs
DEFINE_OUTPUT(m_OnUse, "OnUse"),
DEFINE_OUTPUT(m_OnUseLocked, "OnUseLocked"),

// Saved fields
DEFINE_FIELD(m_bLocked, FIELD_BOOLEAN),

END_DATADESC()

CFuncTransition::CFuncTransition()
{
	m_bLocked = m_bIsActive = false;
	m_flLastUsed = 0.0f;

	m_Door = NULL_STRING;
	m_Destination = NULL_STRING;

	m_OpenSound = NULL_STRING;
	m_CloseSound = NULL_STRING;
	m_LockedSound = NULL_STRING;

	m_UnlockedMessage = NULL_STRING;
	m_LockedMessage = NULL_STRING;

	m_textParms.channel = 1;
	m_textParms.x = -1;
	m_textParms.y = 0.7;
	m_textParms.effect = 0;
	m_textParms.fadeinTime = 1.5f;
	m_textParms.fadeoutTime = 0.5f;
	m_textParms.holdTime = 2.0f;
	m_textParms.fxTime = 0.25f;

	m_textParms.r1 = 100;
	m_textParms.g1 = 100;
	m_textParms.b1 = 100;

	m_textParms.r2 = 240;
	m_textParms.g2 = 110;
	m_textParms.b2 = 0;
}

void CFuncTransition::Spawn(void)
{
	BaseClass::Spawn();

	Precache();

	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_BSP);
	SetModel(STRING(GetModelName()));

	SetUse(&CFuncTransition::TransitionUse);

	m_takedamage = DAMAGE_NO;

	CreateVPhysics();

	AddEffects(EF_NOSHADOW | EF_NORECEIVESHADOW);
	SetBlocksLOS(false);
	m_nRenderMode = kRenderEnvironmental; // do not render

	if (HasSpawnFlags(SF_TRANSITION_LOCKED))
		m_bLocked = true;
}

void CFuncTransition::Precache(void)
{
	BaseClass::Precache();

	const char* lockedSound = STRING(m_LockedSound);
	const char* openSound = STRING(m_OpenSound);
	const char* closeSound = STRING(m_CloseSound);

	if (lockedSound && lockedSound[0])
		PrecacheScriptSound(lockedSound);

	if (openSound && openSound[0])
		PrecacheScriptSound(openSound);

	if (closeSound && closeSound[0])
		PrecacheScriptSound(closeSound);
}

bool CFuncTransition::CreateVPhysics()
{
	VPhysicsInitStatic();
	return true;
}

void CFuncTransition::TransitionUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (m_bIsActive || (m_flLastUsed >= gpGlobals->curtime))
		return;

	CBasePlayer* pPlayer = ToBasePlayer(pActivator);
	if (!pPlayer)
		return;

	m_flLastUsed = (gpGlobals->curtime + 1.0f);
	OnUse(pPlayer);
}

void CFuncTransition::OnUse(CBasePlayer* pPlayer)
{
	if (!pPlayer || pPlayer->m_bIsTransiting)
		return; // busy

	const char* pDoor = STRING(m_Door);

	if (pDoor && pDoor[0]) // ensure that default anim for door is idle.
	{
		CDynamicProp* pDoorProp = NULL;
		do
		{
			pDoorProp = dynamic_cast<CDynamicProp*>(gEntList.FindEntityByName(pDoorProp, pDoor));
			if (pDoorProp)
				pDoorProp->PropSetAnim("handle");

		} while (pDoorProp != NULL);
	}

	if (m_bLocked)
	{
		m_OnUseLocked.FireOutput(pPlayer, this);

		const char* lockedMessage = STRING(m_LockedMessage);
		if (lockedMessage && lockedMessage[0])
			UTIL_HudMessage(pPlayer, m_textParms, lockedMessage);

		PlaySound(pPlayer, STRING(m_LockedSound));
		return;
	}

	m_bIsActive = true;
	m_OnUse.FireOutput(pPlayer, this);

	pPlayer->SetLaggedMovementValue(0.1f);
	PlaySound(pPlayer, STRING(m_OpenSound));

	color32 black = { 0,0,0,255 };
	UTIL_ScreenFade(pPlayer, black, func_transition_fade_time.GetFloat(), 1.0f, FFADE_OUT | FFADE_STAYOUT | FFADE_PURGE);

	SetThink(&CFuncTransition::Transit);
	SetNextThink(gpGlobals->curtime + func_transition_time.GetFloat());
}

void CFuncTransition::Transit(void)
{
	m_bIsActive = false;
	SetThink(NULL);

	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer)
		return;

	const char* pDest = STRING(m_Destination);
	if (!pDest || !pDest[0]) // no dest = assume change level.
		return;

	pPlayer->SetLaggedMovementValue(1.0f);
	PlaySound(pPlayer, STRING(m_CloseSound));

	color32 black = { 0,0,0,255 };
	UTIL_ScreenFade(pPlayer, black, func_transition_fade_time.GetFloat(), 0.0f, FFADE_IN | FFADE_PURGE);

	CBaseEntity* pTeleportDest = gEntList.FindEntityByName(NULL, pDest);
	if (pTeleportDest)
	{
		Vector vPos = pTeleportDest->GetAbsOrigin();
		QAngle angles = pTeleportDest->GetAbsAngles();

		CTraceFilterWorldOnly filter;
		trace_t tr;
		UTIL_TraceLine(vPos, vPos + Vector(0.0f, 0.0f, -1.0f) * MAX_TRACE_LENGTH, MASK_SHOT, &filter, &tr);

		pPlayer->SetLocalOrigin(tr.endpos + Vector(0, 0, 1));
		pPlayer->SetLocalAngles(angles);
		pPlayer->SnapEyeAngles(angles);
		pPlayer->SetAbsVelocity(vec3_origin);

		pPlayer->m_Local.m_vecPunchAngle = vec3_angle;
		pPlayer->m_Local.m_vecPunchAngleVel = vec3_angle;

		pPlayer->PhysicsTouchTriggers();
	}
}

void CFuncTransition::Lock()
{
	m_bLocked = true;
}

void CFuncTransition::Unlock()
{
	if (m_bLocked)
	{
		CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
		const char* unlockedMessage = STRING(m_UnlockedMessage);
		if (pPlayer && unlockedMessage && unlockedMessage[0])
			UTIL_HudMessage(pPlayer, m_textParms, unlockedMessage);
	}

	m_bLocked = false;
}

void CFuncTransition::InputLock(inputdata_t& inputdata)
{
	Lock();
}

void CFuncTransition::InputUnlock(inputdata_t& inputdata)
{
	Unlock();
}

void CFuncTransition::PlaySound(CBasePlayer* pPlayer, const char* pSound)
{
	if (!pSound || !pSound[0])
		return;

	static char chSoundName[128];

	CSoundParameters params;
	const bool bIsSoundScript = GetParametersForSound(pSound, params, NULL);

	Q_strncpy(chSoundName, (bIsSoundScript ? params.soundname : pSound), sizeof(chSoundName));

	const Vector vPos = pPlayer->EyePosition();

	CSingleUserRecipientFilter filter(pPlayer);

	EmitSound_t ep;
	ep.m_nChannel = CHAN_STATIC;
	ep.m_pSoundName = chSoundName;
	ep.m_flVolume = bIsSoundScript ? params.volume : 1.0f;
	ep.m_SoundLevel = SNDLVL_NONE;
	ep.m_nFlags = 0;
	ep.m_nPitch = PITCH_NORM;
	ep.m_pOrigin = &vPos;

	EmitSound(filter, entindex(), ep);
}

int	CFuncTransition::ObjectCaps(void)
{
	return ((BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_IMPULSE_USE | FCAP_USE_IN_RADIUS);
}