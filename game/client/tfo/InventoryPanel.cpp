//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Inventory Panel GUI - Still quite hardcoded, this file has been heavily refactored and tweaked for the past years.
//
//=============================================================================//

#include "cbase.h"
#include "InventoryPanel.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "c_basehlplayer.h"
#include "vgui_controls/Panel.h"
#include "vgui_controls/AnimationController.h"
#include "vgui/ISurface.h"
#include <vgui/ILocalize.h>
#include <vgui/IInput.h>
#include "ienginevgui.h"
#include "c_baseplayer.h" 
#include "fmod_manager.h"
#include "GameBase_Client.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void CInventoryPanel::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CInventoryPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pLabelItemInfo->SetFont(pScheme->GetFont("TFOInventory"));
	m_pLabelItemInfo->SetFgColor(Color(220, 220, 220, 245));
}

void CInventoryPanel::PerformDefaultLayout()
{
	InRolloverUse = false;
	InRolloverDrop = false;

	m_pImgBackground->SetVisible(true);

	m_pImgGrid->SetVisible(true);
	m_pImgGrid->SetImage("inventory/shared/grid");

	m_pLabelItemInfo->SetText("");
	m_pLabelItemInfo->SetVisible(true);

	for (int i = 0; i < _ARRAYSIZE(m_pInvItem); i++)
	{
		m_bSelectedItem[i] = false;
		InRolloverInvItems[i] = false;

		m_pInvItem[i]->SetVisible(false);
		m_pInvItem[i]->SetEnabled(false);

		int w, h;
		m_pInvItem[i]->GetSize(w, h);
		m_pInvItem[i]->SetSize(w, h);
	}

	// Button(s)

	// USE
	m_pImgUse->SetVisible(true);
	m_pButtonUse->SetVisible(true);

	// DROP
	m_pImgDrop->SetVisible(true);
	m_pButtonDrop->SetVisible(true);

	// Reset to Default Images:
	m_pImgUse->SetImage("inventory/menu_use");
	m_pImgDrop->SetImage("inventory/menu_drop");

	PerformLayout();
}

void CInventoryPanel::OnThink()
{
	// Get mouse coords
	int x, y;
	vgui::input()->GetCursorPos(x, y);

	SetSize(ScreenWidth(), ScreenHeight());
	m_pImgBackground->SetSize(ScreenWidth(), ScreenHeight());
	CheckRollovers(x, y);
	SetPos(0, 0);

	int iNumSelected = 0;
	for (int i = 0; i <= 11; i++)
	{
		if (InRolloverInvItems[i] || m_bSelectedItem[i])
			iNumSelected++;
	}

	// Clear info text:
	if (iNumSelected <= 0)
		m_pLabelItemInfo->SetText("");

	BaseClass::OnThink();
}

void CInventoryPanel::CheckRollovers(int x, int y)
{
	// Default Buttons
	m_pImgUse->SetImage(m_pButtonUse->IsWithin(x, y) ? "inventory/menu_use_over" : "inventory/menu_use");
	m_pImgDrop->SetImage(m_pButtonDrop->IsWithin(x, y) ? "inventory/menu_drop_over" : "inventory/menu_drop");

	// Items
	if ((GameBaseClient->pszInventoryList.Count() <= 0))
		return;

	// We check if any other item is currently selected, if there's a selected item we don't want to override the description text when we hover over another item.
	bool bAnyItemIsSelected = false;
	for (int i = 0; i < GameBaseClient->pszInventoryList.Count(); i++)
	{
		if (m_bSelectedItem[i])
		{
			bAnyItemIsSelected = true;
			break;
		}
	}

	for (int i = 0; i < GameBaseClient->pszInventoryList.Count(); i++)
	{
		if (m_pInvItem[i]->IsWithin(x, y))
		{
			if (!InRolloverInvItems[i])
			{
				KeyValues *kvInfo = GameBaseClient->GetInventoryItemDetails(GameBaseClient->pszInventoryList[i].inventoryItem);

				if (!bAnyItemIsSelected)
				{
					m_pLabelItemInfo->SetText(VarArgs("%s\n%s", GetName(kvInfo), GetDescription(kvInfo)));
					vgui::surface()->PlaySound("dialogue/buttonclick.wav");
				}

				kvInfo->deleteThis();

				InRolloverInvItems[i] = true;
			}
		}
		else
		{
			if (InRolloverInvItems[i] && !m_bSelectedItem[i])
				InRolloverInvItems[i] = false;
		}
	}
}

