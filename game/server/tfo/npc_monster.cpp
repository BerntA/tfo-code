//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Custom Monster NPC Entity - Allows parsing of unique stuff. Parses npc scripts in resource/data/npc
//
//=============================================================================//

#include "cbase.h"
#include "ai_basenpc.h"
#include "ai_default.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "ai_motor.h"
#include "game.h"
#include "npcevent.h"
#include "entitylist.h"
#include "ai_task.h"
#include "activitylist.h"
#include "engine/IEngineSound.h"
#include "npc_BaseZombie.h"
#include "npc_monster.h"
#include "filesystem.h"
#include <KeyValues.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define BREATH_VOL_MAX  0.6
#define ZOMBIE_ENEMY_BREATHE_DIST		300	// How close we must be to our enemy before we start breathing hard.

LINK_ENTITY_TO_CLASS(npc_monster, CNPC_Monster);

BEGIN_DATADESC(CNPC_Monster)
DEFINE_FIELD(m_flNextPainSoundTime, FIELD_TIME),
DEFINE_FIELD(m_bCanOpenDoors, FIELD_BOOLEAN),
DEFINE_FIELD(m_bNearEnemy, FIELD_BOOLEAN),
DEFINE_FIELD(m_iSkin, FIELD_INTEGER),
DEFINE_FIELD(m_iDamage[0], FIELD_INTEGER),
DEFINE_FIELD(m_iDamage[1], FIELD_INTEGER),
DEFINE_FIELD(m_flAttackRange, FIELD_FLOAT),
DEFINE_ARRAY(m_pchEntName, FIELD_CHARACTER, MAX_NPC_SCRIPT_NAME),
DEFINE_ARRAY(m_pchModelPath, FIELD_CHARACTER, MAX_WEAPON_STRING),
DEFINE_ARRAY(m_pchSoundScript, FIELD_CHARACTER, MAX_WEAPON_STRING),
DEFINE_KEYFIELD(m_cScript, FIELD_STRING, "Script"),
END_DATADESC()

void CNPC_Monster::SetScript(const char* szScript)
{
	m_cScript = MAKE_STRING(szScript);
}

