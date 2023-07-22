//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Pops Up on Death, allows you to restart the game (load latest save if possible) or go back to the main menu.
//
//=============================================================================//

#ifndef DEATH_PANEL_H
#define DEATH_PANEL_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/Frame.h"
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/IInput.h>

class CDeathPanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CDeathPanel, vgui::Frame);

public:
	CDeathPanel(vgui::VPANEL parent);
	~CDeathPanel();

	void OnCommand(const char* pcCommand);

	void ApplySchemeSettings(vgui::IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);
	}

	// The panel background image should be square, not rounded.
	virtual void PaintBackground()
	{
		SetBgColor(Color(0, 0, 0, 0));
		SetPaintBackgroundType(0);
		BaseClass::PaintBackground();
	}

	void OnShowPanel(bool state);
	void PerformLayout();
	void PerformDefaultLayout();
	void OnThink();

	void CheckRollovers(int x, int y);

	void OnScreenSizeChanged(int iOldWide, int iOldTall);

private:

	void ShowLayout(bool bShow);
	vgui::Button *m_pButtonSlot1;
	vgui::Button *m_pButtonSlot2;

	vgui::ImagePanel *m_pImgSlot1;
	vgui::ImagePanel *m_pImgSlot2;

	vgui::ImagePanel *m_pImgBackground;

	bool InRolloverSlot1;
	bool InRolloverSlot2;
};

#endif // DEATH_PANEL_H