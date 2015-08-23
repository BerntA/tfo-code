//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Finds the given classname and fires an output when you tell it to find it. (if found)
//
//=============================================================================//

#include "cbase.h"
#include "baseentity.h"
#include "baseentity_shared.h"
#include "hl2_gamerules.h"

class CTFOClassnameFinder : public CLogicalEntity
{
	DECLARE_CLASS( CTFOClassnameFinder, CLogicalEntity );
	DECLARE_DATADESC();

public:

	CTFOClassnameFinder( void );
	void FindTarget( inputdata_t &inputdata );

private:

	string_t szTarget;
	COutputEvent pFoundTarget;
};

CTFOClassnameFinder::CTFOClassnameFinder( void )
{
	szTarget = NULL_STRING;
}

BEGIN_DATADESC( CTFOClassnameFinder )

	DEFINE_INPUTFUNC( FIELD_VOID, "Find", FindTarget ),
	DEFINE_OUTPUT( pFoundTarget, "OnFound" ),
	DEFINE_KEYFIELD( szTarget, FIELD_STRING, "TargetEnt" ),
	
END_DATADESC()

LINK_ENTITY_TO_CLASS( tfo_entity_finder, CTFOClassnameFinder );

void CTFOClassnameFinder::FindTarget( inputdata_t &inputdata )
{
	bool bFound = false;
	CBaseEntity *pMyFoundEnt = NULL;

	// Try classname
	pMyFoundEnt = gEntList.FindEntityByClassname( NULL, szTarget.ToCStr() );
	while ( pMyFoundEnt )
	{
		if ( !strcmp( pMyFoundEnt->GetClassname(), szTarget.ToCStr() ) )
		{
			pFoundTarget.FireOutput( this, this );
			bFound = true;
			break;
		}

		pMyFoundEnt = gEntList.FindEntityByClassname( pMyFoundEnt, szTarget.ToCStr() );
	}

	if ( bFound ) 
		return;

	// Try the actual name
	pMyFoundEnt = gEntList.FindEntityByName( NULL, szTarget.ToCStr() );
	while ( pMyFoundEnt )
	{
		if ( !strcmp( pMyFoundEnt->GetEntityName().ToCStr(), szTarget.ToCStr() ) )
		{
			pFoundTarget.FireOutput( this, this );
			break;
		}

		pMyFoundEnt = gEntList.FindEntityByName( pMyFoundEnt, szTarget.ToCStr() );
	}
}