CInventoryPanel::CInventoryPanel(vgui::VPANEL parent) : BaseClass(NULL, "InventoryPanel")
{
	SetParent(parent);

	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	SetProportional(true);
	SetTitleBarVisible(false);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);
	SetSizeable(false);
	SetMoveable(false);
	SetVisible(false);

	// Labels
	m_pLabelItemInfo = vgui::SETUP_PANEL(new vgui::Label(this, "ItemInfo", ""));

	// Images
	m_pImgUse = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Use"));
	m_pImgDrop = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Drop"));
	m_pImgBackground = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "BackgroundIMG"));
	m_pImgGrid = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "GridIMG"));

	// Buttons
	m_pButtonUse = vgui::SETUP_PANEL(new vgui::Button(this, "btnUse", ""));
	m_pButtonDrop = vgui::SETUP_PANEL(new vgui::Button(this, "btnDrop", ""));

	// Inventory Panel BG Shared...
	m_pImgBackground->SetZPos(10);
	m_pImgBackground->SetEnabled(true);
	m_pImgBackground->SetVisible(true);
	m_pImgBackground->SetShouldScaleImage(true);
	m_pImgBackground->SetImage("inventory/shared/inventory_bg");

	// Use
	m_pButtonUse->SetPaintBorderEnabled(false);
	m_pButtonUse->SetPaintEnabled(false);
	m_pImgUse->SetImage("inventory/menu_use");
	m_pImgUse->SetZPos(35);
	m_pImgUse->SetShouldScaleImage(true);
	m_pButtonUse->SetZPos(40);
	m_pButtonUse->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonUse->SetArmedSound("dialogue/buttonclick.wav");
	m_pButtonUse->SetCommand("Use");

	// Drop	
	m_pButtonDrop->SetPaintBorderEnabled(false);
	m_pButtonDrop->SetPaintEnabled(false);
	m_pImgDrop->SetImage("inventory/menu_drop");
	m_pImgDrop->SetZPos(35);
	m_pImgDrop->SetShouldScaleImage(true);
	m_pButtonDrop->SetZPos(40);
	m_pButtonDrop->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonDrop->SetArmedSound("dialogue/buttonclick.wav");
	m_pButtonDrop->SetCommand("Drop");

	// Scroll & Grid
	m_pImgGrid->SetZPos(20);
	m_pImgGrid->SetImage("inventory/shared/grid");
	m_pImgGrid->SetShouldScaleImage(true);

	// Items & Notes...
	for (int i = 0; i < _ARRAYSIZE(m_pInvItem); i++)
	{
		m_pInvItem[i] = vgui::SETUP_PANEL(new vgui::InventoryItem(this, VarArgs("invItem%i", i)));
		m_pInvItem[i]->SetZPos(35);
		m_pInvItem[i]->SetControlID(i);
	}

	m_pLabelItemInfo->SetText("");
	m_pLabelItemInfo->SetZPos(45);

	PerformDefaultLayout();

	SetScheme("TFOScheme");

	m_pImgBackground->SetSize(ScreenWidth(), ScreenHeight());

	InvalidateLayout();

	LoadControlSettings("resource/ui/inventorypanel.res");
}

CInventoryPanel::~CInventoryPanel()
{
}

void CInventoryPanel::OnScreenSizeChanged(int iOldWide, int iOldTall)
{
	BaseClass::OnScreenSizeChanged(iOldWide, iOldTall);
	LoadControlSettings("resource/ui/inventorypanel.res");
}

void CInventoryPanel::OnShowPanel(bool bShow)
{
	PerformDefaultLayout();

	if (bShow)
	{
		engine->ClientCmd_Unrestricted("gameui_preventescapetoshow\n");
		RedrawAll();
	}
	else
		engine->ClientCmd_Unrestricted("gameui_allowescapetoshow\n");
}

