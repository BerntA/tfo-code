//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Mauser C96
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "NPCevent.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"
#include "player.h"
#include "game.h"
#include "in_buttons.h"
#include "grenade_ar2.h"
#include "AI_Memory.h"
#include "soundent.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "baseplayer_shared.h"
#include "te_effect_dispatch.h"
#include "particle_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponMauser : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CWeaponMauser, CHLSelectFireMachineGun );

	CWeaponMauser();

	DECLARE_SERVERCLASS();
	
	void	Precache( void );
	void	AddViewKick( void );

	void	ItemPostFrame( void );

	int		GetMinBurst() { return 2; }
	int		GetMaxBurst() { return 5; }

	void Equip( CBaseCombatCharacter *pOwner );
	bool	Reload( void );

	float	GetFireRate( void ) { return 0.075f; }	
	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	virtual const Vector& GetBulletSpread( void )
	{
		static Vector cone;

		if (m_bIsIronsighted)
			cone = VECTOR_CONE_1DEGREES;
		else
			cone = VECTOR_CONE_4DEGREES;

		return cone;
	}

	const WeaponProficiencyInfo_t *GetProficiencyValues();

	void FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir );
	void Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary );
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	DECLARE_ACTTABLE();

protected:

	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;
};

IMPLEMENT_SERVERCLASS_ST(CWeaponMauser, DT_WeaponMauser)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_mauser, CWeaponMauser );
PRECACHE_WEAPON_REGISTER(weapon_mauser);

BEGIN_DATADESC( CWeaponMauser )

	DEFINE_FIELD( m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( m_flNextGrenadeCheck, FIELD_TIME ),

END_DATADESC()

acttable_t	CWeaponMauser::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE_PISTOL,				true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_PISTOL,			true },
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_PISTOL,		true },
	{ ACT_RELOAD,					ACT_RELOAD_PISTOL,				true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_PISTOL,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_PISTOL,				true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_PISTOL,true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_PISTOL_LOW,			false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_PISTOL_LOW,	false },
	{ ACT_COVER_LOW,				ACT_COVER_PISTOL_LOW,			false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_PISTOL_LOW,		false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_PISTOL,		false },
	{ ACT_WALK,						ACT_WALK_PISTOL,				false },
	{ ACT_RUN,						ACT_RUN_PISTOL,					false },

    { ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_PISTOL,                    false },
    { ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_PISTOL,                    false },
    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_PISTOL,            false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_PISTOL,            false },
    { ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,    false },
    { ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_PISTOL,        false },
    { ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_PISTOL,                    false },
    { ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_PISTOL,                false },
};

IMPLEMENT_ACTTABLE(CWeaponMauser);

//=========================================================
CWeaponMauser::CWeaponMauser( )
{
	m_fMinRange1		= 0;// No minimum range. 
	m_fMaxRange1		= 1400;

	m_bMagazineStyleReloads = true; // Magazine style reloads
	m_bAltFiresUnderwater = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMauser::Precache( void )
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponMauser::Equip( CBaseCombatCharacter *pOwner )
{
	if( pOwner->Classify() == CLASS_PLAYER_ALLY )
	{
		m_fMaxRange1 = 3000;
	}
	else
	{
		m_fMaxRange1 = 1400;
	}

	BaseClass::Equip( pOwner );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMauser::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir )
{
	// FIXME: use the returned number of bullets to account for >10hz firerate
	WeaponSoundRealtime( SINGLE_NPC );

	CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );
	pOperator->FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED,
		MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0 );

	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMauser::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary )
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;
	GetAttachment( LookupAttachment( "muzzle" ), vecShootOrigin, angShootDir );
	AngleVectors( angShootDir, &vecShootDir );
	FireNPCPrimaryAttack( pOperator, vecShootOrigin, vecShootDir );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMauser::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
	case EVENT_WEAPON_SMG1:
		{
			Vector vecShootOrigin, vecShootDir;
			QAngle angDiscard;

			// Support old style attachment point firing
			if ((pEvent->options == NULL) || (pEvent->options[0] == '\0') || (!pOperator->GetAttachment(pEvent->options, vecShootOrigin, angDiscard)))
			{
				vecShootOrigin = pOperator->Weapon_ShootPosition();
			}

			CAI_BaseNPC *npc = pOperator->MyNPCPointer();
			ASSERT( npc != NULL );
			vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );

			FireNPCPrimaryAttack( pOperator, vecShootOrigin, vecShootDir );
		}
		break;

	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponMauser::Reload( void )
{
	bool fRet;
	float fCacheTime = m_flNextSecondaryAttack;

	fRet = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
	if ( fRet )
	{
		// Undo whatever the reload process has done to our secondary
		// attack timer. We allow you to interrupt reloading to fire
		// a grenade.
		m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;

		WeaponSound( RELOAD );

		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		CEffectData data;
		data.m_vOrigin = pOwner->WorldSpaceCenter() + RandomVector( 0, 0 );
		data.m_vAngles = QAngle( 90, random->RandomInt( 0, 360 ), 0 );
		data.m_nEntIndex = entindex();
		DispatchEffect( "ClipEject", data );
	}

	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMauser::AddViewKick( void )
{
	#define	EASY_DAMPEN			0.5f
	#define	MAX_VERTICAL_KICK	1.0f	//Degrees
	#define	SLIDE_LIMIT			2.0f	//Seconds
	
	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( pPlayer == NULL )
		return;

	DoMachineGunKick( pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT );
}

void CWeaponMauser::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( pPlayer == NULL )
		return;

	// Ammo Fix
	int CurrAmmo = pPlayer->GetAmmoCount(m_iPrimaryAmmoType);
	if ( CurrAmmo <= 19 && CurrAmmo >= 1 )
	{
		pPlayer->RemoveAmmo( CurrAmmo, m_iPrimaryAmmoType );
	}
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponMauser::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0,		0.75	},
		{ 5.00,		0.75	},
		{ 10.0/3.0, 0.75	},
		{ 5.0/3.0,	0.75	},
		{ 1.00,		1.0		},
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}