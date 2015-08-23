//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Save Panel - Your Best Friend. (save station)
//
//=============================================================================//

#include "cbase.h"
#include "vgui_controls/Frame.h"
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/IInput.h>
#include <vgui/KeyCode.h>

class CSavingPanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CSavingPanel, vgui::Frame);

public:
	CSavingPanel(vgui::VPANEL parent);
	~CSavingPanel();

	void OnCommand(const char* pcCommand);

	void ApplySchemeSettings(vgui::IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);
	}

	void PaintBackground()
	{
		SetBgColor(Color(0, 0, 0, 0));
		SetPaintBackgroundType(0);
		BaseClass::PaintBackground();
	}

	void PerformLayout();
	void PerformDefaultLayout();
	void OnThink();
	void OnShowPanel(bool bShow);
	void OnScreenSizeChanged(int iOldWide, int iOldTall);
	void OnKeyCodeTyped(vgui::KeyCode code);
	void CheckRollovers(int x, int y);

private:

	vgui::Button *m_pButtonSlot[4];

	vgui::ImagePanel *m_pImgSlot[4];

	vgui::ImagePanel *m_pImgYes;
	vgui::ImagePanel *m_pImgNo;
	vgui::ImagePanel *m_pImgSure;

	vgui::Button *m_pButtonYes;
	vgui::Button *m_pButtonNo;

	vgui::ImagePanel *m_pImgBackground;

	vgui::ImagePanel *m_pImgSlots;

	vgui::ImagePanel *m_pImgCross;

	vgui::Button *m_pButtonCross;

	bool InAlert;
	bool InSlot[4];

	// Final Parts
	bool InRolloverSlot[4];
	bool InRolloverCross;

	bool InRolloverYes;
	bool InRolloverNo;

	KeyValues *GetSaveData(const char *szFile);
};