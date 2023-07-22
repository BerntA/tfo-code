//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Inventory Panel GUI - Still quite hardcoded, this file has been heavily refactored and tweaked for the past years.
//
//=============================================================================//

#ifndef GAME_INV_PANEL_H
#define GAME_INV_PANEL_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/Frame.h"
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/IInput.h>
#include <vgui/ILocalize.h>
#include "hl2_gamerules.h"
#include <vgui/KeyCode.h>
#include <KeyValues.h>
#include <tier0/mem.h>
#include "filesystem.h"
#include "utldict.h"
#include "utlstring.h"
#include "InventoryItem.h"

class CInventoryPanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CInventoryPanel, vgui::Frame);

public:
	CInventoryPanel(vgui::VPANEL parent);
	~CInventoryPanel();

	void OnCommand(const char* pcCommand);

	// The panel background image should be square, not rounded.
	void PaintBackground()
	{
		SetBgColor(Color(0, 0, 0, 0));
		SetPaintBackgroundType(0);
		BaseClass::PaintBackground();
	}

	void PerformLayout();
	void PerformDefaultLayout();
	void OnThink();

	void CheckRollovers(int x, int y);

	// Misc...
	void OnShowPanel(bool bShow);
	void RedrawAll(void);

private:

	// Inventory
	vgui::InventoryItem *m_pInvItem[12];

	bool m_bSelectedItem[12]; // Selected State
	bool InRolloverInvItems[12]; // Hovered State

	// Base GUI
	vgui::ImagePanel *m_pImgUse;
	vgui::ImagePanel *m_pImgDrop;

	vgui::ImagePanel *m_pImgBackground;
	vgui::ImagePanel *m_pImgGrid;

	vgui::Button *m_pButtonUse;
	vgui::Button *m_pButtonDrop;

	// Description & Name for items...
	vgui::Label *m_pLabelItemInfo;

	bool InRolloverUse;
	bool InRolloverDrop;

	const char *GetName(KeyValues *kvData);
	const char *GetDescription(KeyValues *kvData);
	const char *GetEntityToAffect(KeyValues *kvData, int entity);
	const char *GetActionOnEntity(KeyValues *kvData, int entity);
	int GetItemExhaustible(KeyValues *kvData);
	int GetEntitiesToAffect(KeyValues *kvData);
	const char *GetItemSoundScripts(KeyValues *kvData, const char *szKeyword, const char *szDefault = "");

protected:

	void ApplySchemeSettings(vgui::IScheme *pScheme);
	void OnScreenSizeChanged(int iOldWide, int iOldTall);
	void OnKeyCodeTyped(vgui::KeyCode code);
};

#endif // GAME_INV_PANEL_H