const char* CNPC_Monster::GetRandomModel(KeyValues* pkvValues)
{
	int iCount = 0, iFind = 0;

	for (KeyValues* sub = pkvValues->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		iCount++;

	iFind = random->RandomInt(1, iCount);
	iCount = 0;

	for (KeyValues* sub = pkvValues->GetFirstSubKey(); sub; sub = sub->GetNextKey())
	{
		iCount++;
		if (iCount == iFind)
			return sub->GetString();
	}

	return NULL;
}

KeyValues* CNPC_Monster::LoadNPCData(const char* szScript)
{
	KeyValues* pkvData = new KeyValues("NPCDATA");
	if (pkvData->LoadFromFile(filesystem, UTIL_VarArgs("resource/data/npcs/%s.txt", szScript), "MOD"))
		return pkvData;

	pkvData->deleteThis();
	return NULL;
}

void CNPC_Monster::ParseNPCScript(const char* szScript)
{
	KeyValues* pkvMyNPCData = LoadNPCData(szScript);
	if (!pkvMyNPCData)
	{
		Warning("NPC_MONSTER linked to %s.txt was removed, no such script exist!\n", szScript);
		UTIL_Remove(this);
		return;
	}

	KeyValues* pkvInfoField = pkvMyNPCData->FindKey("Info");
	KeyValues* pkvModelField = pkvMyNPCData->FindKey("Model");
	KeyValues* pkvRandomModelsField = pkvMyNPCData->FindKey("Models");

	const bool bRandomModel = (pkvRandomModelsField && pkvModelField && !pkvModelField->FindKey("Path"));

	Q_strncpy(m_pchEntName, (pkvInfoField ? pkvInfoField->GetString("Name") : ""), MAX_NPC_SCRIPT_NAME);
	Q_strncpy(m_pchSoundScript, (pkvInfoField ? pkvInfoField->GetString("SoundScript") : ""), MAX_WEAPON_STRING);

	m_iHealth = (pkvInfoField ? pkvInfoField->GetInt("Health", 100) : 100);
	m_iMaxHealth = m_iHealth;

	m_iDamage[0] = (pkvInfoField ? pkvInfoField->GetInt("DamageOneHand", 10) : 10);
	m_iDamage[1] = (pkvInfoField ? pkvInfoField->GetInt("DamageBothHands", 10) : 10);
	m_flAttackRange = (pkvInfoField ? pkvInfoField->GetFloat("AttackRange", 70.0f) : 70.0f);
	m_bCanOpenDoors = ((pkvInfoField && (pkvInfoField->GetInt("CanOpenDoors") >= 1)) ? true : false);

	if (bRandomModel)
	{
		const char* pRandomModel = GetRandomModel(pkvRandomModelsField);
		Q_strncpy(m_pchModelPath, (pRandomModel ? pRandomModel : ""), MAX_WEAPON_STRING);
	}
	else
		Q_strncpy(m_pchModelPath, (pkvModelField ? pkvModelField->GetString("Path") : ""), MAX_WEAPON_STRING);

	m_fIsTorso = ((pkvModelField && (pkvModelField->GetInt("IsTorso") >= 1)) ? true : false);

	if (pkvModelField && !strcmp(pkvModelField->GetString("Skin"), "random"))
		m_iSkin = random->RandomInt(0, pkvModelField->GetInt("MaxSkins"));
	else
		m_iSkin = (pkvModelField ? pkvModelField->GetInt("Skin") : 0);

	SetBloodColor(pkvModelField ? pkvModelField->GetInt("BloodType") : BLOOD_COLOR_RED);
	Precache();

	pkvMyNPCData->deleteThis();
}

void CNPC_Monster::Precache(void)
{
	PrecacheModel(m_pchModelPath);

	PrecacheScriptSound(UTIL_VarArgs("NPC_%s.Die", m_pchSoundScript));
	PrecacheScriptSound(UTIL_VarArgs("NPC_%s.Idle", m_pchSoundScript));
	PrecacheScriptSound(UTIL_VarArgs("NPC_%s.Pain", m_pchSoundScript));
	PrecacheScriptSound(UTIL_VarArgs("NPC_%s.Alert", m_pchSoundScript));
	PrecacheScriptSound(UTIL_VarArgs("NPC_%s.FootstepRight", m_pchSoundScript));
	PrecacheScriptSound(UTIL_VarArgs("NPC_%s.FootstepLeft", m_pchSoundScript));
	PrecacheScriptSound(UTIL_VarArgs("NPC_%s.Attack", m_pchSoundScript));

	PrecacheScriptSound(UTIL_VarArgs("NPC_%s.FastBreath", m_pchSoundScript));
	PrecacheScriptSound(UTIL_VarArgs("NPC_%s.Moan1", m_pchSoundScript));

	// Default Zombie Precaching
	PrecacheScriptSound("Zombie.FootstepRight");
	PrecacheScriptSound("Zombie.FootstepLeft");
	PrecacheScriptSound("Zombie.ScuffRight");
	PrecacheScriptSound("Zombie.ScuffLeft");
	PrecacheScriptSound("Zombie.Pain");
	PrecacheScriptSound("Zombie.Die");
	PrecacheScriptSound("Zombie.Alert");
	PrecacheScriptSound("Zombie.Idle");
	PrecacheScriptSound("Zombie.Attack");

	PrecacheScriptSound("NPC_BaseZombie.Moan1");
	PrecacheScriptSound("NPC_BaseZombie.Moan2");
	PrecacheScriptSound("NPC_BaseZombie.Moan3");
	PrecacheScriptSound("NPC_BaseZombie.Moan4");

	PrecacheScriptSound("Zombie.AttackHit");
	PrecacheScriptSound("Zombie.AttackMiss");

	BaseClass::Precache();
}

void CNPC_Monster::PrecacheInTemplate(void)
{
	ParseNPCScript(STRING(m_cScript));
	BaseClass::PrecacheInTemplate();
}

void CNPC_Monster::Spawn(void)
{
	ParseNPCScript(STRING(m_cScript));

	m_flLastBloodTrail = gpGlobals->curtime;
	m_flNextPainSoundTime = 0;

	AddSpawnFlags(SF_NPC_LONG_RANGE);
	m_flFieldOfView = 0.2;

	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_INNATE_MELEE_ATTACK1);

	BaseClass::Spawn();
}

