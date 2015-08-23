//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Custom Graphic Options
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
#include "OptionsMenuGraphics.h"
#include <vgui_controls/Button.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/TextImage.h>
#include <vgui_controls/ImageList.h>
#include "vgui_controls/CheckButton.h"
#include "igameuifuncs.h"
#include "modes.h"
#include "utlvector.h"
#include "materialsystem/materialsystem_config.h"
#include "inetchannelinfo.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: list of valid dx levels
//-----------------------------------------------------------------------------
int g_DirectXLevels[] =
{
	70,
	80,
	81,
	90,
	95,
};

//-----------------------------------------------------------------------------
// Purpose: returns the string name of a given dxlevel
//-----------------------------------------------------------------------------
void GetNameForDXLevel(int dxlevel, char *name, int bufferSize)
{
	if (dxlevel == 95)
	{
		Q_snprintf(name, bufferSize, "DirectX v9.0+");
	}
	else
	{
		Q_snprintf(name, bufferSize, "DirectX v%.1f", dxlevel / 10.0f);
	}
}

OptionsMenuGraphics::OptionsMenuGraphics(vgui::Panel *parent, char const *panelName) : vgui::Panel(parent, panelName)
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
		0,
		60,
		60,
		120,
		120,
		180,
		180,
		240,
	};

	int iOptionsMenuPosX[] =
	{
		7,
		158,
		7,
		158,
		7,
		158,
		7,
		158,
	};

	m_pSlider = vgui::SETUP_PANEL(new vgui::GraphicalSlider(this, "Slider"));
	m_pSliderGUI = vgui::SETUP_PANEL(new vgui::GraphicalOverlay(this, "SliderGUI"));
	m_pSliderInfo = vgui::SETUP_PANEL(new vgui::Label(this, "InfoSlider", ""));

	m_pSlider->SetPos(6, 268);
	m_pSlider->SetRange(1, 100);
	m_pSlider->SetSize(256, 40);
	m_pSlider->SetTickCaptions("Low", "High");
	m_pSlider->SetZPos(45);

	m_pSliderGUI->SetPos(7, 265);
	m_pSliderGUI->SetSize(300, 40);
	m_pSliderGUI->SetZPos(35);

	m_pSliderInfo->SetZPos(50);
	m_pSliderInfo->SetText("");
	m_pSliderInfo->SetSize(285, 24);
	m_pSliderInfo->SetPos(0, 240);
	m_pSliderInfo->SetContentAlignment(Label::a_center);

	// Set slider to default value!
	ConVar *slider_var = cvar->FindVar("mat_motion_blur_strength");
	if (slider_var)
	{
		int iVal = slider_var->GetInt();

		if (iVal > 0)
			m_pSlider->SetValue(iVal);
		else
			m_pSlider->SetValue(1.0f);
	}

	const char *szName[] =
	{
		"ModelDetail",
		"TextureDetail",
		"AntialiasingMode",
		"FilteringMode",
		"ShadowDetail",
		"WaterDetail",
		"VSync",
		"Multicore"
	};

	for (int i = 0; i <= 7; i++)
	{
		m_pGraphicsCombo[i] = vgui::SETUP_PANEL(new vgui::ComboBox(this, szName[i], iItems(i), false));
		m_pGraphicsComboInfo[i] = vgui::SETUP_PANEL(new vgui::Label(this, VarArgs("%sInfo", szName[i]), ""));

		m_pGraphicsCombo[i]->SetSize(120, 30);
		m_pGraphicsCombo[i]->SetPos(iOptionsMenuPosX[i], iOptionsMenuPosY[i] + 25);
		m_pGraphicsComboInfo[i]->SetZPos(50);
		m_pGraphicsComboInfo[i]->SetText("");
		m_pGraphicsComboInfo[i]->SetSize(120, 24);
		m_pGraphicsComboInfo[i]->SetPos(iOptionsMenuPosX[i], iOptionsMenuPosY[i]);
		m_pGraphicsComboInfo[i]->SetContentAlignment(Label::a_center);
	}

	// Set Texts:
	m_pGraphicsComboInfo[0]->SetText("Model Quality");
	m_pGraphicsComboInfo[1]->SetText("Texture Quality");
	m_pGraphicsComboInfo[2]->SetText("Antialiasing Mode");
	m_pGraphicsComboInfo[3]->SetText("Filtering Mode");
	m_pGraphicsComboInfo[4]->SetText("Shadow Quality");
	m_pGraphicsComboInfo[5]->SetText("Water Quality");
	m_pGraphicsComboInfo[6]->SetText("Wait for VSync");
	m_pGraphicsComboInfo[7]->SetText("Multi-Threadning");

	// Add items

	// Model
	m_pGraphicsCombo[0]->AddItem("#gameui_low", NULL);
	m_pGraphicsCombo[0]->AddItem("#gameui_medium", NULL);
	m_pGraphicsCombo[0]->AddItem("#gameui_high", NULL);

	// Texture
	m_pGraphicsCombo[1]->AddItem("#gameui_low", NULL);
	m_pGraphicsCombo[1]->AddItem("#gameui_medium", NULL);
	m_pGraphicsCombo[1]->AddItem("#gameui_high", NULL);
	m_pGraphicsCombo[1]->AddItem("#gameui_ultra", NULL);

	// Antialias
	m_nNumAAModes = 0;
	m_pGraphicsCombo[2]->AddItem("#GameUI_None", NULL);
	m_nAAModes[m_nNumAAModes].m_nNumSamples = 1;
	m_nAAModes[m_nNumAAModes].m_nQualityLevel = 0;
	m_nNumAAModes++;

	if (materials->SupportsMSAAMode(2))
	{
		m_pGraphicsCombo[2]->AddItem("#GameUI_2X", NULL);
		m_nAAModes[m_nNumAAModes].m_nNumSamples = 2;
		m_nAAModes[m_nNumAAModes].m_nQualityLevel = 0;
		m_nNumAAModes++;
	}

	if (materials->SupportsMSAAMode(4))
	{
		m_pGraphicsCombo[2]->AddItem("#GameUI_4X", NULL);
		m_nAAModes[m_nNumAAModes].m_nNumSamples = 4;
		m_nAAModes[m_nNumAAModes].m_nQualityLevel = 0;
		m_nNumAAModes++;
	}

	if (materials->SupportsMSAAMode(6))
	{
		m_pGraphicsCombo[2]->AddItem("#GameUI_6X", NULL);
		m_nAAModes[m_nNumAAModes].m_nNumSamples = 6;
		m_nAAModes[m_nNumAAModes].m_nQualityLevel = 0;
		m_nNumAAModes++;
	}

	if (materials->SupportsCSAAMode(4, 2))							// nVidia CSAA			"8x"
	{
		m_pGraphicsCombo[2]->AddItem("#GameUI_8X_CSAA", NULL);
		m_nAAModes[m_nNumAAModes].m_nNumSamples = 4;
		m_nAAModes[m_nNumAAModes].m_nQualityLevel = 2;
		m_nNumAAModes++;
	}

	if (materials->SupportsCSAAMode(4, 4))							// nVidia CSAA			"16x"
	{
		m_pGraphicsCombo[2]->AddItem("#GameUI_16X_CSAA", NULL);
		m_nAAModes[m_nNumAAModes].m_nNumSamples = 4;
		m_nAAModes[m_nNumAAModes].m_nQualityLevel = 4;
		m_nNumAAModes++;
	}

	if (materials->SupportsMSAAMode(8))
	{
		m_pGraphicsCombo[2]->AddItem("#GameUI_8X", NULL);
		m_nAAModes[m_nNumAAModes].m_nNumSamples = 8;
		m_nAAModes[m_nNumAAModes].m_nQualityLevel = 0;
		m_nNumAAModes++;
	}

	if (materials->SupportsCSAAMode(8, 2))							// nVidia CSAA			"16xQ"
	{
		m_pGraphicsCombo[2]->AddItem("#GameUI_16XQ_CSAA", NULL);
		m_nAAModes[m_nNumAAModes].m_nNumSamples = 8;
		m_nAAModes[m_nNumAAModes].m_nQualityLevel = 2;
		m_nNumAAModes++;
	}

	// Filtering
	m_pGraphicsCombo[3]->AddItem("#GameUI_Bilinear", NULL);
	m_pGraphicsCombo[3]->AddItem("#GameUI_Trilinear", NULL);
	m_pGraphicsCombo[3]->AddItem("#GameUI_Anisotropic2X", NULL);
	m_pGraphicsCombo[3]->AddItem("#GameUI_Anisotropic4X", NULL);
	m_pGraphicsCombo[3]->AddItem("#GameUI_Anisotropic8X", NULL);
	m_pGraphicsCombo[3]->AddItem("#GameUI_Anisotropic16X", NULL);

	// Shadows
	m_pGraphicsCombo[4]->AddItem("#gameui_low", NULL);
	m_pGraphicsCombo[4]->AddItem("#gameui_medium", NULL);
	m_pGraphicsCombo[4]->AddItem("#gameui_high", NULL);

	// Reflections in water
	m_pGraphicsCombo[5]->AddItem("#gameui_noreflections", NULL);
	m_pGraphicsCombo[5]->AddItem("#gameui_reflectonlyworld", NULL);
	m_pGraphicsCombo[5]->AddItem("#gameui_reflectall", NULL);

	// VSync
	m_pGraphicsCombo[6]->AddItem("#gameui_disabled", NULL);
	m_pGraphicsCombo[6]->AddItem("#gameui_enabled", NULL);

	// Multicore
	m_pGraphicsCombo[7]->AddItem("#gameui_disabled", NULL);
	m_pGraphicsCombo[7]->AddItem("#gameui_enabled", NULL);

	MarkDefaultSettingsAsRecommended();

	InvalidateLayout();
}

