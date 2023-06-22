//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Door/Portal/Teleport transition handler.
//
//=============================================================================//

#ifndef TFO_FUNC_TRANSITION_H
#define TFO_FUNC_TRANSITION_H

#ifdef _WIN32
#pragma once
#endif

#include "baseentity.h"

class CFuncTransition : public CBaseEntity
{
public:
	DECLARE_CLASS(CFuncTransition, CBaseEntity);
	DECLARE_DATADESC();

	CFuncTransition();

	void Spawn(void);
	void Precache(void);
	bool CreateVPhysics();

	void TransitionUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void OnUse(CBasePlayer* pPlayer);
	void Transit(void);

	void Lock();
	void Unlock();

	void InputLock(inputdata_t& inputdata);
	void InputUnlock(inputdata_t& inputdata);

	void PlaySound(CBasePlayer* pPlayer, const char* pSound);

	int	ObjectCaps(void);

private:
	COutputEvent m_OnUse;
	COutputEvent m_OnUseLocked;

	string_t m_Door;
	string_t m_Destination;

	string_t m_OpenSound;
	string_t m_CloseSound;
	string_t m_LockedSound;

	string_t m_UnlockedMessage;
	string_t m_LockedMessage;

	bool m_bLocked;
	float m_flLastUsed;

	hudtextparms_t	m_textParms;
};

#endif // TFO_FUNC_TRANSITION_H