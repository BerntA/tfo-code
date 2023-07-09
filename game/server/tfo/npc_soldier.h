//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Custom Soldier NPC - Enemy. Gangsters, etc...
//
//=============================================================================//

#ifndef NPC_SOLDIERS_H
#define NPC_SOLDIERS_H

#ifdef _WIN32
#pragma once
#endif

#include "npc_combine.h"

class CNPC_Soldier : public CNPC_Combine
{
public:
	DECLARE_CLASS(CNPC_Soldier, CNPC_Combine);
	DECLARE_DATADESC();

	void		Spawn(void);
	void		Precache(void);
	void		PrecacheInTemplate(void);
	void		DeathSound(const CTakeDamageInfo& info);
	void		BuildScheduleTestBits(void);
	void		Event_Killed(const CTakeDamageInfo& info);
	void		OnListened();

	void		ClearAttackConditions(void);

	bool		IsLightDamage(const CTakeDamageInfo& info);
	bool		IsHeavyDamage(const CTakeDamageInfo& info);

	bool		AllowedToIgnite(void) { return true; }

	// Allow us to set the script from our spawn function (throuh console)
	void SetScript(const char* szScript);

	// Get Ent Name
	const char* GetEntName(void) { return m_szEntName; }

	Class_T	Classify(void);

	int OnTakeDamage_Alive(const CTakeDamageInfo& inputInfo);

protected:
	DEFINE_CUSTOM_AI;

private:

	// TFO Specific Parsings:
	KeyValues* LoadNPCData(const char* szScript);
	void ParseNPCScript(const char* szScript);
	const char* GetRandomModel(KeyValues* pkvValues);

	bool m_bIsAngry;
	bool m_bIsFriendly;

	string_t m_cScript;
	char m_szEntName[MAX_NPC_SCRIPT_NAME];
};

#endif // NPC_SOLDIERS_H