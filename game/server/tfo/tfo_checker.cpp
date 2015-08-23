//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Checks if you're a donator / got end game achiev.
//
//=============================================================================//

#include "cbase.h"
#include "gamerules.h"
#include "baseanimating.h"
#include "items.h"
#include "hl2_player.h"
#include "basecombatweapon.h"
#include "player.h"

class CTFOChecker : public CLogicalEntity
{
public:
	DECLARE_CLASS(CTFOChecker, CLogicalEntity);
	DECLARE_DATADESC();

	CTFOChecker()
	{
	}

	void CheckWep(inputdata_t &inputData);

private:

	COutputEvent m_OnCheck;
};

LINK_ENTITY_TO_CLASS(tfo_checker, CTFOChecker);

BEGIN_DATADESC(CTFOChecker)

DEFINE_INPUTFUNC(FIELD_VOID, "Check", CheckWep),
DEFINE_OUTPUT(m_OnCheck, "OnCheck"),

END_DATADESC()

void CTFOChecker::CheckWep(inputdata_t &inputData)
{
	CBasePlayer *pClient = UTIL_GetLocalPlayer();
	if (pClient)
	{
		if (pClient->m_bCanPickupRewards)
			m_OnCheck.FireOutput(this, this);
	}
}