//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Custom Audio Options
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
#include "OptionsMenuAudio.h"
#include <vgui_controls/Button.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/TextImage.h>
#include <vgui_controls/ImageList.h>
#include "vgui_controls/CheckButton.h"
#include "utlvector.h"
#include "fmod_manager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

OptionsMenuAudio::OptionsMenuAudio(vgui::Panel *parent, char const *panelName) : vgui::Panel(parent, panelName)
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
		22,
		70,
	};

	for (int i = 0; i <= 1; i++)
	{
		m_pSlider[i] = vgui::SETUP_PANEL(new vgui::GraphicalSlider(this, "Slider"));
		m_pSliderGUI[i] = vgui::SETUP_PANEL(new vgui::GraphicalOverlay(this, "SliderGUI"));
		m_pSliderInfo[i] = vgui::SETUP_PANEL(new vgui::Label(this, "InfoSlider", ""));

		m_pSlider[i]->SetPos(6, iOptionsMenuPosY[i]);
		m_pSlider[i]->SetRange(0, 100);
		m_pSlider[i]->SetSize(256, 40);
		m_pSlider[i]->SetTickCaptions("Low", "High");
		m_pSlider[i]->SetZPos(45);

		m_pSliderGUI[i]->SetPos(7, (iOptionsMenuPosY[i] - 2));
		m_pSliderGUI[i]->SetSize(300, 40);
		m_pSliderGUI[i]->SetZPos(35);

		m_pSliderInfo[i]->SetZPos(50);
		m_pSliderInfo[i]->SetText("");
		m_pSliderInfo[i]->SetSize(120, 24);
		m_pSliderInfo[i]->SetPos(75, (iOptionsMenuPosY[i] - 22));
		m_pSliderInfo[i]->SetContentAlignment(Label::a_center);
	}

	m_pSpeakerCombo = vgui::SETUP_PANEL(new vgui::ComboBox(this, "SpeakerSetup", 6, false));
	m_pSpeakerCombo->AddItem("#GameUI_Headphones", new KeyValues("SpeakerSetup", "speakers", 0));
	m_pSpeakerCombo->AddItem("#GameUI_2Speakers", new KeyValues("SpeakerSetup", "speakers", 2));
	m_pSpeakerCombo->AddItem("#GameUI_4Speakers", new KeyValues("SpeakerSetup", "speakers", 4));
	m_pSpeakerCombo->AddItem("#GameUI_5Speakers", new KeyValues("SpeakerSetup", "speakers", 5));
	m_pSpeakerCombo->AddItem("#GameUI_7Speakers", new KeyValues("SpeakerSetup", "speakers", 7));

	m_pSpeakerCombo->SetSize(120, 30);
	m_pSpeakerCombo->SetPos(75, 150);

	m_pSpeakerInfo = vgui::SETUP_PANEL(new vgui::Label(this, "SpeakerInfo", ""));
	m_pSpeakerInfo->SetZPos(50);
	m_pSpeakerInfo->SetText("Speaker Settings");
	m_pSpeakerInfo->SetSize(120, 24);
	m_pSpeakerInfo->SetPos(75, 126);
	m_pSpeakerInfo->SetContentAlignment(Label::a_center);

	InvalidateLayout();
}

OptionsMenuAudio::~OptionsMenuAudio()
{
}

const char *OptionsMenuAudio::szGetName(int iIndex)
{
	if (iIndex == 0)
		return "Game Volume";
	else
		return "Music Volume";
}

const char *OptionsMenuAudio::szCheckVars(int iIndex)
{
	if (iIndex == 0)
		return "volume";
	else
		return "snd_musicvolume";
}

void OptionsMenuAudio::OnThink()
{
	for (int i = 0; i <= 1; i++)
	{
		int x, y, w, h;
		m_pSlider[i]->GetSize(w, h);
		m_pSlider[i]->GetNobPos(x, y);

		m_pSliderGUI[i]->PositionOverlay(w, h, x, y);

		int iValueSlider = m_pSlider[i]->GetValue();
		m_pSliderInfo[i]->SetText(VarArgs("%s: %i", szGetName(i), iValueSlider));
	}
}

void OptionsMenuAudio::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	for (int i = 0; i <= 1; i++)
	{
		m_pSlider[i]->SetFgColor(Color(0, 0, 0, 0));
		m_pSlider[i]->SetBgColor(Color(0, 0, 0, 0));
		m_pSliderInfo[i]->SetFgColor(Color(100, 5, 5, 255));
		m_pSliderInfo[i]->SetFont(pScheme->GetFont("TFOInventorySmall"));
	}

	m_pSpeakerInfo->SetFgColor(Color(100, 5, 5, 255));
	m_pSpeakerInfo->SetFont(pScheme->GetFont("TFOInventorySmall"));

	m_pSpeakerCombo->SetBgColor(Color(100, 5, 5, 255));
	m_pSpeakerCombo->SetAlpha(180);
	m_pSpeakerCombo->SetFgColor(Color(100, 5, 5, 255));
	m_pSpeakerCombo->SetFont(pScheme->GetFont("TFOInventorySmall"));
	m_pSpeakerCombo->SetBorder(NULL);
	m_pSpeakerCombo->SetPaintBorderEnabled(false);
	m_pSpeakerCombo->SetDisabledBgColor(Color(100, 5, 5, 255));
}

// Make sure panel is transparent...
void OptionsMenuAudio::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBackgroundType(0);
	BaseClass::PaintBackground();
}

void OptionsMenuAudio::ApplyAllChanges()
{
	for (int i = 0; i <= 1; i++)
	{
		float flCurrValue = (float)(m_pSlider[i]->GetValue() * 0.01);
		ConVar *slider_var = cvar->FindVar(szCheckVars(i));
		if (slider_var)
			slider_var->SetValue(flCurrValue);
	}

	ConVar *speaker_var = cvar->FindVar("snd_surround_speakers");
	if (speaker_var)
	{
		int speakers = m_pSpeakerCombo->GetActiveItemUserData()->GetInt("speakers");
		speaker_var->SetValue(speakers);

		// headphones at high quality get enhanced stereo turned on
		ConVar *dsp_enhance_stereo = cvar->FindVar("dsp_enhance_stereo");
		if (dsp_enhance_stereo)
		{
			if (speakers == 0)
			{
				dsp_enhance_stereo->SetValue(1);
			}
			else
			{
				dsp_enhance_stereo->SetValue(0);
			}
		}
	}

	FMODManager()->UpdateVolume();

	engine->ClientCmd_Unrestricted("host_writeconfig\n");
}

void OptionsMenuAudio::UpdateLayout()
{
	for (int i = 0; i <= 1; i++)
	{
		ConVar *slider_var = cvar->FindVar(szCheckVars(i));
		if (slider_var)
		{
			float flVal = slider_var->GetFloat();

			if (flVal > 0)
				m_pSlider[i]->SetValue((flVal / 0.01));
			else
				m_pSlider[i]->SetValue(0);
		}
	}

	ConVar *speaker_var = cvar->FindVar("snd_surround_speakers");
	if (speaker_var)
	{
		int speakers = speaker_var->GetInt();
		for (int itemID = 0; itemID < m_pSpeakerCombo->GetItemCount(); itemID++)
		{
			KeyValues *kv = m_pSpeakerCombo->GetItemUserData(itemID);
			if (kv && kv->GetInt("speakers") == speakers)
			{
				m_pSpeakerCombo->ActivateItem(itemID);
			}
		}
	}
}