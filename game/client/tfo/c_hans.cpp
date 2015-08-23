//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Hans, a friend of Walter and Karl. Ally NPC
//
//=============================================================================//

#include "cbase.h"
#include "c_ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_Hans : public C_AI_BaseNPC
{
public:
	DECLARE_CLASS(C_Hans, C_AI_BaseNPC);
	DECLARE_CLIENTCLASS();

	C_Hans();
	virtual			~C_Hans();

private:
	C_Hans(const C_Hans &); // not defined, not accessible
};

IMPLEMENT_CLIENTCLASS_DT(C_Hans, DT_NPC_Hans, CNPC_Hans)
END_RECV_TABLE()

C_Hans::C_Hans()
{
}


C_Hans::~C_Hans()
{
}