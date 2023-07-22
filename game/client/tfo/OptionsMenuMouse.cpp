//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Custom Mouse Options
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
#include "OptionsMenuMouse.h"
#include <vgui_controls/Button.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/TextImage.h>
#include <vgui_controls/ImageList.h>
#include "vgui_controls/CheckButton.h"
#include "utlvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

const char *szNames[] =
{
	"Filter",
	"Reverse",
};

const char *szImagesOff[] =
{
	"options/mousefilter_off",
	"options/mousereverse_off",
};

const char *szImagesOn[] =
{
	"options/mousefilter_on",
	"options/mousereverse_on",
};

const char *szCheckVars[] =
{
	"m_filter",
	"m_pitch",
};

OptionsMenuMouse::OptionsMenuMouse(vgui::Panel *parent, char const *panelName) : vgui::Panel(parent, panelName)
{
	AddActionSignalTarget(this);
	SetParent(parent);
	SetName(panelName);

	SetSize(285, 315);

	SetMouseInputEnabled(true);
	SetKeyBoardInputEnabled(true);
	SetProportional(false);

	SetScheme("TFOScheme");

	int iOptionsMenuPosY[] =
	{
		22,
		67,
		150,
	};

	for (int i = 0; i <= 1; i++)
	{
		m_pMouseBtnOpts[i] = vgui::SETUP_PANEL(new vgui::Button(this, szNames[i], ""));
		m_pMouseImgOpts[i] = vgui::SETUP_PANEL(new vgui::ImagePanel(this, szNames[i]));

		m_pMouseBtnOpts[i]->AddActionSignalTarget(this);
		m_pMouseBtnOpts[i]->SetSize(256, 28);
		m_pMouseBtnOpts[i]->SetPaintBorderEnabled(false);
		m_pMouseBtnOpts[i]->SetPaintEnabled(false);
		m_pMouseBtnOpts[i]->SetReleasedSound("ui/buttonclick.wav");
		m_pMouseBtnOpts[i]->SetZPos(30);
		m_pMouseBtnOpts[i]->SetCommand(VarArgs("Check%i", (i + 1)));

		m_pMouseImgOpts[i]->SetZPos(20);
		m_pMouseImgOpts[i]->SetImage(szImagesOff[i]);
		m_pMouseImgOpts[i]->SetSize(256, 64);

		m_pMouseBtnOpts[i]->SetPos(6, iOptionsMenuPosY[i]);
		m_pMouseImgOpts[i]->SetPos(6, (iOptionsMenuPosY[i] - 12));
	}

	m_pSensivity = vgui::SETUP_PANEL(new vgui::GraphicalSlider(this, "MouseSliderSensitivity"));
	m_pSensivity->SetPos(6, iOptionsMenuPosY[2]);
	m_pSensivity->SetRange(1, 20);
	m_pSensivity->SetSize(256, 40);
	m_pSensivity->SetTickCaptions("Low", "High");
	m_pSensivity->SetZPos(45);

	m_pSliderOverlay = vgui::SETUP_PANEL(new vgui::GraphicalOverlay(this, "MouseSliderOverlay"));
	m_pSliderOverlay->SetPos(7, (iOptionsMenuPosY[2] - 2));
	m_pSliderOverlay->SetSize(300, 40);
	m_pSliderOverlay->SetZPos(35);

	m_pInfoSensivity = vgui::SETUP_PANEL(new vgui::Label(this, "InfoSensivity", ""));
	m_pInfoSensivity->SetZPos(50);
	m_pInfoSensivity->SetText("");
	m_pInfoSensivity->SetSize(130, 24);
	m_pInfoSensivity->SetPos(70, 128);
	m_pInfoSensivity->SetContentAlignment(Label::a_center);

	InvalidateLayout();
}

// On deletion of player class / vgui.
OptionsMenuMouse::~OptionsMenuMouse()
{
}

void OptionsMenuMouse::OnThink()
{
	int x, y, w, h;

	m_pSensivity->GetSize(w, h);
	m_pSensivity->GetNobPos(x, y);

	m_pSliderOverlay->PositionOverlay(w, h, x, y);

	m_pInfoSensivity->SetText(VarArgs("Sensitivity: %i", m_pSensivity->GetValue()));
}

void OptionsMenuMouse::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	//BaseClass::ApplySchemeSettings(pScheme);

	m_pInfoSensivity->SetFgColor(Color(100, 5, 5, 255));
	m_pInfoSensivity->SetFont(pScheme->GetFont("TFOInventorySmall"));
	m_pSensivity->SetFgColor(Color(0, 0, 0, 0));
	m_pSensivity->SetBgColor(Color(0, 0, 0, 0));
}

// Make sure panel is transparent...
void OptionsMenuMouse::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBackgroundType(0);
	BaseClass::PaintBackground();
}

void OptionsMenuMouse::ApplyAllChanges()
{
	ConVar *slider_var = cvar->FindVar("sensitivity");
	if (slider_var)
	{
		slider_var->SetValue(m_pSensivity->GetValue());
	}

	engine->ClientCmd_Unrestricted("host_writeconfig\n");
}

void OptionsMenuMouse::UpdateLayout()
{
	for (int i = 0; i <= 1; i++)
	{
		ConVar *mouse_var = cvar->FindVar(szCheckVars[i]);
		if (mouse_var)
		{
			if (i == 0)
			{
				bool bIsChecked = (mouse_var->GetInt() > 0) ? true : false;

				if (bIsChecked)
					m_pMouseImgOpts[i]->SetImage(szImagesOn[i]);
				else
					m_pMouseImgOpts[i]->SetImage(szImagesOff[i]);
			}
			else
			{

				bool bIsChecked = (mouse_var->GetFloat() >= 0) ? true : false;

				if (bIsChecked)
					m_pMouseImgOpts[i]->SetImage(szImagesOff[i]);
				else
					m_pMouseImgOpts[i]->SetImage(szImagesOn[i]);
			}
		}

		ConVar *slider_var = cvar->FindVar("sensitivity");
		if (slider_var)
		{
			m_pSensivity->SetValue(slider_var->GetInt());
		}
	}
}

void OptionsMenuMouse::OnCommand(const char *command)
{
	for (int i = 0; i <= 1; i++)
	{
		if (!Q_stricmp(command, VarArgs("Check%i", (i + 1))))
		{
			ConVar *mouse_var = cvar->FindVar(szCheckVars[i]);
			if (mouse_var)
			{
				if (i == 0)
				{
					bool bIsChecked = (mouse_var->GetInt() > 0) ? true : false;
					mouse_var->SetValue(!bIsChecked);
				}
				else
				{
					bool bIsChecked = (mouse_var->GetFloat() >= 0) ? true : false;

					if (bIsChecked)
						mouse_var->SetValue("-0.022000");
					else
						mouse_var->SetValue("0.022000");
				}

				UpdateLayout();

				engine->ClientCmd_Unrestricted("host_writeconfig\n"); // update config file...
			}
		}
	}

	BaseClass::OnCommand(command);
}