OptionsMenuGraphics::~OptionsMenuGraphics()
{
}

int OptionsMenuGraphics::FindMSAAMode(int nAASamples, int nAAQuality)
{
	// Run through the AA Modes supported by the device
	for (int nAAMode = 0; nAAMode < m_nNumAAModes; nAAMode++)
	{
		// If we found the mode that matches what we're looking for, return the index
		if ((m_nAAModes[nAAMode].m_nNumSamples == nAASamples) && (m_nAAModes[nAAMode].m_nQualityLevel == nAAQuality))
		{
			return nAAMode;
		}
	}

	return 0;	// Didn't find what we're looking for, so no AA
}

int OptionsMenuGraphics::iItems(int iIndex)
{
	switch (iIndex)
	{
	case 0:
	{
		return 6;
	}
	case 1:
	{
		return 6;
	}
	case 2:
	{
		return 10;
	}
	case 3:
	{
		return 6;
	}
	case 4:
	{
		return 6;
	}
	case 5:
	{
		return 6;
	}
	case 6:
	{
		return 2;
	}
	case 7:
	{
		return 2;
	}
	default:
	{
		return 6;
	}
	}
}

void OptionsMenuGraphics::SetComboItemAsRecommended(vgui::ComboBox *combo, int iItem)
{
	// get the item text
	wchar_t text[512];
	combo->GetItemText(iItem, text, sizeof(text));

	// append the recommended flag
	wchar_t newText[512];
	_snwprintf(newText, sizeof(newText) / sizeof(wchar_t), L"%s *", text);

	// reset
	combo->UpdateItem(iItem, newText, NULL);
}

