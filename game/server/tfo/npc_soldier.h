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

//=========================================================
//	>> CNPC_Soldier
//=========================================================
class CNPC_Soldier : public CNPC_Combine
{
	DECLARE_CLASS( CNPC_Soldier, CNPC_Combine );
#if HL2_EPISODIC
	DECLARE_DATADESC();
#endif

public: 
	void		Spawn( void );
	void		Precache( void );
	void		DeathSound( const CTakeDamageInfo &info );
	void		PrescheduleThink( void );
	void		BuildScheduleTestBits( void );
	int			SelectSchedule ( void );
	float		GetHitgroupDamageMultiplier( int iHitGroup, const CTakeDamageInfo &info );
	void		HandleAnimEvent( animevent_t *pEvent );
	void		OnChangeActivity( Activity eNewActivity );
	void		Event_Killed( const CTakeDamageInfo &info );
	void		OnListened();

	void		ClearAttackConditions( void );

	bool		m_fIsBlocking;

	bool		IsLightDamage( const CTakeDamageInfo &info );
	bool		IsHeavyDamage( const CTakeDamageInfo &info );

	virtual	bool		AllowedToIgnite( void ) { return true; }

	// Allow us to set the script from our spawn function (throuh console)
	void SetScript(const char *szScript);

	// Get Ent Name
	const char *GetEntName(void) { return cEntName.ToCStr(); }
	int GetMaxHP(void) { return m_iMaxHealth; }

	// Override Class:
	virtual Class_T	Classify(void);

	virtual int OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo);

private:
	bool		ShouldHitPlayer( const Vector &targetDir, float targetDist );

	// TFO Specific Parsings:
	KeyValues *pkvNPCData(const char *szScript);
	void ParseNPCScript(const char *szScript);
	const char *GetRandomModel(KeyValues *pkvValues);

	bool m_bIsAngry;
	bool m_bIsFriendly;
	int m_iMaxHealth;
	int m_iDamage;
	string_t cModel;
	string_t cEntName;
	string_t cScript;

#if HL2_EPISODIC
public:
	Activity	NPC_TranslateActivity( Activity eNewActivity );
#endif

};

#endif // NPC_SOLDIERS_H
