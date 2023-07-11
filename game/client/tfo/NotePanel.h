//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Displays Notes and sometimes fades in a VO, depends what the script says.
// Notice: In TFO v2.8 and lower Notes would be stored in your Inventory so you could read them later on, this feature has been removed in TFO V2.9+
//
//=============================================================================//

#include "cbase.h"
#include "vgui_controls/Frame.h"
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/IInput.h>
#include <vgui/KeyCode.h>
#include <vgui_controls/RichText.h>
#include "KeyValues.h"
#include "filesystem.h"

class CNotePanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CNotePanel, vgui::Frame);

public:
	CNotePanel(vgui::VPANEL parent);
	~CNotePanel();

	void OnCommand(const char* command);
	void ApplySchemeSettings(vgui::IScheme* pScheme);

	void PaintBackground()
	{
		SetBgColor(Color(0, 0, 0, 0));
		SetPaintBackgroundType(0);
		BaseClass::PaintBackground();
	}

	void PerformDefaultLayout();
	void OnThink();
	void OnShowPanel(bool bShow);
	void OnScreenSizeChanged(int iOldWide, int iOldTall);
	void OnKeyCodeTyped(vgui::KeyCode code);

	void ParseScriptFile(const char* szFile);

private:
	bool m_bCanFadeOutMusic;

	vgui::ImagePanel* m_pNote;
	vgui::Button* m_pButtonClose;
	vgui::RichText* m_pNoteText;
};