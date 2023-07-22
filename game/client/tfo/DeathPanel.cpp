//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Pops Up on Death, allows you to restart the game (load latest save if possible) or go back to the main menu.
//
//=============================================================================//

#include "cbase.h"
#include "DeathPanel.h"
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
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
#include "GameBase_Client.h"
#include "hl2_gamerules.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>

void CDeathPanel::PerformLayout()
{
	SetPos(0, 0);
	BaseClass::PerformLayout();
}

void CDeathPanel::PerformDefaultLayout()
{
	InRolloverSlot1 = false;
	InRolloverSlot2 = false;
	ShowLayout(true);

	m_pImgSlot1->SetImage("death/retry");
	m_pImgSlot2->SetImage("death/quit");

	PerformLayout();
}

void CDeathPanel::OnScreenSizeChanged(int iOldWide, int iOldTall)
{
	BaseClass::OnScreenSizeChanged(iOldWide, iOldTall);

	LoadControlSettings("resource/ui/deathpanel.res");
}

void CDeathPanel::OnShowPanel(bool state)
{
	if (state)
	{
		engine->ClientCmd_Unrestricted("gameui_preventescapetoshow\n");
	}
	else
	{
		engine->ClientCmd_Unrestricted("gameui_allowescapetoshow\n");
	}

	SetVisible(state);
}

void CDeathPanel::OnThink()
{
	int x, y;
	vgui::input()->GetCursorPos(x, y);


	CheckRollovers(x, y);

	SetSize(ScreenWidth(), ScreenHeight());
	SetPos(0, 0);
	m_pImgBackground->SetSize(ScreenWidth(), ScreenHeight());

	BaseClass::OnThink();
}

void CDeathPanel::CheckRollovers(int x, int y)
{
	if (m_pButtonSlot1->IsWithin(x, y))
	{
		if (!InRolloverSlot1)
		{
			vgui::surface()->PlaySound("ui/buttonrollover.wav");
			m_pImgSlot1->SetImage("death/retry_over");
			InRolloverSlot1 = true;
		}
	}
	else
	{
		if (InRolloverSlot1)
		{
			m_pImgSlot1->SetImage("death/retry");
			InRolloverSlot1 = false;
		}
	}

	if (m_pButtonSlot2->IsWithin(x, y))
	{
		if (!InRolloverSlot2)
		{
			vgui::surface()->PlaySound("ui/buttonrollover.wav");
			m_pImgSlot2->SetImage("death/quit_over");
			InRolloverSlot2 = true;
		}
	}
	else
	{
		if (InRolloverSlot2)
		{
			m_pImgSlot2->SetImage("death/quit");
			InRolloverSlot2 = false;
		}
	}
}

CDeathPanel::CDeathPanel(vgui::VPANEL parent) : BaseClass(NULL, "DeathPanel")
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

	m_pImgSlot1 = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Slot1"));
	m_pImgSlot2 = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Slot2"));

	m_pImgBackground = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Backgrounds"));

	// Save, load, continue, poop... 
	m_pButtonSlot1 = vgui::SETUP_PANEL(new vgui::Button(this, "btnSlot1", ""));
	m_pButtonSlot2 = vgui::SETUP_PANEL(new vgui::Button(this, "btnSlot2", ""));

	m_pButtonSlot1->SetPaintBorderEnabled(false);
	m_pButtonSlot1->SetPaintEnabled(false);
	m_pButtonSlot1->SetZPos(650);
	m_pImgSlot1->SetZPos(600);
	m_pImgSlot1->SetImage("death/retry");
	m_pImgSlot1->SetShouldScaleImage(true);
	m_pButtonSlot1->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonSlot1->SetCommand("Retry");

	m_pButtonSlot2->SetPaintBorderEnabled(false);
	m_pButtonSlot2->SetPaintEnabled(false);
	m_pButtonSlot2->SetZPos(650);
	m_pImgSlot2->SetZPos(600);
	m_pImgSlot2->SetImage("death/quit");
	m_pImgSlot2->SetShouldScaleImage(true);
	m_pButtonSlot2->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonSlot2->SetCommand("Quit");

	// BG All main menu
	m_pImgBackground->SetImage("death/background");
	m_pImgBackground->SetShouldScaleImage(true);
	m_pImgBackground->SetZPos(500);

	SetScheme("TFOScheme");

	LoadControlSettings("resource/ui/deathpanel.res");

	InvalidateLayout();

	PerformDefaultLayout();
}

CDeathPanel::~CDeathPanel()
{
}

void CDeathPanel::ShowLayout(bool bShow)
{
	m_pImgSlot1->SetEnabled(bShow);
	m_pImgSlot1->SetVisible(bShow);
	m_pImgSlot2->SetEnabled(bShow);
	m_pImgSlot2->SetVisible(bShow);

	m_pImgBackground->SetEnabled(bShow);
	m_pImgBackground->SetVisible(bShow);

	m_pButtonSlot1->SetEnabled(bShow);
	m_pButtonSlot1->SetVisible(bShow);

	m_pButtonSlot2->SetEnabled(bShow);
	m_pButtonSlot2->SetVisible(bShow);
}

void CDeathPanel::OnCommand(const char* pcCommand)
{
	if (!Q_stricmp(pcCommand, "Retry"))
	{
		ShowLayout(false);

		PerformDefaultLayout();

		// Reload will auto reload last save. ( any save )...
		engine->ClientCmd("tfo_gameui_command OpenDeathPanel\n");

		GameBaseClient->MapLoad("", false, true);
	}

	if (!Q_stricmp(pcCommand, "Quit"))
	{
		ShowLayout(false);

		PerformDefaultLayout();

		engine->ClientCmd("tfo_gameui_command OpenDeathPanel\n");

		if (!GameBaseClient->CanLoadMainMenu())
			GameBaseClient->Initialize();
	}
}