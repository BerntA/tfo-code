//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Custom Video Options
//
//=============================================================================//

#include "cbase.h"
#include <stdio.h>
#include "filesystem.h"
#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui/MouseCode.h>
#include "OptionsMenuVideo.h"
#include <vgui_controls/Button.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/TextImage.h>
#include <vgui_controls/ImageList.h>
#include "vgui_controls/CheckButton.h"
#include "IGameUIFuncs.h"
#include "modes.h"
#include "utlvector.h"
#include "materialsystem/materialsystem_config.h"
#include "inetchannelinfo.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

struct RatioToAspectMode_t
{
	int anamorphic;
	float aspectRatio;
};

RatioToAspectMode_t g_RatioToAspectModes[] =
{
	{ 0, 4.0f / 3.0f },
	{ 1, 16.0f / 9.0f },
	{ 2, 16.0f / 10.0f },
	{ 2, 1.0f },
};

//-----------------------------------------------------------------------------
// Purpose: returns the aspect ratio mode number for the given resolution
//-----------------------------------------------------------------------------
int GetScreenAspectMode(int width, int height)
{
	float aspectRatio = (float)width / (float)height;

	// just find the closest ratio
	float closestAspectRatioDist = 99999.0f;
	int closestAnamorphic = 0;
	for (int i = 0; i < ARRAYSIZE(g_RatioToAspectModes); i++)
	{
		float dist = fabs(g_RatioToAspectModes[i].aspectRatio - aspectRatio);
		if (dist < closestAspectRatioDist)
		{
			closestAspectRatioDist = dist;
			closestAnamorphic = g_RatioToAspectModes[i].anamorphic;
		}
	}

	return closestAnamorphic;
}

//-----------------------------------------------------------------------------
// Purpose: returns the string name of the specified resolution mode
//-----------------------------------------------------------------------------
void GetResolutionName(vmode_t *mode, char *sz, int sizeofsz)
{
	if (mode->width == 1280 && mode->height == 1024)
	{
		// LCD native monitor resolution gets special case
		Q_snprintf(sz, sizeofsz, "%i x %i (LCD)", mode->width, mode->height);
	}
	else
	{
		Q_snprintf(sz, sizeofsz, "%i x %i", mode->width, mode->height);
	}
}