void OptionsMenuGraphics::MarkDefaultSettingsAsRecommended()
{
	// Pull in data from dxsupport.cfg database (includes fine-grained per-vendor/per-device config data)
	KeyValues *pKeyValues = new KeyValues("config");
	materials->GetRecommendedConfigurationInfo(0, pKeyValues);

	// Read individual values from keyvalues which came from dxsupport.cfg database
	int nSkipLevels = pKeyValues->GetInt("ConVar.mat_picmip", -1);
	int nAnisotropicLevel = pKeyValues->GetInt("ConVar.mat_forceaniso", 16);
	int nForceTrilinear = pKeyValues->GetInt("ConVar.mat_trilinear", 0);
	int nAASamples = pKeyValues->GetInt("ConVar.mat_antialias", 8);
	int nAAQuality = pKeyValues->GetInt("ConVar.mat_aaquality", 0);
	int nRenderToTextureShadows = pKeyValues->GetInt("ConVar.r_shadowrendertotexture", 1);
	int nShadowDepthTextureShadows = pKeyValues->GetInt("ConVar.r_flashlightdepthtexture", 1);
#ifndef _X360
	int nWaterUseRealtimeReflection = pKeyValues->GetInt("ConVar.r_waterforceexpensive", 1);
#endif
	int nWaterUseEntityReflection = pKeyValues->GetInt("ConVar.r_waterforcereflectentities", 1);
	int nMatVSync = pKeyValues->GetInt("ConVar.mat_vsync", 1);
	int nRootLOD = pKeyValues->GetInt("ConVar.r_rootlod", 0);
	int nMulticore = pKeyValues->GetInt("ConVar.mat_queue_mode", -1);

	SetComboItemAsRecommended(m_pGraphicsCombo[0], 2 - nRootLOD);
	SetComboItemAsRecommended(m_pGraphicsCombo[1], 2 - nSkipLevels);

	switch (nAnisotropicLevel)
	{
	case 2:
		SetComboItemAsRecommended(m_pGraphicsCombo[3], 2);
		break;
	case 4:
		SetComboItemAsRecommended(m_pGraphicsCombo[3], 3);
		break;
	case 8:
		SetComboItemAsRecommended(m_pGraphicsCombo[3], 4);
		break;
	case 16:
		SetComboItemAsRecommended(m_pGraphicsCombo[3], 5);
		break;
	case 0:
	default:

		if (nForceTrilinear != 0)
		{
			SetComboItemAsRecommended(m_pGraphicsCombo[3], 1);
		}
		else
		{
			SetComboItemAsRecommended(m_pGraphicsCombo[3], 0);
		}

		break;
	}

	// Map desired mode to list item number
	int nMSAAMode = FindMSAAMode(nAASamples, nAAQuality);
	SetComboItemAsRecommended(m_pGraphicsCombo[2], nMSAAMode);

	if (nShadowDepthTextureShadows)
		SetComboItemAsRecommended(m_pGraphicsCombo[4], 2);	// Shadow depth mapping (in addition to RTT shadows)
	else if (nRenderToTextureShadows)
		SetComboItemAsRecommended(m_pGraphicsCombo[4], 1);	// RTT shadows
	else
		SetComboItemAsRecommended(m_pGraphicsCombo[4], 0);	// Blobbies

#ifndef _X360
	if (nWaterUseRealtimeReflection)
#endif
	{
		if (nWaterUseEntityReflection)
		{
			SetComboItemAsRecommended(m_pGraphicsCombo[5], 2);
		}
		else
		{
			SetComboItemAsRecommended(m_pGraphicsCombo[5], 1);
		}
	}
#ifndef _X360
	else
	{
		SetComboItemAsRecommended(m_pGraphicsCombo[5], 0);
	}
#endif

	SetComboItemAsRecommended(m_pGraphicsCombo[6], nMatVSync != 0);
	SetComboItemAsRecommended(m_pGraphicsCombo[7], nMulticore != 0);

	pKeyValues->deleteThis();

	// Force TFO Defaults:
	// We don't allow any changes to shader quality, HRD Level, motion blur or color correction. These things must be on/maxed...! Todays GPU's won't have a problem with this.
	//engine->ClientCmd_Unrestricted( "mat_dxlevel 95" ); Don't force this one, some GPU's freak out because of this.
	engine->ClientCmd_Unrestricted("mat_hdr_level 2\n");
	engine->ClientCmd_Unrestricted("mat_reducefillrate 0\n");
	engine->ClientCmd_Unrestricted("mat_colorcorrection 1\n");
	engine->ClientCmd_Unrestricted("mat_motion_blur_enabled 1\n");
	engine->ClientCmd_Unrestricted("mat_motion_blur_forward_enabled 1\n");
	engine->ClientCmd_Unrestricted("mat_savechanges\n");
}