void CInventoryPanel::OnKeyCodeTyped(vgui::KeyCode code)
{
	if (code == KEY_ESCAPE || code == KEY_TAB)
	{
		vgui::surface()->PlaySound("hud/read_paper.wav");
		PerformDefaultLayout();
		engine->ClientCmd("tfo_gameui_command OpenInventoryPanel\n");
	}
	else
		BaseClass::OnKeyCodeTyped(code);
}

// Redraw / Recreate / Reset & ReAdd stuff...
void CInventoryPanel::RedrawAll(void)
{
	if ((GameBaseClient->pszInventoryList.Count() <= 0))
		return;

	for (int i = 0; i < _ARRAYSIZE(m_pInvItem); i++)
	{
		m_pInvItem[i]->SetVisible(false);
		m_pInvItem[i]->SetEnabled(false);

		m_bSelectedItem[i] = false;
		InRolloverInvItems[i] = false;
	}

	for (int i = 0; i < GameBaseClient->pszInventoryList.Count(); i++)
	{
		KeyValues *pkvData = GameBaseClient->GetInventoryItemDetails(GameBaseClient->pszInventoryList[i].inventoryItem);
		m_pInvItem[i]->SetItemDetails(pkvData);
		m_pInvItem[i]->SetVisible(true);
		m_pInvItem[i]->SetEnabled(true);
		pkvData->deleteThis();
	}
}

const char *CInventoryPanel::GetName(KeyValues *kvData)
{
	const char *szResult = NULL;

	KeyValues *pkvStandardInfo = kvData->FindKey("GenericInfo");
	if (pkvStandardInfo)
		szResult = ReadAndAllocStringValue(pkvStandardInfo, "Name");

	return szResult;
}

const char *CInventoryPanel::GetDescription(KeyValues *kvData)
{
	const char *szResult = NULL;

	KeyValues *pkvStandardInfo = kvData->FindKey("GenericInfo");
	if (pkvStandardInfo)
		szResult = ReadAndAllocStringValue(pkvStandardInfo, "Description");

	return szResult;
}

const char *CInventoryPanel::GetEntityToAffect(KeyValues *kvData, int entity)
{
	const char *szResult = NULL;

	int iStart = 0;
	KeyValues *pkvInventoryInfo = kvData->FindKey("InventoryData");
	if (pkvInventoryInfo)
	{
		KeyValues *pkvEntities = pkvInventoryInfo->FindKey("Entities");
		if (pkvEntities)
		{
			for (KeyValues *sub = pkvEntities->GetFirstSubKey(); sub; sub = sub->GetNextKey())
			{
				iStart++;
				if (iStart == entity)
				{
					szResult = sub->GetName();
					break;
				}
			}
		}
	}

	return szResult;
}

const char *CInventoryPanel::GetActionOnEntity(KeyValues *kvData, int entity)
{
	const char *szResult = NULL;

	int iStart = 0;
	KeyValues *pkvInventoryInfo = kvData->FindKey("InventoryData");
	if (pkvInventoryInfo)
	{
		KeyValues *pkvEntities = pkvInventoryInfo->FindKey("Entities");
		if (pkvEntities)
		{
			for (KeyValues *sub = pkvEntities->GetFirstSubKey(); sub; sub = sub->GetNextKey())
			{
				iStart++;
				if (iStart == entity)
				{
					szResult = sub->GetString();
					break;
				}
			}
		}
	}

	return szResult;
}

int CInventoryPanel::GetItemExhaustible(KeyValues *kvData)
{
	int iResult = 0;

	KeyValues *pkvInventoryInfo = kvData->FindKey("InventoryData");
	if (pkvInventoryInfo)
		iResult = atoi(ReadAndAllocStringValue(pkvInventoryInfo, "Exhaustible"));

	return iResult;
}

// Return available / choosen entities that the item can affect through out the game.
int CInventoryPanel::GetEntitiesToAffect(KeyValues *kvData)
{
	int iResult = 0;

	KeyValues *pkvInventoryInfo = kvData->FindKey("InventoryData");
	if (pkvInventoryInfo)
	{
		KeyValues *pkvEntities = pkvInventoryInfo->FindKey("Entities");
		if (pkvEntities)
		{
			for (KeyValues *sub = pkvEntities->GetFirstSubKey(); sub; sub = sub->GetNextKey())
				iResult++;
		}
	}

	return iResult;
}

