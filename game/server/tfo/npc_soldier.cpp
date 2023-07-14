//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Custom Soldier NPC - Enemy. Gangsters, etc...
//
//=============================================================================//

#include "cbase.h"
#include "ai_hull.h"
#include "ai_motor.h"
#include "ai_squad.h"
#include "npc_soldier.h"
#include "bitstring.h"
#include "engine/IEngineSound.h"
#include "soundent.h"
#include "ndebugoverlay.h"
#include "npcevent.h"
#include "hl2/hl2_player.h"
#include "game.h"
#include "ammodef.h"
#include "explode.h"
#include "ai_memory.h"
#include "Sprite.h"
#include "soundenvelope.h"
#include "hl2_gamerules.h"
#include "gameweaponmanager.h"
#include "filesystem.h"
#include "achievement_manager.h"
#include <KeyValues.h>
#include "vehicle_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(npc_soldier, CNPC_Soldier);

void CNPC_Soldier::SetScript(const char* szScript)
{
	m_cScript = MAKE_STRING(szScript);
}

KeyValues* CNPC_Soldier::LoadNPCData(const char* szScript)
{
	KeyValues* pkvData = new KeyValues("NPCDATA");
	if (pkvData->LoadFromFile(filesystem, UTIL_VarArgs("data/npcs/%s.txt", szScript), "MOD"))
		return pkvData;

	pkvData->deleteThis();
	return NULL;
}

const char* CNPC_Soldier::GetRandomModel(KeyValues* pkvValues)
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

void CNPC_Soldier::ParseNPCScript(const char* szScript)
{
	KeyValues* pkvMyNPCData = LoadNPCData(szScript);
	if (!pkvMyNPCData)
	{
		Warning("NPC_SOLDIER linked to %s.txt was removed, no such script exist!\n", szScript);
		UTIL_Remove(this);
		return;
	}

	KeyValues* pkvInfoField = pkvMyNPCData->FindKey("Info");
	KeyValues* pkvModelField = pkvMyNPCData->FindKey("Model");
	KeyValues* pkvRandomModelsField = pkvMyNPCData->FindKey("Models");

	const bool bRandomModel = (pkvRandomModelsField && pkvModelField && !pkvModelField->FindKey("Path"));
	const int iHealth = (pkvInfoField ? pkvInfoField->GetInt("Health", 100) : 100);
	int iSkin = 0;

	Q_strncpy(m_szEntName, (pkvInfoField ? pkvInfoField->GetString("Name") : ""), MAX_NPC_SCRIPT_NAME);

	SetHealth(iHealth);
	SetMaxHealth(iHealth);
	SetKickDamage(pkvInfoField ? pkvInfoField->GetInt("MeleeDamage", 20) : 20);
	SetNumGrenades(pkvInfoField ? pkvInfoField->GetInt("NumGrenades") : 0);
	SetBloodColor(pkvModelField ? pkvModelField->GetInt("BloodType") : BLOOD_COLOR_RED);

	m_bIsFriendly = (pkvInfoField && (pkvInfoField->GetInt("IsFriendly") >= 1)) ? true : false;
	m_bNoPushback = (pkvInfoField && (pkvInfoField->GetInt("NoPushback") >= 1)) ? true : false;

	char pchModelPath[MAX_WEAPON_STRING]; pchModelPath[0] = 0;

	if (bRandomModel)
	{
		const char* pRandomModel = GetRandomModel(pkvRandomModelsField);
		Q_strncpy(pchModelPath, (pRandomModel ? pRandomModel : ""), MAX_WEAPON_STRING);
	}
	else
		Q_strncpy(pchModelPath, (pkvModelField ? pkvModelField->GetString("Path") : ""), MAX_WEAPON_STRING);

	Precache();
	PrecacheModel(pchModelPath);
	SetModel(pchModelPath);

	if (pkvModelField && !strcmp(pkvModelField->GetString("Skin"), "random"))
		iSkin = random->RandomInt(0, pkvModelField->GetInt("MaxSkins"));
	else if (pkvModelField)
		iSkin = pkvModelField->GetInt("Skin");

	m_nSkin = iSkin;

	pkvMyNPCData->deleteThis();
}

Class_T	CNPC_Soldier::Classify(void)
{
	if (m_bIsAngry)
		return CLASS_SOLDIER_ANGRY;
	else if (m_bIsFriendly)
		return CLASS_CITIZEN_REBEL;

	return BaseClass::Classify();
}

void CNPC_Soldier::Spawn(void)
{
	ParseNPCScript(STRING(m_cScript));

	CapabilitiesAdd(bits_CAP_ANIMATEDFACE);
	CapabilitiesAdd(bits_CAP_MOVE_SHOOT);
	CapabilitiesAdd(bits_CAP_DOORS_GROUP);
	CapabilitiesAdd(bits_CAP_MOVE_JUMP);

	m_bIsAngry = false;

	BaseClass::Spawn();
}