OptionsMenuVideo::OptionsMenuVideo(vgui::Panel *parent, char const *panelName) : vgui::Panel(parent, panelName)
{
	AddActionSignalTarget(this);
	SetParent(parent);
	SetName(panelName);

	SetSize(285, 315);

	SetMouseInputEnabled(true);
	SetKeyBoardInputEnabled(true);
	SetProportional(false);

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/TFOScheme.res", "TFOScheme"));

	int iOptionsMenuPosY[] =
	{
		0,
		55,
		120,
	};

	// FOV
	m_pFOVSlider = vgui::SETUP_PANEL(new vgui::GraphicalSlider(this, "FOVSlider"));
	m_pFOVSliderGUI = vgui::SETUP_PANEL(new vgui::GraphicalOverlay(this, "FOVSliderGUI"));
	m_pFOVSliderInfo = vgui::SETUP_PANEL(new vgui::Label(this, "InfoFOVSlider", ""));

	m_pFOVSlider->SetPos(6, 205 + 5);
	m_pFOVSlider->SetRange(75, 90);
	m_pFOVSlider->SetSize(256, 40);
	m_pFOVSlider->SetTickCaptions("Near", "Far");
	m_pFOVSlider->SetZPos(45);

	m_pFOVSliderGUI->SetPos(7, 203 + 5);
	m_pFOVSliderGUI->SetSize(300, 40);
	m_pFOVSliderGUI->SetZPos(35);

	m_pFOVSliderInfo->SetZPos(50);
	m_pFOVSliderInfo->SetText("");
	m_pFOVSliderInfo->SetSize(150, 24);
	m_pFOVSliderInfo->SetPos(45, 180 + 5);
	m_pFOVSliderInfo->SetContentAlignment(Label::a_center);

	// Set slider to default value!
	ConVar *fov_var = cvar->FindVar("fov_desired");
	if (fov_var)
	{
		int iVal = fov_var->GetInt();

		if (iVal > 0)
			m_pFOVSlider->SetValue(iVal);
		else
			m_pFOVSlider->SetValue(75.0f);
	}

	m_pSlider = vgui::SETUP_PANEL(new vgui::GraphicalSlider(this, "Slider"));
	m_pSliderGUI = vgui::SETUP_PANEL(new vgui::GraphicalOverlay(this, "SliderGUI"));
	m_pSliderInfo = vgui::SETUP_PANEL(new vgui::Label(this, "InfoSlider", ""));

	m_pSlider->SetPos(6, 271 + 5);
	m_pSlider->SetRange(0, 100);
	m_pSlider->SetSize(256, 40);
	m_pSlider->SetTickCaptions("Bright", "Dark");
	m_pSlider->SetZPos(45);

	m_pSliderGUI->SetPos(7, 269 + 5);
	m_pSliderGUI->SetSize(300, 40);
	m_pSliderGUI->SetZPos(35);

	m_pSliderInfo->SetZPos(50);
	m_pSliderInfo->SetText("");
	m_pSliderInfo->SetSize(150, 24);
	m_pSliderInfo->SetPos(45, 246 + 5);
	m_pSliderInfo->SetContentAlignment(Label::a_center);

	// set slider value! Important, or else it forces gamma to 0 by default startup... -_-
	ConVar *slider_var = cvar->FindVar("mat_monitorgamma");
	if (slider_var)
	{
		float flVal = slider_var->GetFloat();

		if (flVal > 0)
			m_pSlider->SetValue(((flVal / 0.01) - (1.6 / 0.01)));
		else
			m_pSlider->SetValue(1.6f);
	}

	const char *szName[] =
	{
		"Resolution",
		"AspectRatio",
	};

	m_pVideoScreenModBtn = vgui::SETUP_PANEL(new vgui::Button(this, "WindowedBtn", ""));
	m_pVideoScreenModeImg = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "WindowedImg"));

	m_pVideoScreenModBtn->AddActionSignalTarget(this);
	m_pVideoScreenModBtn->SetSize(256, 28);
	m_pVideoScreenModBtn->SetPaintBorderEnabled(false);
	m_pVideoScreenModBtn->SetPaintEnabled(false);
	m_pVideoScreenModBtn->SetReleasedSound("ui/buttonclick.wav");
	m_pVideoScreenModBtn->SetZPos(30);
	m_pVideoScreenModBtn->SetCommand("CheckWindowed");

	m_pVideoScreenModeImg->SetZPos(20);
	m_pVideoScreenModeImg->SetImage("options/videowindowmode_off");
	m_pVideoScreenModeImg->SetSize(256, 64);

	m_pVideoScreenModBtn->SetPos(6, iOptionsMenuPosY[0]);
	m_pVideoScreenModeImg->SetPos(6, (iOptionsMenuPosY[0] - 12));

	for (int i = 0; i <= 1; i++)
	{
		m_pResolutionCombo[i] = vgui::SETUP_PANEL(new vgui::ComboBox(this, szName[i], iItems(i), false));
		m_pResolutionInfo[i] = vgui::SETUP_PANEL(new vgui::Label(this, "Info", ""));

		m_pResolutionCombo[i]->SetSize(120, 30);
		m_pResolutionInfo[i]->SetZPos(50);
		m_pResolutionInfo[i]->SetText("");
		m_pResolutionInfo[i]->SetSize(120, 24);
		m_pResolutionInfo[i]->SetPos(75, iOptionsMenuPosY[(i + 1)]);
		m_pResolutionInfo[i]->SetContentAlignment(Label::a_center);
	}

	m_pResolutionCombo[0]->SetPos(75, 75 + 5);
	m_pResolutionCombo[1]->SetPos(75, 140 + 5);

	m_pResolutionInfo[0]->SetText("Aspect Ratio");
	m_pResolutionInfo[1]->SetText("Resolution");

	char pszAspectName[3][64];
	wchar_t *unicodeText = g_pVGuiLocalize->Find("#GameUI_AspectNormal");
	g_pVGuiLocalize->ConvertUnicodeToANSI(unicodeText, pszAspectName[0], 32);
	unicodeText = g_pVGuiLocalize->Find("#GameUI_AspectWide16x9");
	g_pVGuiLocalize->ConvertUnicodeToANSI(unicodeText, pszAspectName[1], 32);
	unicodeText = g_pVGuiLocalize->Find("#GameUI_AspectWide16x10");
	g_pVGuiLocalize->ConvertUnicodeToANSI(unicodeText, pszAspectName[2], 32);

	int iNormalItemID = m_pResolutionCombo[0]->AddItem(pszAspectName[0], NULL);
	int i16x9ItemID = m_pResolutionCombo[0]->AddItem(pszAspectName[1], NULL);
	int i16x10ItemID = m_pResolutionCombo[0]->AddItem(pszAspectName[2], NULL);

	const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();

	int iAspectMode = GetScreenAspectMode(config.m_VideoMode.m_Width, config.m_VideoMode.m_Height);
	switch (iAspectMode)
	{
	default:
	case 0:
		m_pResolutionCombo[0]->ActivateItem(iNormalItemID);
		break;
	case 1:
		m_pResolutionCombo[0]->ActivateItem(i16x9ItemID);
		break;
	case 2:
		m_pResolutionCombo[0]->ActivateItem(i16x10ItemID);
		break;
	}

	PrepareResolutionList();

	InvalidateLayout();
}