void OptionsMenuGraphics::OnThink()
{
	int x, y, w, h;
	m_pSlider->GetSize(w, h);
	m_pSlider->GetNobPos(x, y);

	m_pSliderGUI->PositionOverlay(w, h, x, y);

	int iValueSlider = m_pSlider->GetValue();
	m_pSliderInfo->SetText(VarArgs("Motion Blur Strength: %i", iValueSlider));

	// Apply Blur Strength
	int iCurrValue = m_pSlider->GetValue();
	ConVar *slider_var = cvar->FindVar("mat_motion_blur_strength");
	if (slider_var)
	{
		if (slider_var->GetInt() != iCurrValue)
		{
			slider_var->SetValue(iCurrValue);
			engine->ClientCmd_Unrestricted("mat_savechanges\n");
			engine->ClientCmd_Unrestricted("host_writeconfig\n");
		}
	}
}

void OptionsMenuGraphics::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pSlider->SetFgColor(Color(0, 0, 0, 0));
	m_pSlider->SetBgColor(Color(0, 0, 0, 0));
	m_pSliderInfo->SetFgColor(Color(100, 5, 5, 255));
	m_pSliderInfo->SetFont(pScheme->GetFont("TFOInventorySmall"));

	for (int i = 0; i <= 7; i++)
	{
		m_pGraphicsCombo[i]->SetBgColor(Color(100, 5, 5, 255));
		m_pGraphicsCombo[i]->SetAlpha(180);
		m_pGraphicsCombo[i]->SetFgColor(Color(100, 5, 5, 255));
		m_pGraphicsCombo[i]->SetFont(pScheme->GetFont("TFOInventorySmall"));
		m_pGraphicsCombo[i]->SetBorder(NULL);
		m_pGraphicsCombo[i]->SetPaintBorderEnabled(false);
		m_pGraphicsCombo[i]->SetDisabledBgColor(Color(100, 5, 5, 255));

		m_pGraphicsComboInfo[i]->SetFgColor(Color(100, 5, 5, 255));
		m_pGraphicsComboInfo[i]->SetFont(pScheme->GetFont("TFOInventorySmall"));
	}
}

