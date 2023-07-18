//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Inventory Item Handler. Reads tfo/resource/inventory/*.* files & parses valuable info such as: model, filename, ID, etc...
//
//=============================================================================//

#include "cbase.h"
#include "inv_inventory_item.h"
#include "hl2_gamerules.h"
#include "hl2_player.h"
#include "gamestringpool.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC(CInventoryItemLogic)
DEFINE_KEYFIELD(szFileName, FIELD_STRING, "ScriptFile"),
END_DATADESC()

LINK_ENTITY_TO_CLASS( inv_inventory_item, CInventoryItemLogic);
PRECACHE_REGISTER( inv_inventory_item );

CInventoryItemLogic::CInventoryItemLogic()
{
	m_GlowColor.Set({ 245, 25, 25, 235 });
	szFileName = NULL_STRING;
}

void CInventoryItemLogic::Spawn()
{
	// If we failed to parse this item:
	if (szFileName == NULL_STRING)
	{
		Warning( "An item with invalid properties has been removed!\n" );
		UTIL_Remove( this );
		return;
	}

	ParseFile( STRING(szFileName) );
}

void CInventoryItemLogic::Precache()
{
	BaseClass::Precache();
}

void CInventoryItemLogic::SetItemNameLink(const char *szItemName)
{
	szFileName = AllocPooledString(szItemName);
}

// Parse items available. Notice this is set by the mapper. The actual "script" to parse. If not set, this entity will self-destruct.
void CInventoryItemLogic::ParseFile( const char *FileName )
{
	bool bShouldRemove = true;

	// We set our keyvalues to read the stuff:
	KeyValues *kvItemInfo = new KeyValues( "InventoryItem" );
	if ( kvItemInfo->LoadFromFile( filesystem, UTIL_VarArgs( "data/inventory/items/%s.txt", FileName ), "MOD" ) )
	{
		KeyValues *pkvModel = kvItemInfo->FindKey( "ModelData" );

		if ( pkvModel != NULL )
		{
			// Get model and skin.
			KeyValues *pkvModel3DData = pkvModel->FindKey("Model");
			if (pkvModel3DData)
			{
				const char *szModel = ReadAndAllocStringValue(pkvModel3DData, "modelname");

				// Precache and set new model + skin.
				PrecacheModel(szModel);
				SetModel(szModel);
				m_nSkin = pkvModel->GetInt("Skin");

				// Do we want this model to be glowable?
				if (pkvModel->GetInt("NoGlow") >= 1)
					SetGlowMethod(GLOW_METHOD_NONE);
				else
					SetGlowMethod(GLOW_METHOD_RADIUS);

				// Get the RGBA colors for our item!
				color32 col32 = { pkvModel->GetInt("GlowR", 255), pkvModel->GetInt("GlowG", 255), pkvModel->GetInt("GlowB", 255), pkvModel->GetInt("GlowA", 255) };
				m_GlowColor = col32;

				bShouldRemove = false;
			}
		}
		else
		{
			Warning( "No model field was found in the script!\nSee sample script for help in the same folder!\nRemoving Entity!\n" );
		}
	}
	else
	{
		Warning( "Couldn't find or read the desired script file %s.txt\nEntity(item) has been removed.\n", FileName );
	}

	kvItemInfo->deleteThis();

	if ( bShouldRemove )
		UTIL_Remove( this );
	else
	{
		AddEffects( EF_NOSHADOW | EF_NORECEIVESHADOW );
		BaseClass::Spawn();
	}
}

// Make sure our client is a player and not a fake client, npc, etc...
void CInventoryItemLogic::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if (!pActivator || !pActivator->IsPlayer())
		return;

	CBasePlayer* pClient = ToBasePlayer(pActivator);
	if (!pClient)
		return;

	// Try to add the inventory item.
	if (pClient->AddInventoryItem(STRING(szFileName)))
	{
		EmitSound( "ItemValve.Touch" );

		// Fire output...
		m_OnUse.FireOutput( this, this );

		// Remove item from existence.
		UTIL_Remove( this );
	}
	else
		EmitSound( "ItemPickup.Reject" );
}