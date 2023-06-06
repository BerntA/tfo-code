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

static const char *pMoanSounds[] =
{
	"Moan1",
};

LINK_ENTITY_TO_CLASS( npc_monster, CNPC_Monster );

BEGIN_DATADESC( CNPC_Monster )

	DEFINE_FIELD( m_flNextPainSoundTime, FIELD_TIME ),
	DEFINE_FIELD(m_bCanOpenDoors, FIELD_BOOLEAN),
	DEFINE_FIELD(m_iMaxHealth, FIELD_INTEGER),
	DEFINE_FIELD(m_iSkin, FIELD_INTEGER),
	DEFINE_FIELD(m_iDamage[0], FIELD_INTEGER),
	DEFINE_FIELD(m_iDamage[1], FIELD_INTEGER),
	DEFINE_FIELD(m_flAttackRange, FIELD_FLOAT),
	DEFINE_FIELD(cModel, FIELD_STRING),
	DEFINE_FIELD(cEntName, FIELD_STRING),
	DEFINE_FIELD(cSoundScript, FIELD_STRING),
	DEFINE_FIELD( m_bNearEnemy, FIELD_BOOLEAN ),
	DEFINE_KEYFIELD( cScript, FIELD_STRING, "Script" ),

END_DATADESC()

// Force Script:
void CNPC_Monster::SetScript( const char *szScript )
{
	cScript = MAKE_STRING(szScript);
}