// Make sure panel is transparent...
void OptionsMenuGraphics::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBackgroundType(0);
	BaseClass::PaintBackground();
}

void OptionsMenuGraphics::ApplyChangesToConVar(const char *pConVarName, int value)
{
	Assert(cvar->FindVar(pConVarName));
	char szCmd[256];
	Q_snprintf(szCmd, sizeof(szCmd), "%s %d\n", pConVarName, value);
	engine->ClientCmd_Unrestricted(szCmd);
}

void OptionsMenuGraphics::ApplyAllChanges()
{
	int iCurrValue = m_pSlider->GetValue();
	ConVar *slider_var = cvar->FindVar("mat_motion_blur_strength");
	if (slider_var)
		slider_var->SetValue(iCurrValue);

	ApplyChangesToConVar("r_rootlod", 2 - m_pGraphicsCombo[0]->GetActiveItem());
	ApplyChangesToConVar("mat_picmip", 2 - m_pGraphicsCombo[1]->GetActiveItem());

	// reset everything tied to the filtering mode, then the switch sets the appropriate one
	ApplyChangesToConVar("mat_trilinear", false);
	ApplyChangesToConVar("mat_forceaniso", 1);

	switch (m_pGraphicsCombo[3]->GetActiveItem())
	{
	case 0:
		break;
	case 1:
		ApplyChangesToConVar("mat_trilinear", true);
		break;
	case 2:
		ApplyChangesToConVar("mat_forceaniso", 2);
		break;
	case 3:
		ApplyChangesToConVar("mat_forceaniso", 4);
		break;
	case 4:
		ApplyChangesToConVar("mat_forceaniso", 8);
		break;
	case 5:
		ApplyChangesToConVar("mat_forceaniso", 16);
		break;
	}

	// Set the AA convars according to the menu item chosen
	int nActiveAAItem = m_pGraphicsCombo[2]->GetActiveItem();
	ApplyChangesToConVar("mat_antialias", m_nAAModes[nActiveAAItem].m_nNumSamples);
	ApplyChangesToConVar("mat_aaquality", m_nAAModes[nActiveAAItem].m_nQualityLevel);

	if (m_pGraphicsCombo[4]->GetActiveItem() == 0)						// Blobby shadows
	{
		ApplyChangesToConVar("r_shadowrendertotexture", 0);			// Turn off RTT shadows
		ApplyChangesToConVar("r_flashlightdepthtexture", 0);			// Turn off shadow depth textures
	}
	else if (m_pGraphicsCombo[4]->GetActiveItem() == 1)					// RTT shadows only
	{
		ApplyChangesToConVar("r_shadowrendertotexture", 1);			// Turn on RTT shadows
		ApplyChangesToConVar("r_flashlightdepthtexture", 0);			// Turn off shadow depth textures
	}
	else if (m_pGraphicsCombo[4]->GetActiveItem() == 2)					// Shadow depth textures
	{
		ApplyChangesToConVar("r_shadowrendertotexture", 1);			// Turn on RTT shadows
		ApplyChangesToConVar("r_flashlightdepthtexture", 1);			// Turn on shadow depth textures
	}

	switch (m_pGraphicsCombo[5]->GetActiveItem())
	{
	default:
	case 0:
#ifndef _X360
		ApplyChangesToConVar("r_waterforceexpensive", false);
#endif
		ApplyChangesToConVar("r_waterforcereflectentities", false);
		break;
	case 1:
#ifndef _X360
		ApplyChangesToConVar("r_waterforceexpensive", true);
#endif
		ApplyChangesToConVar("r_waterforcereflectentities", false);
		break;
	case 2:
#ifndef _X360
		ApplyChangesToConVar("r_waterforceexpensive", true);
#endif
		ApplyChangesToConVar("r_waterforcereflectentities", true);
		break;
	}

	ApplyChangesToConVar("mat_vsync", m_pGraphicsCombo[6]->GetActiveItem());
	ApplyChangesToConVar("mat_queue_mode", (m_pGraphicsCombo[7]->GetActiveItem() == 0) ? 0 : -1);

	// Save
	engine->ClientCmd_Unrestricted("mat_savechanges\n");
	engine->ClientCmd_Unrestricted("host_writeconfig\n");
}

