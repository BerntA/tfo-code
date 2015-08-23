//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Simple math function. (useless) Used for the tnt sequence in the tunnel map, you have to pickup all three tnts, we have to know what order we're in. 1/3, 2/3, etc...
// Using branches didn't solve the problem so I made this entity. It has to register the pickups so it tells you 2/3 on the next tnt pickup.
//
//=============================================================================//

#include "cbase.h"
#include "gamerules.h"
#include "baseanimating.h"
#include "items.h"
#include "hl2_player.h"
#include "basecombatweapon.h"
#include "player.h"

class CTFOMath : public CLogicalEntity
{
public:
	DECLARE_CLASS(CTFOMath, CLogicalEntity);
	DECLARE_DATADESC();

	CTFOMath()
	{
		m_iValue = 0;
	}

	void Spawn(void);
	void AddVal(inputdata_t &inputData);
	void CheckValue(int CurrVal);

private:

	int m_iValue;
	COutputEvent	m_OnAdd1;
	COutputEvent	m_OnAdd2;
	COutputEvent	m_OnAdd3;
};

LINK_ENTITY_TO_CLASS(tfo_math, CTFOMath);

BEGIN_DATADESC(CTFOMath)

DEFINE_INPUTFUNC(FIELD_VOID, "Add", AddVal),

DEFINE_OUTPUT(m_OnAdd1, "OnAdd1"),
DEFINE_OUTPUT(m_OnAdd2, "OnAdd2"),
DEFINE_OUTPUT(m_OnAdd3, "OnAdd3"),

DEFINE_FIELD(m_iValue, FIELD_INTEGER),

END_DATADESC()

void CTFOMath::Spawn(void)
{
	BaseClass::Spawn();

	m_iValue = 0;
}

void CTFOMath::AddVal(inputdata_t &inputData)
{
	CheckValue(m_iValue);
	m_iValue++;
}

void CTFOMath::CheckValue(int CurrVal)
{
	if (CurrVal == 0)
		m_OnAdd1.FireOutput(this, this);
	else if (CurrVal == 1)
		m_OnAdd2.FireOutput(this, this);
	else if (CurrVal == 2)
		m_OnAdd3.FireOutput(this, this);
}