//-----------------------------------------------------------------------------
// Purpose: Return a random model in the Models key.
//-----------------------------------------------------------------------------
const char *CNPC_Monster::GetRandomModel(KeyValues *pkvValues)
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
// Purpose: Return data about the npc, if found.
//-----------------------------------------------------------------------------
KeyValues *CNPC_Monster::pkvNPCData( const char *szScript )
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
// Purpose: Parse data found from npcData keyValue...
//-----------------------------------------------------------------------------
void CNPC_Monster::ParseNPCScript( const char *szScript )
{
	// Get our data and make sure it is not NULL...
	KeyValues *pkvMyNPCData = pkvNPCData(szScript);
	if (!pkvMyNPCData)
	{
		Warning("NPC_MONSTER linked to %s.txt was removed, no such script exist!\n", szScript);
		UTIL_Remove(this);
		return;
	}

	// Parse our data:
	KeyValues *pkvInfoField = pkvMyNPCData->FindKey("Info");
	KeyValues *pkvModelField = pkvMyNPCData->FindKey("Model");
	KeyValues *pkvRandomModelsField = pkvMyNPCData->FindKey("Models");

	bool bRandomModel = false;

	if (pkvInfoField)
	{
		cEntName = MAKE_STRING(ReadAndAllocStringValue(pkvInfoField, "Name"));
		cSoundScript = MAKE_STRING(ReadAndAllocStringValue(pkvInfoField, "SoundScript"));

		m_iHealth = pkvInfoField->GetInt("Health", 100);
		m_iDamage[0] = pkvInfoField->GetInt("DamageOneHand", 10);
		m_iDamage[1] = pkvInfoField->GetInt("DamageBothHands", 10);
		m_flAttackRange = pkvInfoField->GetFloat("AttackRange", 70.0f);
		m_iMaxHealth = m_iHealth;
		m_bCanOpenDoors = ((pkvInfoField->GetInt("CanOpenDoors") >= 1) ? true : false);
	}

	if (pkvModelField)
	{
		bRandomModel = (!pkvModelField->FindKey("Path"));

		if (!bRandomModel)
			cModel = MAKE_STRING(ReadAndAllocStringValue(pkvModelField, "Path"));

		m_fIsTorso = ((pkvModelField->GetInt("IsTorso") >= 1) ? true : false);

		if (!strcmp(pkvModelField->GetString("Skin"), "random"))
			m_iSkin = random->RandomInt(0, pkvModelField->GetInt("MaxSkins"));
		else
			m_iSkin = pkvModelField->GetInt("Skin");

		SetBloodColor(pkvModelField->GetInt("BloodType"));
	}

	if (pkvRandomModelsField && bRandomModel)
		cModel = MAKE_STRING(GetRandomModel(pkvRandomModelsField));

	Precache();

	pkvMyNPCData->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Monster::Precache( void )
{
	PrecacheModel( cModel.ToCStr() );

	PrecacheScriptSound( UTIL_VarArgs( "NPC_%s.Die", cSoundScript.ToCStr() ) );
	PrecacheScriptSound(UTIL_VarArgs("NPC_%s.Idle", cSoundScript.ToCStr()));
	PrecacheScriptSound(UTIL_VarArgs("NPC_%s.Pain", cSoundScript.ToCStr()));
	PrecacheScriptSound(UTIL_VarArgs("NPC_%s.Alert", cSoundScript.ToCStr()));
	PrecacheScriptSound(UTIL_VarArgs("NPC_%s.FootstepRight", cSoundScript.ToCStr()));
	PrecacheScriptSound(UTIL_VarArgs("NPC_%s.FootstepLeft", cSoundScript.ToCStr()));
	PrecacheScriptSound(UTIL_VarArgs("NPC_%s.Attack", cSoundScript.ToCStr()));

	PrecacheScriptSound(UTIL_VarArgs("NPC_%s.FastBreath", cSoundScript.ToCStr()));
	PrecacheScriptSound(UTIL_VarArgs("NPC_%s.Moan1", cSoundScript.ToCStr()));

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

	PrecacheScriptSound( "Zombie.AttackHit" );
	PrecacheScriptSound( "Zombie.AttackMiss" );

	BaseClass::Precache();
}

void CNPC_Monster::PrecacheInTemplate(void)
{
	ParseNPCScript(STRING(cScript));
	BaseClass::PrecacheInTemplate();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Monster::Spawn( void )
{
	ParseNPCScript( cScript.ToCStr() );

	m_flLastBloodTrail = gpGlobals->curtime;

	m_flNextPainSoundTime = 0;

	m_fIsHeadless = true;

	AddSpawnFlags( SF_NPC_LONG_RANGE );
	m_flFieldOfView = 0.2;

	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_INNATE_MELEE_ATTACK1 );

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: Returns a moan sound for this class of zombie.
//-----------------------------------------------------------------------------
const char *CNPC_Monster::GetMoanSound( int nSound )
{
	return UTIL_VarArgs("NPC_%s.%s", cSoundScript.ToCStr(), pMoanSounds[nSound % ARRAYSIZE(pMoanSounds)]);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Monster::StopLoopingSounds( void )
{
	BaseClass::StopLoopingSounds();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : info - 
//-----------------------------------------------------------------------------
void CNPC_Monster::Event_Killed( const CTakeDamageInfo &info )
{
	BaseClass::Event_Killed( info );
}

void CNPC_Monster::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator )
{
	BaseClass::TraceAttack( info, vecDir, ptr, pAccumulator );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputInfo - 
// Output : int
//-----------------------------------------------------------------------------
int CNPC_Monster::OnTakeDamage_Alive( const CTakeDamageInfo &inputInfo )
{
	return BaseClass::OnTakeDamage_Alive( inputInfo );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CNPC_Monster::MaxYawSpeed( void )
{
	return BaseClass::MaxYawSpeed();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Class_T	CNPC_Monster::Classify( void )
{
	return CLASS_ZOMBIE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//
// NOTE: This function is still heavy with common code (found at the bottom).
//		 we should consider moving some into the base class! (sjb)
//-----------------------------------------------------------------------------
void CNPC_Monster::SetZombieModel( void )
{
	Hull_t lastHull = GetHullType();

	SetModel( cModel.ToCStr() );

	if (m_fIsTorso)
		SetHullType(HULL_TINY);
	else
		SetHullType(HULL_HUMAN);

	SetHullSizeNormal( true );
	SetDefaultEyeOffset();
	SetActivity( ACT_IDLE );

	m_nSkin = m_iSkin;

	if ( lastHull != GetHullType() )
	{
		if ( VPhysicsGetObject() )
		{
			SetupVPhysicsHull();
		}
	}

	CollisionProp()->SetSurroundingBoundsType( USE_OBB_COLLISION_BOUNDS );
}

//-----------------------------------------------------------------------------
// Purpose: Turns off our breath so we can play another vocal sound.
//			TODO: pass in duration
//-----------------------------------------------------------------------------
void CNPC_Monster::BreatheOffShort( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Catches the monster-specific events that occur when tagged animation
//			frames are played.
// Input  : pEvent - 
//-----------------------------------------------------------------------------
void CNPC_Monster::HandleAnimEvent( animevent_t *pEvent )
{
	QAngle pAng;
	Vector vecDir;

	if ( pEvent->event == AE_ZOMBIE_ATTACK_RIGHT ) 
	{
		Vector right, forward;
		AngleVectors( GetLocalAngles(), &forward, &right, NULL );

		right = right * 100;
		forward = forward * 200;
		pAng = QAngle(-15, -20, -10);
		vecDir = (right + forward);
		ClawAttack(GetClawAttackRange(), m_iDamage[0], pAng, vecDir, ZOMBIE_BLOOD_RIGHT_HAND);
		return;
	}

	if ( pEvent->event == AE_ZOMBIE_ATTACK_LEFT )
	{
		Vector right, forward;
		AngleVectors( GetLocalAngles(), &forward, &right, NULL );

		right = right * -100;
		forward = forward * 200;
		vecDir = (right + forward);
		pAng = QAngle(-15, 20, -10);
		ClawAttack(GetClawAttackRange(), m_iDamage[0], pAng, vecDir, ZOMBIE_BLOOD_LEFT_HAND);
		return;
	}

	if ( pEvent->event == AE_ZOMBIE_ATTACK_BOTH )
	{
		Vector forward;
		QAngle qaPunch( 45, random->RandomInt(-5,5), random->RandomInt(-5,5) );
		AngleVectors( GetLocalAngles(), &forward );
		forward = forward * 200;
		ClawAttack( GetClawAttackRange(), m_iDamage[1], qaPunch, forward, ZOMBIE_BLOOD_BOTH_HANDS );
		return;
	}

	BaseClass::HandleAnimEvent( pEvent );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Monster::PrescheduleThink( void )
{
	bool bNearEnemy = false;
	if ( GetEnemy() != NULL )
	{
		float flDist = (GetEnemy()->GetAbsOrigin() - GetAbsOrigin()).Length();
		if ( flDist < ZOMBIE_ENEMY_BREATHE_DIST )
		{
			bNearEnemy = true;
		}
	}

	if ( bNearEnemy )
	{
		if ( !m_bNearEnemy )
		{
			m_bNearEnemy = true;
		}
	}
	else if ( m_bNearEnemy )
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


//-----------------------------------------------------------------------------
// Purpose: Allows for modification of the interrupt mask for the current schedule.
//			In the most cases the base implementation should be called first.
//-----------------------------------------------------------------------------
void CNPC_Monster::BuildScheduleTestBits( void )
{
	BaseClass::BuildScheduleTestBits();

	if ( IsCurSchedule( SCHED_CHASE_ENEMY ) )
	{
		SetCustomInterruptCondition( COND_LIGHT_DAMAGE );
		SetCustomInterruptCondition( COND_HEAVY_DAMAGE );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CNPC_Monster::SelectFailSchedule( int nFailedSchedule, int nFailedTask, AI_TaskFailureCode_t eTaskFailCode )
{
	int nSchedule = BaseClass::SelectFailSchedule( nFailedSchedule, nFailedTask, eTaskFailCode );

	return nSchedule;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
int CNPC_Monster::SelectSchedule( void )
{
	int nSchedule = BaseClass::SelectSchedule();

	if ( nSchedule == SCHED_SMALL_FLINCH )
	{
		m_flNextFlinchTime = gpGlobals->curtime + random->RandomFloat( 1, 3 );
	}

	return nSchedule;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : scheduleType - 
// Output : int
//-----------------------------------------------------------------------------
int CNPC_Monster::TranslateSchedule( int scheduleType )
{
	if ( scheduleType == SCHED_COMBAT_FACE && IsUnreachable( GetEnemy() ) )
		return SCHED_TAKE_COVER_FROM_ENEMY;

	return BaseClass::TranslateSchedule( scheduleType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CNPC_Monster::ShouldPlayIdleSound( void )
{
	return CAI_BaseNPC::ShouldPlayIdleSound();
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack hit sound
//-----------------------------------------------------------------------------
void CNPC_Monster::AttackHitSound( void )
{
	EmitSound( "Zombie.AttackHit" );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack miss sound
//-----------------------------------------------------------------------------
void CNPC_Monster::AttackMissSound( void )
{
	EmitSound( "Zombie.AttackMiss" );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack sound.
//-----------------------------------------------------------------------------
void CNPC_Monster::AttackSound( void )
{
	EmitSound(UTIL_VarArgs("NPC_%s.Attack", cSoundScript.ToCStr()));
}

//-----------------------------------------------------------------------------
// Purpose: Play a random idle sound.
//-----------------------------------------------------------------------------
void CNPC_Monster::IdleSound( void )
{
	// HACK: base zombie code calls IdleSound even when not idle!
	if ( m_NPCState != NPC_STATE_COMBAT )
	{
		BreatheOffShort();
		EmitSound(UTIL_VarArgs("NPC_%s.Idle", cSoundScript.ToCStr()));
	}
}

//-----------------------------------------------------------------------------
// Purpose: Play a random pain sound.
//-----------------------------------------------------------------------------
void CNPC_Monster::PainSound( const CTakeDamageInfo &info )
{
	// Don't make pain sounds too often.
	if ( m_flNextPainSoundTime <= gpGlobals->curtime )
	{	
		//BreatheOffShort();
		EmitSound(UTIL_VarArgs("NPC_%s.Pain", cSoundScript.ToCStr()));
		m_flNextPainSoundTime = gpGlobals->curtime + random->RandomFloat( 4.0, 7.0 );
	}
}

void CNPC_Monster::DeathSound(const CTakeDamageInfo &info )
{
	if ( !( info.GetDamageType() & ( DMG_BLAST | DMG_ALWAYSGIB) ) ) 
	{
		EmitSound(UTIL_VarArgs("NPC_%s.Die", cSoundScript.ToCStr()));
	}
}

//-----------------------------------------------------------------------------
// Purpose: Play a random alert sound.
//-----------------------------------------------------------------------------
void CNPC_Monster::AlertSound( void )
{
	BreatheOffShort();

	EmitSound(UTIL_VarArgs("NPC_%s.Alert", cSoundScript.ToCStr()));
}


//-----------------------------------------------------------------------------
// Purpose: Sound of a footstep
//-----------------------------------------------------------------------------
void CNPC_Monster::FootstepSound( bool fRightFoot )
{
	if( fRightFoot )
	{
		EmitSound(UTIL_VarArgs("NPC_%s.FootstepRight", cSoundScript.ToCStr()));
	}
	else
	{
		EmitSound(UTIL_VarArgs("NPC_%s.FootstepLeft", cSoundScript.ToCStr()));
	}

	if( ShouldPlayFootstepMoan() )
	{
		m_flNextMoanSound = gpGlobals->curtime;
	}
}

//-----------------------------------------------------------------------------
// Purpose: If we don't have any headcrabs to throw, we must close to attack our enemy.
//-----------------------------------------------------------------------------
bool CNPC_Monster::MustCloseToAttack(void)
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Overloaded so that explosions don't split the poison zombie in twain.
//-----------------------------------------------------------------------------
bool CNPC_Monster::ShouldBecomeTorso( const CTakeDamageInfo &info, float flDamageThreshold )
{
	return false;
}

int ACT_ZOMBIE_MONSTER_THREAT;

AI_BEGIN_CUSTOM_NPC( npc_monster, CNPC_Monster )

	DECLARE_ACTIVITY( ACT_ZOMBIE_MONSTER_THREAT )

AI_END_CUSTOM_NPC()