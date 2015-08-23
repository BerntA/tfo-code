//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Custom Monster NPC Entity - Allows parsing of unique stuff. Parses npc scripts in resource/data/npc
//
//=============================================================================//

#include "cbase.h"
#include "ai_basenpc.h"
#include "ai_default.h"
#include "npc_headcrab.h"
#include "npcevent.h"
#include "entitylist.h"
#include "npc_BaseZombie.h"

class CNPC_Monster : public CAI_BlendingHost<CNPC_BaseZombie>
{
	DECLARE_CLASS( CNPC_Monster, CAI_BlendingHost<CNPC_BaseZombie> );
	DECLARE_DATADESC();

public:

	//
	// CBaseZombie implemenation.
	//
	bool ShouldBecomeTorso( const CTakeDamageInfo &info, float flDamageThreshold );
	virtual bool IsChopped( const CTakeDamageInfo &info )	{ return false; }
	bool CanSwatPhysicsObjects( void ) { return false; }

	//
	// CAI_BaseNPC implementation.
	//
	virtual float MaxYawSpeed( void );

	virtual float GetClawAttackRange() const { return m_flAttackRange; }

	virtual void PrescheduleThink( void );
	virtual void BuildScheduleTestBits( void );
	virtual int SelectSchedule( void );
	virtual int SelectFailSchedule( int nFailedSchedule, int nFailedTask, AI_TaskFailureCode_t eTaskFailCode );
	virtual int TranslateSchedule( int scheduleType );

	virtual bool ShouldPlayIdleSound( void );

	//
	// CBaseAnimating implementation.
	//
	virtual void HandleAnimEvent( animevent_t *pEvent );

	//
	// CBaseEntity implementation.
	//
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual void SetZombieModel( void );

	virtual Class_T Classify( void );
	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual int OnTakeDamage_Alive( const CTakeDamageInfo &inputInfo );

	virtual void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator );

	DEFINE_CUSTOM_AI;

	void PainSound( const CTakeDamageInfo &info );
	void DeathSound( const CTakeDamageInfo &info );
	void AlertSound( void );
	void IdleSound( void );
	void AttackSound( void );
	void AttackHitSound( void );
	void AttackMissSound( void );
	void FootstepSound( bool fRightFoot );
	void FootscuffSound( bool fRightFoot ) {};

	virtual void StopLoopingSounds( void );

	// Allow us to set the script from our spawn function (throuh console)
	void SetScript(const char *szScript);

	// Get Ent Name
	const char *GetEntName(void) { return cEntName.ToCStr(); }
	int GetMaxHP(void) { return m_iMaxHealth; }

protected:

	virtual bool CanOpenDoors( void ) { return m_bCanOpenDoors; }
	virtual bool MustCloseToAttack( void );

	virtual const char *GetMoanSound( int nSoundIndex );

private:

	// TFO Specific Parsings:
	KeyValues *pkvNPCData(const char *szScript);
	void ParseNPCScript(const char *szScript);
	const char *GetRandomModel(KeyValues *pkvValues);

	bool m_bCanOpenDoors;
	int m_iMaxHealth;
	int m_iSkin;
	int m_iDamage[2];
	float m_flAttackRange;
	float m_flLastBloodTrail;

	string_t cEntName;
	string_t cScript;
	string_t cModel;
	string_t cSoundScript;

	void BreatheOffShort( void );

	CSoundPatch *m_pFastBreathSound;
	CSoundPatch *m_pSlowBreathSound;

	float m_flNextPainSoundTime;

	bool m_bNearEnemy;
};