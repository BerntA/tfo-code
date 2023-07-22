//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Loading Panel - Overrides the default loading panel and also hides it + reads its progress value.
//
//=============================================================================//

#ifndef LOAD_GAME_PANEL_H
#define LOAD_GAME_PANEL_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include "vgui_controls/Frame.h"
#include <vgui/ISurface.h>
#include "vgui_controls/Button.h"
#include "vgui_controls/Divider.h"
#include "vgui_controls/ImagePanel.h"
#include <vgui_controls/Label.h>
#include "ImageProgressBar.h"

class CLoadingPanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CLoadingPanel, vgui::Frame);

public:
	CLoadingPanel(vgui::VPANEL parent);
	~CLoadingPanel();

	void SetRandomLoadingTip();
	void SetCustomLoadingImage(const char* szImage, bool bVisible);
	void SetIsLoadingMainMenu(bool bValue);

protected:
	virtual void OnThink();
	virtual void PaintBackground();
	virtual void PerformLayout();
	virtual void OnTick();
	virtual void OnScreenSizeChanged(int iOldWide, int iOldTall);
	virtual void ApplySchemeSettings(vgui::IScheme* pScheme);

private:
	void SetupLayout(void);
	void SetLoadingAttributes(void);
	void HideLegacyLoadingLayout(void);

	vgui::ImagePanel* m_pImgLoadingBackground;
	vgui::ImagePanel* m_pImgLoadingForeground;
	vgui::ImagePanel* m_pImgEagle;

	ImageProgressBar* m_pImgLoadingBar;
	vgui::Label* m_pTextLoadingTip;
	vgui::Divider* m_pBottom;

	bool m_bIsLoading;
	bool m_bIsLoadingMainMenu;
	bool m_bIsMenuVisibleAndInGame;
};

#endif // LOAD_GAME_PANEL_H