const char* CNPC_Monster::GetMoanSound(int nSound)
{
	return UTIL_VarArgs("NPC_%s.Moan1", m_pchSoundScript);
}

Class_T	CNPC_Monster::Classify(void)
{
	return CLASS_ZOMBIE;
}

void CNPC_Monster::SetZombieModel(void)
{
	Hull_t lastHull = GetHullType();

	SetModel(m_pchModelPath);
	SetHullType(m_fIsTorso ? HULL_TINY : HULL_HUMAN);
	SetHullSizeNormal(true);
	SetDefaultEyeOffset();
	SetActivity(ACT_IDLE);

	m_nSkin = m_iSkin;

	if ((lastHull != GetHullType()) && VPhysicsGetObject())
		SetupVPhysicsHull();

	CollisionProp()->SetSurroundingBoundsType(USE_OBB_COLLISION_BOUNDS);
}

void CNPC_Monster::HandleAnimEvent(animevent_t* pEvent)
{
	QAngle pAng;
	Vector vecDir;

	if (pEvent->event == AE_ZOMBIE_ATTACK_RIGHT)
	{
		Vector right, forward;
		AngleVectors(GetLocalAngles(), &forward, &right, NULL);

		right = right * 100;
		forward = forward * 200;
		pAng = QAngle(-15, -20, -10);
		vecDir = (right + forward);
		ClawAttack(GetClawAttackRange(), m_iDamage[0], pAng, vecDir, ZOMBIE_BLOOD_RIGHT_HAND);
		return;
	}

	if (pEvent->event == AE_ZOMBIE_ATTACK_LEFT)
	{
		Vector right, forward;
		AngleVectors(GetLocalAngles(), &forward, &right, NULL);

		right = right * -100;
		forward = forward * 200;
		vecDir = (right + forward);
		pAng = QAngle(-15, 20, -10);
		ClawAttack(GetClawAttackRange(), m_iDamage[0], pAng, vecDir, ZOMBIE_BLOOD_LEFT_HAND);
		return;
	}

	if (pEvent->event == AE_ZOMBIE_ATTACK_BOTH)
	{
		Vector forward;
		QAngle qaPunch(45, random->RandomInt(-5, 5), random->RandomInt(-5, 5));
		AngleVectors(GetLocalAngles(), &forward);
		forward = forward * 200;
		ClawAttack(GetClawAttackRange(), m_iDamage[1], qaPunch, forward, ZOMBIE_BLOOD_BOTH_HANDS);
		return;
	}

	BaseClass::HandleAnimEvent(pEvent);
}

void CNPC_Monster::PrescheduleThink(void)
{
	bool bNearEnemy = false;
	if (GetEnemy() != NULL)
	{
		float flDist = (GetEnemy()->GetAbsOrigin() - GetAbsOrigin()).Length();
		if (flDist < ZOMBIE_ENEMY_BREATHE_DIST)
		{
			bNearEnemy = true;
		}
	}

	if (bNearEnemy)
	{
		if (!m_bNearEnemy)
		{
			m_bNearEnemy = true;
		}
	}
	else if (m_bNearEnemy)
	{
		m_bNearEnemy = false;
	}

	BaseClass::PrescheduleThink();

	// We're chopped off! And we're moving! Add some blood trail...
	if (m_fIsTorso && IsMoving() && (m_flLastBloodTrail < gpGlobals->curtime))
	{
		m_flLastBloodTrail = gpGlobals->curtime + 1.0f; // We don't want to spam blood all over the place.
		trace_t tr;
		UTIL_TraceLine((GetAbsOrigin() + Vector(0, 0, 50)), (GetAbsOrigin() + Vector(0, 0, -300)), MASK_ALL, this, COLLISION_GROUP_NONE, &tr);
		UTIL_DecalTrace(&tr, "Blood");
	}
}

