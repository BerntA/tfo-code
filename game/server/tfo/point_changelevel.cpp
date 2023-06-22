//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Simplified changelevel logic.
//
//=============================================================================//

#include "cbase.h"
#include "point_changelevel.h"
#include "player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static char g_chSpawnPoint[MAX_MAP_NAME]; // Level transition spawn point

extern void SetChangelevelDebugParams(const char* map, const char* landmark);

const char* GetLevelTransitionSpawn(void) { return g_chSpawnPoint; }
void SetLevelTransitionSpawn(const char* value) { Q_strncpy(g_chSpawnPoint, value, MAX_MAP_NAME); }

LINK_ENTITY_TO_CLASS(point_changelevel, CPointChangelevel);

BEGIN_DATADESC(CPointChangelevel)
DEFINE_KEYFIELD(m_nextMap, FIELD_STRING, "map"),
DEFINE_KEYFIELD(m_loadingScreen, FIELD_STRING, "loadingimage"),

DEFINE_INPUTFUNC(FIELD_VOID, "ChangeLevel", InputChangeLevel),

DEFINE_OUTPUT(m_OnSpawnedInPoint, "OnSpawnInPoint"),
DEFINE_OUTPUT(m_OnChangeLevel, "OnChangeLevel"),
END_DATADESC()

CPointChangelevel::CPointChangelevel()
{
	m_nextMap = NULL_STRING;
	m_loadingScreen = NULL_STRING;
}

void CPointChangelevel::Spawn()
{
	BaseClass::Spawn();

	if (m_nextMap == NULL_STRING)
	{
		Warning("Found point_changelevel with no next map, removing!\n");
		UTIL_Remove(this);
		return;
	}

	const char* pLandmark = GetLandmark();
	if (!pLandmark || !pLandmark[0])
	{
		Warning("Found point_changelevel with no targetname, removing!\n");
		UTIL_Remove(this);
		return;
	}
}

void CPointChangelevel::InputChangeLevel(inputdata_t& inputdata)
{
	CBasePlayer* pTarget = UTIL_GetLocalPlayer();
	if (pTarget == NULL)
		return;

	color32 black = { 0,0,0,255 };
	UTIL_ScreenFade(pTarget, black, 0.5f, 1.0f, FFADE_OUT | FFADE_STAYOUT | FFADE_PURGE);

	ConVar* loadIMG = cvar->FindVar("tfo_loading_image");
	if (loadIMG)
		loadIMG->SetValue(GetLoadingScreen());

	SetLevelTransitionSpawn(GetLandmark());
	SetChangelevelDebugParams(GetNextMap(), GetLandmark());

	// Tell our base plr class that we're goin on a ride...
	pTarget->ProcessTransition();
	pTarget->SetLaggedMovementValue(0.1f);

	m_OnChangeLevel.FireOutput(pTarget, this);

	SetThink(&CPointChangelevel::DoChangeLevel);
	SetNextThink(gpGlobals->curtime + 1.2f);
}

void CPointChangelevel::DoChangeLevel(void)
{
	SetThink(NULL);
	engine->ChangeLevel(GetNextMap(), GetLandmark());
}

void CPointChangelevel::OnSpawnedInPoint()
{
	CBasePlayer* pTarget = UTIL_GetLocalPlayer();
	if (pTarget)
		pTarget->SetLaggedMovementValue(1.0f);

	m_OnSpawnedInPoint.FireOutput(pTarget, this, 0.1f);
}