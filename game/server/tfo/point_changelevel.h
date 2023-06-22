//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Simplified changelevel logic.
//
//=============================================================================//

#ifndef TFO_POINT_CHANGELEVEL_H
#define TFO_POINT_CHANGELEVEL_H

#ifdef _WIN32
#pragma once
#endif

#include "baseentity.h"

class CPointChangelevel : public CPointEntity
{
public:
	DECLARE_CLASS(CPointChangelevel, CPointEntity);
	DECLARE_DATADESC();

	CPointChangelevel();

	void Spawn();
	void InputChangeLevel(inputdata_t& inputdata);
	void DoChangeLevel(void);
	void OnSpawnedInPoint();
	int	ObjectCaps(void);

	const char* GetNextMap(void) { return STRING(m_nextMap); }
	const char* GetLandmark(void) { return STRING(GetEntityName()); }
	const char* GetLoadingScreen(void) { return STRING(m_loadingScreen); }

private:
	string_t m_nextMap;
	string_t m_loadingScreen;

	COutputEvent m_OnSpawnedInPoint;
	COutputEvent m_OnChangeLevel;
};

#endif // TFO_POINT_CHANGELEVEL_H