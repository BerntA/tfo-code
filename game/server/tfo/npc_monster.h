//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Custom Monster NPC Entity - Allows parsing of unique stuff. Parses npc scripts in data/npc
//
//=============================================================================//

#ifndef NPC_MONSTER_CUSTOM_H
#define NPC_MONSTER_CUSTOM_H

#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "ai_basenpc.h"
#include "ai_default.h"
#include "npcevent.h"
#include "entitylist.h"
#include "npc_BaseZombie.h"

class CNPC_Monster : public CAI_BlendingHost<CNPC_BaseZombie>
{
	DECLARE_CLASS(CNPC_Monster, CAI_BlendingHost<CNPC_BaseZombie>);
	DECLARE_DATADESC();

public:

	bool ShouldBecomeTorso(const CTakeDamageInfo& info, float flDamageThreshold);
	bool IsChopped(const CTakeDamageInfo& info) { return false; }
	bool CanSwatPhysicsObjects(void) { return false; }
	float GetClawAttackRange() const { return m_flAttackRange; }

	void PrescheduleThink(void);
	void BuildScheduleTestBits(void);
	int SelectSchedule(void);
	int TranslateSchedule(int scheduleType);
	bool ShouldPlayIdleSound(void);

	void HandleAnimEvent(animevent_t* pEvent);

	void Spawn(void);
	void Precache(void);
	void PrecacheInTemplate(void);
	void SetZombieModel(void);

	Class_T Classify(void);

	void PainSound(const CTakeDamageInfo& info);
	void DeathSound(const CTakeDamageInfo& info);
	void AlertSound(void);
	void IdleSound(void);
	void AttackSound(void);
	void AttackHitSound(void);
	void AttackMissSound(void);
	void FootstepSound(bool fRightFoot);
	void FootscuffSound(bool fRightFoot) {};

	// Allow us to set the script from our spawn function (thru console)
	void SetScript(const char* szScript);

	// Get Ent Name
	const char* GetEntName(void) { return m_pchEntName; }

protected:

	DEFINE_CUSTOM_AI;

	bool CanOpenDoors(void) { return m_bCanOpenDoors; }
	bool MustCloseToAttack(void);

	const char* GetMoanSound(int nSoundIndex);

private:

	// TFO Specific Parsings:
	KeyValues* LoadNPCData(const char* szScript);
	void ParseNPCScript(const char* szScript);
	const char* GetRandomModel(KeyValues* pkvValues);

	bool m_bCanOpenDoors;
	bool m_bNearEnemy;

	int m_iSkin;
	int m_iDamage[2];

	float m_flAttackRange;
	float m_flLastBloodTrail;
	float m_flNextPainSoundTime;

	char m_pchEntName[MAX_NPC_SCRIPT_NAME];
	char m_pchModelPath[MAX_WEAPON_STRING];
	char m_pchSoundScript[MAX_WEAPON_STRING];
	string_t m_cScript;
};

#endif // NPC_MONSTER_CUSTOM_H