// On deletion of player class / vgui.
OptionsMenuVideo::~OptionsMenuVideo()
{
}

//-----------------------------------------------------------------------------
// Purpose: Generates resolution list
//-----------------------------------------------------------------------------
void OptionsMenuVideo::PrepareResolutionList()
{
	// get the currently selected resolution
	char sz[256];
	m_pResolutionCombo[1]->GetText(sz, 256);
	int currentWidth = 0, currentHeight = 0;
	sscanf(sz, "%i x %i", &currentWidth, &currentHeight);

	// Clean up before filling the info again.
	m_pResolutionCombo[1]->DeleteAllItems();
	m_pResolutionCombo[0]->SetItemEnabled(1, false);
	m_pResolutionCombo[0]->SetItemEnabled(2, false);

	// get full video mode list
	vmode_t *plist = NULL;
	int count = 0;
	gameuifuncs->GetVideoModes(&plist, &count);

	const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();

	bool bWindowed = (config.Windowed());
	int desktopWidth, desktopHeight;
	gameuifuncs->GetDesktopResolution(desktopWidth, desktopHeight);

	// iterate all the video modes adding them to the dropdown
	bool bFoundWidescreen = false;
	int selectedItemID = -1;
	for (int i = 0; i < count; i++, plist++)
	{
		char sz[256];
		GetResolutionName(plist, sz, sizeof(sz));

		// don't show modes bigger than the desktop for windowed mode
		if (bWindowed && (plist->width > desktopWidth || plist->height > desktopHeight))
			continue;

		int itemID = -1;
		int iAspectMode = GetScreenAspectMode(plist->width, plist->height);
		if (iAspectMode > 0)
		{
			m_pResolutionCombo[0]->SetItemEnabled(iAspectMode, true);
			bFoundWidescreen = true;
		}

		// filter the list for those matching the current aspect
		if (iAspectMode == m_pResolutionCombo[0]->GetActiveItem())
		{
			itemID = m_pResolutionCombo[1]->AddItem(sz, NULL);
		}

		// try and find the best match for the resolution to be selected
		if (plist->width == currentWidth && plist->height == currentHeight)
		{
			selectedItemID = itemID;
		}
		else if (selectedItemID == -1 && plist->width == config.m_VideoMode.m_Width && plist->height == config.m_VideoMode.m_Height)
		{
			selectedItemID = itemID;
		}
	}

	// disable ratio selection if we can't display widescreen.
	m_pResolutionCombo[0]->SetEnabled(bFoundWidescreen);

	m_nSelectedMode = selectedItemID;
	iCurrentAspectItem = m_pResolutionCombo[0]->GetActiveItem();

	if (selectedItemID != -1)
	{
		m_pResolutionCombo[1]->ActivateItem(selectedItemID);
	}
	else
	{
		char sz[256];
		sprintf(sz, "%d x %d", config.m_VideoMode.m_Width, config.m_VideoMode.m_Height);
		m_pResolutionCombo[1]->SetText(sz);
	}
}

int OptionsMenuVideo::iItems(int iIndex)
{
	if (iIndex == 0)
		return 8;
	else
		return 6;
}

