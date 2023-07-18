//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Inventory Item Handler. Reads tfo/resource/inventory/*.* files & parses valuable info such as: model, filename, ID, etc...
//
//=============================================================================//

#ifndef TFO_INVENTORY_LOGIC_H
#define TFO_INVENTORY_LOGIC_H

#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "items.h"
#include "baseentity.h"
#include "player.h"
#include "KeyValues.h"
#include "string_t.h"

class CInventoryItemLogic : public CItem
{
	DECLARE_CLASS(CInventoryItemLogic, CItem);
	DECLARE_DATADESC();

public:
	CInventoryItemLogic();

	void Spawn();
	void Precache();
	void ParseFile(const char* FileName);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void SetItemNameLink(const char* szItemName); // Used for dropping...

private:
	string_t szFileName; // Filename to the parsed file.
};

#endif // TFO_INVENTORY_LOGIC_H