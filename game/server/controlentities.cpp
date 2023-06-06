//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: contains entities who have no physical representation in the game, and who
//		must be triggered by other entities to cause their effects.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "player.h"
#include "tier1/strtools.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: changes the gravity of the player who activates this entity
//-----------------------------------------------------------------------------
class CTargetChangeGravity : public CPointEntity
{
public:
	DECLARE_CLASS( CTargetChangeGravity, CPointEntity );

	DECLARE_DATADESC();

	void InputChangeGrav( inputdata_t &inputdata );
	void InputResetGrav( inputdata_t &inputdata );

	int m_iGravity;

	int m_iOldGrav;
};

LINK_ENTITY_TO_CLASS( target_changegravity, CTargetChangeGravity );

BEGIN_DATADESC( CTargetChangeGravity )

	DEFINE_KEYFIELD( m_iGravity, FIELD_INTEGER, "gravity" ),
	DEFINE_FIELD( m_iOldGrav, FIELD_INTEGER ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ChangeGrav", InputChangeGrav ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ResetGrav", InputResetGrav ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Input handler for changing the activator's gravity.
//-----------------------------------------------------------------------------
void CTargetChangeGravity::InputChangeGrav( inputdata_t &inputdata )
{
	CBasePlayer *pl = ToBasePlayer( inputdata.pActivator );
	if ( !pl )
		return;

	// Save the gravity to restore it in InputResetGrav
	if ( m_iOldGrav )
		m_iOldGrav = pl->GetGravity();

	pl->SetGravity(m_iGravity);
}

//-----------------------------------------------------------------------------
// Purpose: Input handler for resetting the activator's gravity.
//-----------------------------------------------------------------------------
void CTargetChangeGravity::InputResetGrav( inputdata_t &inputdata )
{
	CBasePlayer *pl = ToBasePlayer( inputdata.pActivator );
	if ( !pl )
		return;

	pl->SetGravity(m_iOldGrav);
}