void OptionsMenuVideo::OnThink()
{
	int x, y, w, h;
	m_pSlider->GetSize(w, h);
	m_pSlider->GetNobPos(x, y);

	m_pSliderGUI->PositionOverlay(w, h, x, y);

	int iValueSlider = m_pSlider->GetValue();
	m_pSliderInfo->SetText(VarArgs("Gamma/Brightness: %i", iValueSlider));

	m_pFOVSlider->GetSize(w, h);
	m_pFOVSlider->GetNobPos(x, y);

	m_pFOVSliderGUI->PositionOverlay(w, h, x, y);

	int iValueFOV = m_pFOVSlider->GetValue();
	m_pFOVSliderInfo->SetText(VarArgs("Field of View: %i", iValueFOV));

	// Apply FOV
	int iCurrValue = m_pFOVSlider->GetValue();
	ConVar *fov_var = cvar->FindVar("fov_desired");
	if (fov_var)
	{
		if (fov_var->GetInt() != iCurrValue)
		{
			fov_var->SetValue(iCurrValue);
			engine->ClientCmd_Unrestricted("mat_savechanges\n");
			engine->ClientCmd_Unrestricted("host_writeconfig\n");
		}
	}

	if (!strcmp(m_pVideoScreenModeImg->GetImageName(), "options/videowindowmode_on"))
	{
		m_pSlider->SetVisible(false);
		m_pSliderGUI->SetVisible(false);
		m_pSliderInfo->SetVisible(false);
		m_pSlider->SetEnabled(false);
		m_pSliderGUI->SetEnabled(false);
		m_pSliderInfo->SetEnabled(false);
	}
	else
	{
		m_pSlider->SetVisible(true);
		m_pSliderGUI->SetVisible(true);
		m_pSliderInfo->SetVisible(true);
		m_pSlider->SetEnabled(true);
		m_pSliderGUI->SetEnabled(true);
		m_pSliderInfo->SetEnabled(true);

		// Apply Gamma
		float flCurrValue = (float)((m_pSlider->GetValue() * 0.01) + 1.6f);
		ConVar *slider_var = cvar->FindVar("mat_monitorgamma");
		if (slider_var)
		{
			if (slider_var->GetFloat() != flCurrValue)
			{
				slider_var->SetValue(flCurrValue);
				engine->ClientCmd_Unrestricted("mat_savechanges\n");
				engine->ClientCmd_Unrestricted("host_writeconfig\n");
			}
		}
	}

	if (m_nSelectedMode != m_pResolutionCombo[1]->GetActiveItem())
		m_nSelectedMode = m_pResolutionCombo[1]->GetActiveItem();

	if (iCurrentAspectItem != m_pResolutionCombo[0]->GetActiveItem())
	{
		iCurrentAspectItem = m_pResolutionCombo[0]->GetActiveItem();
		PrepareResolutionList();
		m_pResolutionCombo[1]->ActivateItem(iCurrentAspectItem);
	}
}

void OptionsMenuVideo::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pSlider->SetFgColor(Color(0, 0, 0, 0));
	m_pSlider->SetBgColor(Color(0, 0, 0, 0));
	m_pSliderInfo->SetFgColor(Color(100, 5, 5, 255));
	m_pSliderInfo->SetFont(pScheme->GetFont("TFOInventorySmall"));

	m_pFOVSlider->SetFgColor(Color(0, 0, 0, 0));
	m_pFOVSlider->SetBgColor(Color(0, 0, 0, 0));
	m_pFOVSliderInfo->SetFgColor(Color(100, 5, 5, 255));
	m_pFOVSliderInfo->SetFont(pScheme->GetFont("TFOInventorySmall"));

	for (int i = 0; i <= 1; i++)
	{
		m_pResolutionCombo[i]->SetBgColor(Color(100, 5, 5, 255));
		m_pResolutionCombo[i]->SetAlpha(180);
		m_pResolutionCombo[i]->SetFgColor(Color(100, 5, 5, 255));
		m_pResolutionCombo[i]->SetFont(pScheme->GetFont("TFOInventorySmall"));
		m_pResolutionCombo[i]->SetBorder(NULL);
		m_pResolutionCombo[i]->SetPaintBorderEnabled(false);
		m_pResolutionCombo[i]->SetDisabledBgColor(Color(100, 5, 5, 255));

		m_pResolutionInfo[i]->SetFgColor(Color(100, 5, 5, 255));
		m_pResolutionInfo[i]->SetFont(pScheme->GetFont("TFOInventorySmall"));
	}
}

