//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Walter, a friend of Hans and Karl. Ally NPC
//
//=============================================================================//

#include "cbase.h"
#include "c_ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_Walter : public C_AI_BaseNPC
{
public:
	DECLARE_CLASS(C_Walter, C_AI_BaseNPC);
	DECLARE_CLIENTCLASS();

	C_Walter();
	virtual			~C_Walter();

private:
	C_Walter(const C_Walter &); // not defined, not accessible
};

IMPLEMENT_CLIENTCLASS_DT(C_Walter, DT_NPC_Walter, CNPC_Walter)
END_RECV_TABLE()

C_Walter::C_Walter()
{
}

C_Walter::~C_Walter()
{
}