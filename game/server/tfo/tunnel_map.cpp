
#include "cbase.h"
#include "gamerules.h"
#include "baseanimating.h"
#include "items.h"
#include "hl2_player.h"
#include "basecombatweapon.h"
#include "player.h"
#include "ammodef.h"
#include "npcevent.h"
#include "eventlist.h"

class CTunnelMap : public CItem
{
public:
	DECLARE_CLASS( CTunnelMap, CItem );
	DECLARE_DATADESC();

	CTunnelMap()
	{
		color32 col32 = { 255,25,25,100 };
		m_GlowColor = col32;
	}

	void Spawn( void );
	void Precache( void );

	//bool MyTouch( CBasePlayer *pPlayer );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

private:

	COutputEvent	m_OnUse;	// Output when somebody clicks us

};

LINK_ENTITY_TO_CLASS( tunnel_map, CTunnelMap );

BEGIN_DATADESC( CTunnelMap )

	// Links our input name from Hammer to our input member function

	DEFINE_OUTPUT( m_OnUse, "OnUse" ),

	END_DATADESC()
	PRECACHE_REGISTER(tunnel_map);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTunnelMap::Spawn( void )
{
	Precache( );
	SetModel( "models/aoc_objects/scroll01_physics.mdl" );
    AddEffects( EF_NOSHADOW | EF_NORECEIVESHADOW );

	BaseClass::Spawn( );
}

void CTunnelMap::Precache( void )
{
	PrecacheModel ("models/aoc_objects/scroll01_physics.mdl");
}

void CTunnelMap:: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) //If player used item
{
	if (!pActivator)
		return;

	if ( !pActivator->IsPlayer() )
		return;

	ConVar* map2 = cvar->FindVar("cl_inv_map_tunnel");

	if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
	{
		m_OnUse.FireOutput( pActivator, this );
		EmitSound( "ItemValve.Touch" );
		map2->SetValue(1);
		UTIL_Remove(this);//remove item	
	}	
}