// Make sure panel is transparent...
void OptionsMenuVideo::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBackgroundType(0);
	BaseClass::PaintBackground();
}

void OptionsMenuVideo::ApplyAllChanges()
{
	float flCurrValue = (float)((m_pSlider->GetValue() * 0.01) + 1.6f);
	ConVar *slider_var = cvar->FindVar("mat_monitorgamma");
	if (slider_var)
		slider_var->SetValue(flCurrValue);

	int iCurrValue = m_pFOVSlider->GetValue();
	ConVar *fov_var = cvar->FindVar("fov_desired");
	if (fov_var)
		fov_var->SetValue(iCurrValue);

	char sz[256];
	if (m_nSelectedMode == -1)
	{
		m_pResolutionCombo[1]->GetText(sz, 256);
	}
	else
	{
		m_pResolutionCombo[1]->GetItemText(m_nSelectedMode, sz, 256);
	}

	int width = 0, height = 0;
	sscanf(sz, "%i x %i", &width, &height);

	// windowed
	bool windowed = (!strcmp(m_pVideoScreenModeImg->GetImageName(), "options/videowindowmode_on")) ? true : false;

	// make sure there is a change
	const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();
	if (config.m_VideoMode.m_Width != width
		|| config.m_VideoMode.m_Height != height
		|| config.Windowed() != windowed)
	{
		// set mode
		char szCmd[256];
		Q_snprintf(szCmd, sizeof(szCmd), "mat_setvideomode %i %i %i\n", width, height, windowed ? 1 : 0);
		engine->ClientCmd_Unrestricted(szCmd);
	}

	// Save
	engine->ClientCmd_Unrestricted("mat_savechanges\n");
	engine->ClientCmd_Unrestricted("host_writeconfig\n");
}

void OptionsMenuVideo::UpdateLayout()
{
	ConVar *slider_var = cvar->FindVar("mat_monitorgamma");
	if (slider_var)
	{
		float flVal = slider_var->GetFloat();

		if (flVal > 0)
			m_pSlider->SetValue(((flVal / 0.01) - (1.6 / 0.01)));
		else
			m_pSlider->SetValue(1.6f);
	}

	ConVar *fov_var = cvar->FindVar("fov_desired");
	if (fov_var)
	{
		int iVal = fov_var->GetInt();

		if (iVal > 0)
			m_pFOVSlider->SetValue(iVal);
		else
			m_pFOVSlider->SetValue(75.0f);
	}

	SetCurrentResolutionComboItem();

	engine->ClientCmd_Unrestricted("host_writeconfig\n"); // update config file...
}

void OptionsMenuVideo::OnCommand(const char *command)
{
	if (!Q_stricmp(command, "CheckWindowed"))
	{
		bool bIsOff = (!strcmp(m_pVideoScreenModeImg->GetImageName(), "options/videowindowmode_off")) ? true : false;

		if (bIsOff)
			m_pVideoScreenModeImg->SetImage("options/videowindowmode_on");
		else
			m_pVideoScreenModeImg->SetImage("options/videowindowmode_off");

		ApplyAllChanges();
	}

	BaseClass::OnCommand(command);
}

void OptionsMenuVideo::SetCurrentResolutionComboItem()
{
	vmode_t *plist = NULL;
	int count = 0;
	gameuifuncs->GetVideoModes(&plist, &count);

	const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();

	int resolution = -1;
	for (int i = 0; i < count; i++, plist++)
	{
		if (plist->width == config.m_VideoMode.m_Width &&
			plist->height == config.m_VideoMode.m_Height)
		{
			resolution = i;
			break;
		}
	}

	if (resolution != -1)
	{
		char sz[256];
		GetResolutionName(plist, sz, sizeof(sz));
		m_pResolutionCombo[1]->SetText(sz);
	}

	if (config.Windowed())
		m_pVideoScreenModeImg->SetImage("options/videowindowmode_on");

	iCurrentAspectItem = m_pResolutionCombo[0]->GetActiveItem();
}