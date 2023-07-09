//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Default Hands
//
//=============================================================================//

#include "cbase.h"
#include "weapon_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_plr_dmg_hands("sk_plr_dmg_hands", "0");

#define	HANDS_RANGE		50.0f
#define	HANDS_REFIRE	0.5f

class CWeaponHands : public CBaseHLBludgeonWeapon
{
public:
	DECLARE_CLASS(CWeaponHands, CBaseHLBludgeonWeapon);
	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponHands();

	float		GetRange(void) { return	HANDS_RANGE; }
	float		GetFireRate(void) { return	HANDS_REFIRE; }
	bool		HasIronsights() { return false; }
	bool		IsHands(void) { return true; }
	bool		VisibleInWeaponSelection() { return false; }
	bool		CanAttack(void) { return true; }
	void		AddViewKick(void);
	float		GetDamageForActivity(Activity hitActivity);
};

IMPLEMENT_SERVERCLASS_ST(CWeaponHands, DT_WeaponHands)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_hands, CWeaponHands);
PRECACHE_WEAPON_REGISTER(weapon_hands);

acttable_t CWeaponHands::m_acttable[] = 
{
	{ ACT_IDLE, ACT_IDLE_SUITCASE, false },
	{ ACT_IDLE_ANGRY, ACT_IDLE_SUITCASE, false },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_MELEE_ATTACK1, true },
	{ ACT_MELEE_ATTACK1, ACT_MELEE_ATTACK1, true },
	{ ACT_RANGE_ATTACK1, ACT_MELEE_ATTACK1, true },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_MELEE_ATTACK1, false },
   
	{ ACT_HL2MP_IDLE, ACT_IDLE_SUITCASE, false },
    { ACT_HL2MP_RUN,                    ACT_RUN,                    false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_SLAM, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_WALK_CROUCH, false },
	{ ACT_HL2MP_JUMP, ACT_JUMP, false },

};

IMPLEMENT_ACTTABLE(CWeaponHands);

CWeaponHands::CWeaponHands( void )
{
}

float CWeaponHands::GetDamageForActivity( Activity hitActivity )
{
	return sk_plr_dmg_hands.GetFloat();
}

void CWeaponHands::AddViewKick( void )
{
	CBasePlayer *pPlayer  = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	QAngle punchAng;

	punchAng.x = random->RandomFloat( 1.0f, 2.0f );
	punchAng.y = random->RandomFloat( -2.0f, -1.0f );
	punchAng.z = 0.0f;
	
	pPlayer->ViewPunch( punchAng ); 
}