// We return a sound script that we will play on the server...
const char *CInventoryPanel::GetItemSoundScripts(KeyValues *kvData, const char *szKeyword, const char *szDefault)
{
	const char *szResult = NULL;

	KeyValues *pkvInventoryInfo = kvData->FindKey("InventoryData");
	if (pkvInventoryInfo)
		szResult = pkvInventoryInfo->GetString(szKeyword, szDefault);

	return szResult;
}

void CInventoryPanel::OnCommand(const char* pcCommand)
{
	// Clicking Items
	for (int i = 0; i < _ARRAYSIZE(m_pInvItem); i++)
	{
		if (!Q_stricmp(pcCommand, VarArgs("OnItem%i", i)))
		{
			m_bSelectedItem[i] = !m_bSelectedItem[i]; // Toggle selection.

			// Deselect every other item.
			for (int x = 0; x < _ARRAYSIZE(m_pInvItem); x++)
			{
				if (m_bSelectedItem[x] && (x != i))
				{
					m_pInvItem[x]->SetSelectedState(false);
					m_bSelectedItem[x] = false;
				}
			}

			m_pInvItem[i]->SetSelectedState(m_bSelectedItem[i]);
			vgui::surface()->PlaySound("ui/buttonclick.wav");

			KeyValues *pkvData = GameBaseClient->GetInventoryItemDetails(GameBaseClient->pszInventoryList[i].inventoryItem);
			m_pLabelItemInfo->SetText(VarArgs("%s\n%s", GetName(pkvData), GetDescription(pkvData)));
			pkvData->deleteThis();

			break; // We found the right stuff, break it up already...
		}
	}

	if (!Q_stricmp(pcCommand, "Use"))
	{
		for (int i = 0; i < GameBaseClient->pszInventoryList.Count(); i++)
		{
			if (m_bSelectedItem[i])
			{
				bool bShouldRemove = false;

				KeyValues *pkvInventoryData = GameBaseClient->GetInventoryItemDetails(GameBaseClient->pszInventoryList[i].inventoryItem);

				// Some items may be removed from inventory on use:
				bShouldRemove = (GetItemExhaustible(pkvInventoryData) == 1) ? true : false;

				const char *szVOSoundSuccess = GetItemSoundScripts(pkvInventoryData, "UseVOSuccessSound", "EMPTY");
				const char *szVOSoundFailure = GetItemSoundScripts(pkvInventoryData, "UseVOFailureSound", "EMPTY");
				const char *szItemSoundSuccess = GetItemSoundScripts(pkvInventoryData, "UseItemSuccessSound", "EMPTY");
				const char *szItemSoundFailure = GetItemSoundScripts(pkvInventoryData, "UseItemFailureSound", "EMPTY");

				// Check how many entities this item can affect:
				int availableEntities = GetEntitiesToAffect(pkvInventoryData);
				if (availableEntities <= 0)
				{
					Warning("Item %s has no Entities to affect!\n", GameBaseClient->pszInventoryList[i].inventoryItem);
					break;
				}

				// Try all possibilities:
				for (int x = 1; x <= availableEntities; x++)
					engine->ClientCmd(VarArgs("tfo_inventory_call_use %s %s %i %s %s %s %s %s\n", GetEntityToAffect(pkvInventoryData, x), GetActionOnEntity(pkvInventoryData, x), ((bShouldRemove) ? 1 : 0), GameBaseClient->pszInventoryList[i].inventoryItem, szVOSoundSuccess, szVOSoundFailure, szItemSoundSuccess, szItemSoundFailure));

				m_bSelectedItem[i] = false;
				InRolloverInvItems[i] = false;

				pkvInventoryData->deleteThis();

				RedrawAll();
				PerformLayout();
				engine->ClientCmd("tfo_gameui_command OpenInventoryPanel\n");

				break;
			}
		}
	}

	if (!Q_stricmp(pcCommand, "Drop"))
	{
		for (int i = 0; i < GameBaseClient->pszInventoryList.Count(); i++)
		{
			if (m_bSelectedItem[i])
			{
				GameBaseClient->RemoveItemFromInventory(GameBaseClient->pszInventoryList[i].inventoryItem, -1, true);
				RedrawAll();
				engine->ClientCmd("tfo_gameui_command OpenInventoryPanel\n");
				break;
			}
		}
	}
}