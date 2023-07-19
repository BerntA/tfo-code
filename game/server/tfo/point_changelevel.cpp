//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Simplified changelevel logic.
//
//=============================================================================//

#include "cbase.h"
#include "point_changelevel.h"
#include "player.h"
#include "hl2_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static char g_chSpawnPoint[MAX_MAP_NAME]; // Level transition spawn point

extern ConVar func_transition_time;
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
	if (!pTarget || pTarget->m_bIsTransiting)
		return;

	tfo_loading_image.SetValue(GetLoadingScreen());

	SetLevelTransitionSpawn(GetLandmark());
	SetChangelevelDebugParams(GetNextMap(), GetLandmark());

	pTarget->ProcessTransition();

	m_OnChangeLevel.FireOutput(pTarget, this);

	SetThink(&CPointChangelevel::DoChangeLevel);
	SetNextThink(gpGlobals->curtime + func_transition_time.GetFloat() + 0.1f);
}

void CPointChangelevel::DoChangeLevel(void)
{
	SetThink(NULL);
	engine->ChangeLevel(GetNextMap(), GetLandmark());
}

void CPointChangelevel::OnSpawnedInPoint()
{
	m_OnSpawnedInPoint.FireOutput(UTIL_GetLocalPlayer(), this, 0.1f);
}

int	CPointChangelevel::ObjectCaps(void)
{
	return (BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION);
}