void CNPC_Soldier::Precache()
{
	UTIL_PrecacheOther("weapon_stiel");
	UTIL_PrecacheOther("stiel_ammo");
	BaseClass::Precache();
}

void CNPC_Soldier::PrecacheInTemplate(void)
{
	ParseNPCScript(STRING(m_cScript));
	BaseClass::PrecacheInTemplate();
}

void CNPC_Soldier::DeathSound(const CTakeDamageInfo& info)
{
	// NOTE: The response system deals with this at the moment
	if (GetFlags() & FL_DISSOLVING)
		return;

	// On Death Achievements:
	if (!strcmp(m_szEntName, "Schienzel"))
		CAchievementManager::SendAchievement("ACH_VENGEANCE");

	GetSentences()->Speak("COMBINE_DIE", SENTENCE_PRIORITY_INVALID, SENTENCE_CRITERIA_ALWAYS);
}

void CNPC_Soldier::ClearAttackConditions()
{
	bool fCanRangeAttack2 = HasCondition(COND_CAN_RANGE_ATTACK2);

	// Call the base class.
	BaseClass::ClearAttackConditions();

	if (fCanRangeAttack2)
	{
		// We don't allow the base class to clear this condition because we
		// don't sense for it every frame.
		SetCondition(COND_CAN_RANGE_ATTACK2);
	}
}

void CNPC_Soldier::BuildScheduleTestBits(void)
{
	//Interrupt any schedule with physics danger (as long as I'm not moving or already trying to block)
	if (m_flGroundSpeed == 0.0 && !IsCurSchedule(SCHED_FLINCH_PHYSICS))
	{
		SetCustomInterruptCondition(COND_HEAR_PHYSICS_DANGER);
	}

	BaseClass::BuildScheduleTestBits();
}

void CNPC_Soldier::OnListened()
{
	BaseClass::OnListened();

	if (HasCondition(COND_HEAR_DANGER) && HasCondition(COND_HEAR_PHYSICS_DANGER))
	{
		if (HasInterruptCondition(COND_HEAR_DANGER))
		{
			ClearCondition(COND_HEAR_PHYSICS_DANGER);
		}
	}
}

int CNPC_Soldier::OnTakeDamage_Alive(const CTakeDamageInfo& inputInfo)
{
	int iTookDamage = BaseClass::OnTakeDamage_Alive(inputInfo);

	// If a friend attacks us we'll go against them.
	CBaseEntity* pAttacker = inputInfo.GetAttacker();
	if (pAttacker)
	{
		if (iTookDamage >= 1 && m_bIsFriendly && pAttacker->IsPlayer() && !m_bIsAngry)
			m_bIsAngry = true;
	}

	return iTookDamage;
}

void CNPC_Soldier::Event_Killed(const CTakeDamageInfo& info)
{
	CBasePlayer* pPlayer = ToBasePlayer(info.GetAttacker());
	if (!pPlayer)
	{
		CPropVehicleDriveable* pVehicle = dynamic_cast<CPropVehicleDriveable*>(info.GetAttacker());
		if (pVehicle && pVehicle->GetDriver() && pVehicle->GetDriver()->IsPlayer())
		{
			pPlayer = assert_cast<CBasePlayer*>(pVehicle->GetDriver());
		}
	}

	if (pPlayer != NULL)
	{
		CHalfLife2* pHL2GameRules = static_cast<CHalfLife2*>(g_pGameRules);

		if (HasSpawnFlags(SF_COMBINE_NO_GRENADEDROP) == false)
		{
			// Attempt to drop a grenade
			if (pHL2GameRules->NPC_ShouldDropGrenade(pPlayer))
			{
				DropItem("stiel_ammo", WorldSpaceCenter() + RandomVector(-4, 4), RandomAngle(0, 360));
				pHL2GameRules->NPC_DroppedGrenade();
			}
		}
	}

	BaseClass::Event_Killed(info);
}

bool CNPC_Soldier::IsLightDamage(const CTakeDamageInfo& info)
{
	return BaseClass::IsLightDamage(info);
}

bool CNPC_Soldier::IsHeavyDamage(const CTakeDamageInfo& info)
{
	if ((info.GetAmmoType() == GetAmmoDef()->Index("G43")) || (info.GetAmmoType() == GetAmmoDef()->Index("K98")) || (info.GetAmmoType() == GetAmmoDef()->Index("SVT40")))
		return true;

	return BaseClass::IsHeavyDamage(info);
}

BEGIN_DATADESC(CNPC_Soldier)
DEFINE_KEYFIELD(m_cScript, FIELD_STRING, "Script"),
DEFINE_FIELD(m_bIsFriendly, FIELD_BOOLEAN),
DEFINE_FIELD(m_bIsAngry, FIELD_BOOLEAN),
DEFINE_ARRAY(m_szEntName, FIELD_CHARACTER, MAX_NPC_SCRIPT_NAME),
END_DATADESC()

AI_BEGIN_CUSTOM_NPC(npc_soldier, CNPC_Soldier)
AI_END_CUSTOM_NPC()