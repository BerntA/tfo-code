//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Custom Soldier NPC - Enemy. Gangsters, etc...
//
//=============================================================================//

#include "cbase.h"
#include "ai_hull.h"
#include "ai_motor.h"
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
#include "weapon_physcannon.h"
#include "hl2_gamerules.h"
#include "gameweaponmanager.h"
#include "filesystem.h"
#include "GameBase_Server.h"
#include <KeyValues.h>
#include "vehicle_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( npc_soldier, CNPC_Soldier );

#define AE_SOLDIER_BLOCK_PHYSICS		20 // trying to block an incoming physics object

// Force Script:
void CNPC_Soldier::SetScript(const char *szScript)
{
	cScript = MAKE_STRING(szScript);
}

//-----------------------------------------------------------------------------
// Purpose: Return data about the npc, if found.
//-----------------------------------------------------------------------------
KeyValues *CNPC_Soldier::pkvNPCData( const char *szScript )
{
	KeyValues *pkvData = new KeyValues( "NPCDATA" );
	if ( pkvData->LoadFromFile( filesystem, UTIL_VarArgs( "resource/data/npcs/%s.txt", szScript ), "MOD" ) )
	{
		return pkvData;
	}

	pkvData->deleteThis();

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Return a random model in the Models key.
//-----------------------------------------------------------------------------
const char *CNPC_Soldier::GetRandomModel(KeyValues *pkvValues)
{
	const char *szModel = NULL;
	int iCount = 0, iFind = 0;

	for (KeyValues *sub = pkvValues->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		iCount++;

	iFind = random->RandomInt(1, iCount);
	iCount = 0;

	for (KeyValues *sub = pkvValues->GetFirstSubKey(); sub; sub = sub->GetNextKey())
	{
		iCount++;
		if (iCount == iFind)
		{
			szModel = ReadAndAllocStringValue(pkvValues, sub->GetName());
			break;
		}
	}

	return szModel;
}

//-----------------------------------------------------------------------------
// Purpose: Parse data found from npcData keyValue...
//-----------------------------------------------------------------------------
void CNPC_Soldier::ParseNPCScript( const char *szScript )
{
	// Get our data and make sure it is not NULL...
	KeyValues *pkvMyNPCData = pkvNPCData(szScript);
	if (!pkvMyNPCData)
	{
		Warning("NPC_SOLDIER linked to %s.txt was removed, no such script exist!\n", szScript);
		UTIL_Remove(this);
		return;
	}

	// Parse our data:
	KeyValues *pkvInfoField = pkvMyNPCData->FindKey("Info");
	KeyValues *pkvModelField = pkvMyNPCData->FindKey("Model");
	KeyValues *pkvRandomModelsField = pkvMyNPCData->FindKey("Models");

	bool bRandomModel = false;
	int iSkin = 0;

	if (pkvInfoField)
	{
		cEntName = MAKE_STRING(ReadAndAllocStringValue(pkvInfoField, "Name"));
		int iHealth = pkvInfoField->GetInt("Health", 100);
		m_iDamage = pkvInfoField->GetInt("MeleeDamage", 20);

		SetHealth(iHealth);
		SetMaxHealth(iHealth);
		SetKickDamage(m_iDamage);
		m_iMaxHealth = iHealth;

		m_bIsFriendly = (pkvInfoField->GetInt("IsFriendly") >= 1) ? true : false;
	}

	if (pkvModelField)
	{
		bRandomModel = (!pkvModelField->FindKey("Path"));

		if (!bRandomModel)
			cModel = MAKE_STRING(ReadAndAllocStringValue(pkvModelField, "Path"));

		if (!strcmp(pkvModelField->GetString("Skin"), "random"))
			iSkin = random->RandomInt(0, pkvModelField->GetInt("MaxSkins"));
		else
			iSkin = pkvModelField->GetInt("Skin");

		SetBloodColor(pkvModelField->GetInt("BloodType"));
	}

	if (pkvRandomModelsField && bRandomModel)
		cModel = MAKE_STRING(GetRandomModel(pkvRandomModelsField));

	Precache();
	SetModel(cModel.ToCStr());
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

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Soldier::Spawn( void )
{
	ParseNPCScript(cScript.ToCStr());

	CapabilitiesAdd( bits_CAP_ANIMATEDFACE );
	CapabilitiesAdd( bits_CAP_MOVE_SHOOT );
	CapabilitiesAdd( bits_CAP_DOORS_GROUP );
	CapabilitiesAdd(bits_CAP_MOVE_JUMP);

	m_bIsAngry = false;

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CNPC_Soldier::Precache()
{
	PrecacheModel( cModel.ToCStr() );

	UTIL_PrecacheOther("weapon_stiel");
	UTIL_PrecacheOther("stiel_ammo");

	BaseClass::Precache();
}

void CNPC_Soldier::PrecacheInTemplate(void)
{
	ParseNPCScript(STRING(cScript));
	BaseClass::PrecacheInTemplate();
}

void CNPC_Soldier::DeathSound( const CTakeDamageInfo &info )
{
	// NOTE: The response system deals with this at the moment
	if ( GetFlags() & FL_DISSOLVING )
		return;

	// On Death Achievements:
	if (!strcmp(cEntName.ToCStr(), "Schienzel"))
		GameBaseServer->SendAchievement("ACH_VENGEANCE");

	GetSentences()->Speak( "COMBINE_DIE", SENTENCE_PRIORITY_INVALID, SENTENCE_CRITERIA_ALWAYS ); 
}

//-----------------------------------------------------------------------------
// Purpose: Soldiers use CAN_RANGE_ATTACK2 to indicate whether they can throw
//			a grenade. Because they check only every half-second or so, this
//			condition must persist until it is updated again by the code
//			that determines whether a grenade can be thrown, so prevent the 
//			base class from clearing it out. (sjb)
//-----------------------------------------------------------------------------
void CNPC_Soldier::ClearAttackConditions( )
{
	bool fCanRangeAttack2 = HasCondition( COND_CAN_RANGE_ATTACK2 );

	// Call the base class.
	BaseClass::ClearAttackConditions();

	if( fCanRangeAttack2 )
	{
		// We don't allow the base class to clear this condition because we
		// don't sense for it every frame.
		SetCondition( COND_CAN_RANGE_ATTACK2 );
	}
}

void CNPC_Soldier::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();
}

//-----------------------------------------------------------------------------
// Purpose: Allows for modification of the interrupt mask for the current schedule.
//			In the most cases the base implementation should be called first.
//-----------------------------------------------------------------------------
void CNPC_Soldier::BuildScheduleTestBits( void )
{
	//Interrupt any schedule with physics danger (as long as I'm not moving or already trying to block)
	if ( m_flGroundSpeed == 0.0 && !IsCurSchedule( SCHED_FLINCH_PHYSICS ) )
	{
		SetCustomInterruptCondition( COND_HEAR_PHYSICS_DANGER );
	}

	BaseClass::BuildScheduleTestBits();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CNPC_Soldier::SelectSchedule ( void )
{
	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float CNPC_Soldier::GetHitgroupDamageMultiplier( int iHitGroup, const CTakeDamageInfo &info )
{
	switch( iHitGroup )
	{
	case HITGROUP_HEAD:
		{
			// Soldiers take double headshot damage
			return 2.0f;
		}
	}

	return BaseClass::GetHitgroupDamageMultiplier( iHitGroup, info );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Soldier::HandleAnimEvent( animevent_t *pEvent )
{
	switch( pEvent->event )
	{
	case AE_SOLDIER_BLOCK_PHYSICS:
		DevMsg( "BLOCKING!\n" );
		m_fIsBlocking = true;
		break;

	default:
		BaseClass::HandleAnimEvent( pEvent );
		break;
	}
}

void CNPC_Soldier::OnChangeActivity( Activity eNewActivity )
{
	// Any new sequence stops us blocking.
	m_fIsBlocking = false;

	BaseClass::OnChangeActivity( eNewActivity );
}

void CNPC_Soldier::OnListened()
{
	BaseClass::OnListened();

	if ( HasCondition( COND_HEAR_DANGER ) && HasCondition( COND_HEAR_PHYSICS_DANGER ) )
	{
		if ( HasInterruptCondition( COND_HEAR_DANGER ) )
		{
			ClearCondition( COND_HEAR_PHYSICS_DANGER );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputInfo - 
// Output : int
//-----------------------------------------------------------------------------
int CNPC_Soldier::OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo)
{
	int iTookDamage = BaseClass::OnTakeDamage_Alive(inputInfo);

	// If a friend attacks us we'll go against them.
	CBaseEntity *pAttacker = inputInfo.GetAttacker();
	if (pAttacker)
	{
		if (iTookDamage >= 1 && m_bIsFriendly && pAttacker->IsPlayer() && !m_bIsAngry)
			m_bIsAngry = true;
	}

	return iTookDamage;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
void CNPC_Soldier::Event_Killed( const CTakeDamageInfo &info )
{
	CBasePlayer *pPlayer = ToBasePlayer( info.GetAttacker() );
	if ( !pPlayer )
	{
		CPropVehicleDriveable *pVehicle = dynamic_cast<CPropVehicleDriveable *>( info.GetAttacker() ) ;
		if ( pVehicle && pVehicle->GetDriver() && pVehicle->GetDriver()->IsPlayer() )
		{
			pPlayer = assert_cast<CBasePlayer *>( pVehicle->GetDriver() );
		}
	}

	if ( pPlayer != NULL )
	{
		CHalfLife2 *pHL2GameRules = static_cast<CHalfLife2 *>(g_pGameRules);

		if ( HasSpawnFlags( SF_COMBINE_NO_GRENADEDROP ) == false )
		{
			// Attempt to drop a grenade
			if ( pHL2GameRules->NPC_ShouldDropGrenade( pPlayer ) )
			{
				DropItem( "stiel_ammo", WorldSpaceCenter()+RandomVector(-4,4), RandomAngle(0,360) );
				pHL2GameRules->NPC_DroppedGrenade();
			}
		}
	}

	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_Soldier::IsLightDamage( const CTakeDamageInfo &info )
{
	return BaseClass::IsLightDamage( info );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_Soldier::IsHeavyDamage( const CTakeDamageInfo &info )
{
	if ( info.GetAmmoType() == GetAmmoDef()->Index("G43") )
		return true;

	if ( info.GetAmmoType() == GetAmmoDef()->Index("K98") )
		return true;

	if ( info.GetAmmoType() == GetAmmoDef()->Index("SVT40") )
		return true;

	// Rollermine shocks
	if( (info.GetDamageType() & DMG_SHOCK) && hl2_episodic.GetBool() )
	{
		return true;
	}

	return BaseClass::IsHeavyDamage( info );
}

#if HL2_EPISODIC
//-----------------------------------------------------------------------------
// Purpose: Translate base class activities into combot activites
//-----------------------------------------------------------------------------
Activity CNPC_Soldier::NPC_TranslateActivity( Activity eNewActivity )
{
	return BaseClass::NPC_TranslateActivity( eNewActivity );
}

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_Soldier )

	DEFINE_KEYFIELD( cScript, FIELD_STRING, "Script" ),
	DEFINE_FIELD(m_iMaxHealth, FIELD_INTEGER),
	DEFINE_FIELD(m_iDamage, FIELD_INTEGER),
	DEFINE_FIELD(cEntName, FIELD_STRING),
	DEFINE_FIELD(cModel, FIELD_STRING),
	DEFINE_FIELD(m_bIsFriendly, FIELD_BOOLEAN),
	DEFINE_FIELD(m_bIsAngry, FIELD_BOOLEAN),

END_DATADESC()
#endif