void OptionsMenuGraphics::UpdateLayout()
{
	ConVar *slider_var = cvar->FindVar("mat_motion_blur_strength");
	if (slider_var)
	{
		int iVal = slider_var->GetInt();

		if (iVal > 0)
			m_pSlider->SetValue(iVal);
		else
			m_pSlider->SetValue(1.0f);
	}

	ConVarRef mat_dxlevel("mat_dxlevel");
	ConVarRef r_rootlod("r_rootlod");
	ConVarRef mat_picmip("mat_picmip");
	ConVarRef mat_trilinear("mat_trilinear");
	ConVarRef mat_forceaniso("mat_forceaniso");
	ConVarRef mat_antialias("mat_antialias");
	ConVarRef mat_aaquality("mat_aaquality");
	ConVarRef mat_vsync("mat_vsync");
	ConVarRef mat_queue_mode("mat_queue_mode");
	ConVarRef r_flashlightdepthtexture("r_flashlightdepthtexture");
#ifndef _X360
	ConVarRef r_waterforceexpensive("r_waterforceexpensive");
#endif
	ConVarRef r_waterforcereflectentities("r_waterforcereflectentities");
	ConVarRef mat_reducefillrate("mat_reducefillrate");
	ConVarRef mat_hdr_level("mat_hdr_level");
	ConVarRef mat_colorcorrection("mat_colorcorrection");
	ConVarRef mat_motion_blur_enabled("mat_motion_blur_enabled");
	ConVarRef r_shadowrendertotexture("r_shadowrendertotexture");

	m_pGraphicsCombo[0]->ActivateItem(2 - clamp(r_rootlod.GetInt(), 0, 2));
	m_pGraphicsCombo[1]->ActivateItem(2 - clamp(mat_picmip.GetInt(), -1, 2));

	if (r_flashlightdepthtexture.GetBool())		// If we're doing flashlight shadow depth texturing...
	{
		r_shadowrendertotexture.SetValue(1);		// ...be sure render to texture shadows are also on
		m_pGraphicsCombo[4]->ActivateItem(2);
	}
	else if (r_shadowrendertotexture.GetBool())	// RTT shadows, but not shadow depth texturing
	{
		m_pGraphicsCombo[4]->ActivateItem(1);
	}
	else	// Lowest shadow quality
	{
		m_pGraphicsCombo[4]->ActivateItem(0);
	}

	switch (mat_forceaniso.GetInt())
	{
	case 2:
		m_pGraphicsCombo[3]->ActivateItem(2);
		break;
	case 4:
		m_pGraphicsCombo[3]->ActivateItem(3);
		break;
	case 8:
		m_pGraphicsCombo[3]->ActivateItem(4);
		break;
	case 16:
		m_pGraphicsCombo[3]->ActivateItem(5);
		break;
	case 0:
	default:
		if (mat_trilinear.GetBool())
		{
			m_pGraphicsCombo[3]->ActivateItem(1);
		}
		else
		{
			m_pGraphicsCombo[3]->ActivateItem(0);
		}
		break;
	}

	// Map convar to item on AA drop-down
	int nAASamples = mat_antialias.GetInt();
	int nAAQuality = mat_aaquality.GetInt();
	int nMSAAMode = FindMSAAMode(nAASamples, nAAQuality);
	m_pGraphicsCombo[2]->ActivateItem(nMSAAMode);

#ifndef _X360
	if (r_waterforceexpensive.GetBool())
#endif
	{
		if (r_waterforcereflectentities.GetBool())
		{
			m_pGraphicsCombo[5]->ActivateItem(2);
		}
		else
		{
			m_pGraphicsCombo[5]->ActivateItem(1);
		}
	}
#ifndef _X360
	else
	{
		m_pGraphicsCombo[5]->ActivateItem(0);
	}
#endif

	m_pGraphicsCombo[6]->ActivateItem(mat_vsync.GetInt());
	m_pGraphicsCombo[7]->ActivateItem((mat_queue_mode.GetInt() < 0) ? 1 : 0);
}