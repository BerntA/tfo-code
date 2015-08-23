//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Checks which weapon you have and outputs the one you have, for example MP40, use the output to handle the ammo spawn (point_template)
// This can be highly improved such as spawning ammo in the origin and angles of this entity and choosing an amount for the different weapons, etc...
//
//=============================================================================//

#include "cbase.h"
#include "gamerules.h"
#include "baseanimating.h"
#include "items.h"
#include "hl2_player.h"
#include "basecombatweapon.h"
#include "player.h"
#include "basecombatweapon_shared.h"
#include "basehlcombatweapon.h"
#include "baseplayer_shared.h"
#include "baseviewmodel_shared.h"
#include "hl2_player.h"

class CTFOWeaponManager : public CLogicalEntity
{
public:
	DECLARE_CLASS( CTFOWeaponManager, CLogicalEntity );
	DECLARE_DATADESC();

	CTFOWeaponManager ()
	{
	}

	void CheckWeapon( inputdata_t &inputData );
	void CheckHolstered( inputdata_t &inputData );

private:

	COutputEvent	m_OnP38;
	COutputEvent	m_OnMauser;
	COutputEvent	m_OnK98;
	COutputEvent	m_OnSVT40;
	COutputEvent	m_OnG43;
	COutputEvent	m_OnMP44;
	COutputEvent	m_OnMP40;

	COutputEvent	m_OnWarn;
	COutputEvent	m_OnRelax;
};

LINK_ENTITY_TO_CLASS( tfo_weapon_manager, CTFOWeaponManager  );

BEGIN_DATADESC( CTFOWeaponManager  )

	DEFINE_INPUTFUNC( FIELD_VOID, "CheckWep", CheckWeapon ),
	DEFINE_INPUTFUNC( FIELD_VOID, "CheckHolster", CheckHolstered ),

	DEFINE_OUTPUT( m_OnP38, "OnP38" ),
	DEFINE_OUTPUT( m_OnMauser, "OnMauser" ),
	DEFINE_OUTPUT( m_OnK98, "OnK98" ),
	DEFINE_OUTPUT( m_OnSVT40, "OnSVT40" ),
	DEFINE_OUTPUT( m_OnG43, "OnG43" ),
	DEFINE_OUTPUT( m_OnMP44, "OnMP44" ),
	DEFINE_OUTPUT( m_OnMP40, "OnMP40" ),

	DEFINE_OUTPUT( m_OnWarn, "OnWarn" ),
	DEFINE_OUTPUT( m_OnRelax, "OnRelax" ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Check Carried Weapons ( not active only ) Dyn ammo spawn
//-----------------------------------------------------------------------------
void CTFOWeaponManager::CheckWeapon( inputdata_t &inputData )
{
	CBasePlayer *pOwner = UTIL_GetLocalPlayer();
	if ( !pOwner )
		return;

	CBaseCombatWeapon *pP38 = pOwner->Weapon_OwnsThisType("weapon_p38");
	CBaseCombatWeapon *pMP40 = pOwner->Weapon_OwnsThisType("weapon_mp40");
	CBaseCombatWeapon *pG43 = pOwner->Weapon_OwnsThisType("weapon_g43");
	CBaseCombatWeapon *pC96 = pOwner->Weapon_OwnsThisType("weapon_mauser");
	CBaseCombatWeapon *pK98S = pOwner->Weapon_OwnsThisType("weapon_k98");
	CBaseCombatWeapon *pK98NS = pOwner->Weapon_OwnsThisType("weapon_k98ns");
	CBaseCombatWeapon *pSTG44 = pOwner->Weapon_OwnsThisType("weapon_stg44");
	CBaseCombatWeapon *pSVT40 = pOwner->Weapon_OwnsThisType("weapon_svt40");

	if (pP38)
		m_OnP38.FireOutput(this, this);
	if (pC96)
		m_OnMauser.FireOutput(this, this);
	if (pMP40)
		m_OnMP40.FireOutput(this, this);
	if (pSTG44)
		m_OnMP44.FireOutput(this, this);
	if (pG43)
		m_OnG43.FireOutput(this, this);
	if (pSVT40)
		m_OnSVT40.FireOutput(this, this);
	if (pK98S || pK98NS)
		m_OnK98.FireOutput(this, this);
}

//-----------------------------------------------------------------------------
// Purpose: Check if the player is holstered = Hand wep if not send a warning to comrades! Attack!
// Notice: The original idea behind the city map (old intro) was to allow the player to do some 'freeride' as in being able to attack friendlies, call in the cops, etc..
//-----------------------------------------------------------------------------
void CTFOWeaponManager::CheckHolstered(inputdata_t &inputData)
{
	CBasePlayer *pOwner = UTIL_GetLocalPlayer();
	if (!pOwner)
		return;

	CBaseCombatWeapon *pActiveWeapon = pOwner->GetActiveWeapon();
	if (!pActiveWeapon)
	{
		m_OnRelax.FireOutput(this, this);
		return;
	}

	if (FClassnameIs(pActiveWeapon, "weapon_hands"))
		m_OnRelax.FireOutput(this, this);
	else
		m_OnWarn.FireOutput(this, this);
}