void CNPC_Monster::BuildScheduleTestBits(void)
{
	BaseClass::BuildScheduleTestBits();

	if (IsCurSchedule(SCHED_CHASE_ENEMY))
	{
		SetCustomInterruptCondition(COND_LIGHT_DAMAGE);
		SetCustomInterruptCondition(COND_HEAVY_DAMAGE);
	}
}

int CNPC_Monster::SelectSchedule(void)
{
	int nSchedule = BaseClass::SelectSchedule();

	if (nSchedule == SCHED_SMALL_FLINCH)
	{
		m_flNextFlinchTime = gpGlobals->curtime + random->RandomFloat(1, 3);
	}

	return nSchedule;
}

int CNPC_Monster::TranslateSchedule(int scheduleType)
{
	if (scheduleType == SCHED_COMBAT_FACE && IsUnreachable(GetEnemy()))
		return SCHED_TAKE_COVER_FROM_ENEMY;

	return BaseClass::TranslateSchedule(scheduleType);
}

bool CNPC_Monster::ShouldPlayIdleSound(void)
{
	return CAI_BaseNPC::ShouldPlayIdleSound();
}

void CNPC_Monster::AttackHitSound(void)
{
	EmitSound("Zombie.AttackHit");
}

void CNPC_Monster::AttackMissSound(void)
{
	EmitSound("Zombie.AttackMiss");
}

void CNPC_Monster::AttackSound(void)
{
	EmitSound(UTIL_VarArgs("NPC_%s.Attack", m_pchSoundScript));
}

void CNPC_Monster::IdleSound(void)
{
	if (m_NPCState != NPC_STATE_COMBAT)
		EmitSound(UTIL_VarArgs("NPC_%s.Idle", m_pchSoundScript));
}

void CNPC_Monster::PainSound(const CTakeDamageInfo& info)
{
	// Don't make pain sounds too often.
	if (m_flNextPainSoundTime <= gpGlobals->curtime)
	{
		EmitSound(UTIL_VarArgs("NPC_%s.Pain", m_pchSoundScript));
		m_flNextPainSoundTime = gpGlobals->curtime + random->RandomFloat(4.0, 7.0);
	}
}

void CNPC_Monster::DeathSound(const CTakeDamageInfo& info)
{
	EmitSound(UTIL_VarArgs("NPC_%s.Die", m_pchSoundScript));
}

void CNPC_Monster::AlertSound(void)
{
	EmitSound(UTIL_VarArgs("NPC_%s.Alert", m_pchSoundScript));
}

void CNPC_Monster::FootstepSound(bool fRightFoot)
{
	EmitSound(UTIL_VarArgs("NPC_%s.%s", m_pchSoundScript, (fRightFoot ? "FootstepRight" : "FootstepLeft")));

	if (ShouldPlayFootstepMoan())
		m_flNextMoanSound = gpGlobals->curtime;
}

bool CNPC_Monster::MustCloseToAttack(void)
{
	return true;
}

bool CNPC_Monster::ShouldBecomeTorso(const CTakeDamageInfo& info, float flDamageThreshold)
{
	return false;
}

int ACT_ZOMBIE_MONSTER_THREAT;

AI_BEGIN_CUSTOM_NPC(npc_monster, CNPC_Monster)
DECLARE_ACTIVITY(ACT_ZOMBIE_MONSTER_THREAT)
AI_END_CUSTOM_NPC()