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
DEFINE_KEYFIELD(m_spawnPoint, FIELD_STRING, "spawn"),

DEFINE_INPUTFUNC(FIELD_VOID, "ChangeLevel", InputChangeLevel),
END_DATADESC()

CPointChangelevel::CPointChangelevel()
{
	m_nextMap = NULL_STRING;
	m_spawnPoint = NULL_STRING;
}

void CPointChangelevel::InputChangeLevel(inputdata_t& inputdata)
{
	CBasePlayer* pTarget = UTIL_GetLocalPlayer();
	if (pTarget == NULL)
		return;

	color32 black = { 0,0,0,255 };
	UTIL_ScreenFade(pTarget, black, 0.5f, 1.0f, FFADE_OUT | FFADE_STAYOUT);

	ConVar* loadIMG = cvar->FindVar("tfo_loading_image");
	if (loadIMG)
		loadIMG->SetValue(STRING(m_nextMap));

	SetLevelTransitionSpawn(STRING(m_spawnPoint));
	SetChangelevelDebugParams(STRING(m_nextMap), STRING(GetEntityName()));

	// Tell our base plr class that we're goin on a ride...
	pTarget->ProcessTransition();

	SetThink(&CPointChangelevel::DoChangeLevel);
	SetNextThink(gpGlobals->curtime + 1.2f);
}

void CPointChangelevel::DoChangeLevel(void)
{
	engine->ChangeLevel(STRING(m_nextMap), STRING(GetEntityName()));
	SetThink(NULL);
}