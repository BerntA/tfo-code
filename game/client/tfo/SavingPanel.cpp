//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Save Panel - Your Best Friend. (save station)
//
//=============================================================================//

#include "cbase.h"
#include "SavingPanel.h"
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
#include "hud_numericdisplay.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
#include "KeyValues.h"
#include "GameBase_Client.h"
#include "filesystem.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>

void CSavingPanel::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CSavingPanel::PerformDefaultLayout()
{
	InAlert = false;

	for (int i = 0; i <= 3; i++)
	{
		m_pImgSlot[i]->SetVisible(true);
		m_pImgSlot[i]->SetEnabled(true);

		m_pButtonSlot[i]->SetVisible(true);
		m_pButtonSlot[i]->SetEnabled(true);
		m_pButtonSlot[i]->SetReleasedSound("ui/buttonclick.wav");
		m_pImgSlot[i]->SetImage("savepanel/empty");

		InRolloverSlot[i] = false;
		InSlot[i] = false;

		// Sizes
		m_pButtonSlot[i]->SetSize(80, 140);
		m_pImgSlot[i]->SetSize(256, 256);
	}

	m_pImgYes->SetVisible(false);
	m_pImgNo->SetVisible(false);
	m_pImgYes->SetEnabled(false);
	m_pImgNo->SetEnabled(false);

	m_pButtonYes->SetVisible(false);
	m_pButtonNo->SetVisible(false);
	m_pButtonYes->SetEnabled(false);
	m_pButtonNo->SetEnabled(false);

	m_pImgSure->SetVisible(false);
	m_pImgSure->SetEnabled(false);

	m_pImgSure->SetSize(512, 512);

	//Cross
	m_pImgCross->SetVisible(true);
	m_pImgCross->SetEnabled(true);

	m_pButtonCross->SetVisible(true);
	m_pButtonCross->SetEnabled(true);

	// SET BG
	m_pImgBackground->SetVisible(true);
	m_pImgBackground->SetEnabled(true);

	InRolloverCross = false;

	InRolloverYes = false;
	InRolloverNo = false;

	m_pImgSlots->SetVisible(true);
	m_pImgSlots->SetEnabled(true);

	m_pImgCross->SetImage("panel/menu_x");
	m_pImgYes->SetImage("mainmenu/yes");
	m_pImgNo->SetImage("mainmenu/no");

	// Position Stuff:
	m_pImgYes->SetPos(80, 225);
	m_pImgNo->SetPos(160, 225);
	m_pButtonYes->SetPos(195, 230);
	m_pButtonNo->SetPos(270, 230);
	m_pImgSure->SetPos(0, 0);

	m_pButtonCross->SetPos(455, 400);
	m_pImgCross->SetPos(455, 400);

	m_pImgSlot[0]->SetPos(15, 85);
	m_pImgSlot[1]->SetPos(250, 85);
	m_pImgSlot[2]->SetPos(15, 230);
	m_pImgSlot[3]->SetPos(250, 230);

	m_pButtonSlot[0]->SetPos(105, 130);
	m_pButtonSlot[1]->SetPos(340, 135);
	m_pButtonSlot[2]->SetPos(105, 270);
	m_pButtonSlot[3]->SetPos(340, 270);
	m_pImgSlots->SetPos(0, 40);

	PerformLayout();
}

void CSavingPanel::OnThink()
{
	// Get mouse coords
	int x, y;
	vgui::input()->GetCursorPos(x, y);

	CheckRollovers(x, y);

	SetSize(512, 512);
	MoveToCenterOfScreen();

	m_pImgYes->SetPos(80, 225);
	m_pImgNo->SetPos(160, 225);
	m_pButtonYes->SetPos(195, 230);
	m_pButtonNo->SetPos(270, 230);
	m_pImgSure->SetPos(0, 0);

	m_pButtonCross->SetPos(455, 400);
	m_pImgCross->SetPos(455, 400);

	m_pImgSlot[0]->SetPos(15, 85);
	m_pImgSlot[1]->SetPos(250, 85);
	m_pImgSlot[2]->SetPos(15, 230);
	m_pImgSlot[3]->SetPos(250, 230);

	m_pButtonSlot[0]->SetPos(105, 130);
	m_pButtonSlot[1]->SetPos(340, 135);
	m_pButtonSlot[2]->SetPos(105, 270);
	m_pButtonSlot[3]->SetPos(340, 270);
	m_pImgSlots->SetPos(0, 40);

	if (!InAlert)
	{
		for (int i = 0; i <= 3; i++)
		{
			m_pImgSlot[i]->SetVisible(true);
			m_pImgSlot[i]->SetEnabled(true);
			m_pButtonSlot[i]->SetVisible(true);
			m_pButtonSlot[i]->SetEnabled(true);
		}

		m_pImgSlots->SetVisible(true);
		m_pImgSlots->SetEnabled(true);

		m_pImgYes->SetVisible(false);
		m_pImgNo->SetVisible(false);
		m_pImgYes->SetEnabled(false);
		m_pImgNo->SetEnabled(false);

		m_pButtonYes->SetVisible(false);
		m_pButtonNo->SetVisible(false);
		m_pButtonYes->SetEnabled(false);
		m_pButtonNo->SetEnabled(false);

		m_pImgSure->SetVisible(false);
		m_pImgSure->SetEnabled(false);

		//Cross
		m_pImgCross->SetVisible(true);
		m_pImgCross->SetEnabled(true);
		m_pButtonCross->SetVisible(true);
		m_pButtonCross->SetEnabled(true);
	}
	else if (InAlert)
	{
		for (int i = 0; i <= 3; i++)
		{
			m_pImgSlot[i]->SetVisible(false);
			m_pImgSlot[i]->SetEnabled(false);
			m_pButtonSlot[i]->SetVisible(false);
			m_pButtonSlot[i]->SetEnabled(false);
		}

		m_pImgSlots->SetVisible(false);
		m_pImgSlots->SetEnabled(false);

		m_pImgCross->SetVisible(false);
		m_pImgCross->SetEnabled(false);

		m_pButtonCross->SetVisible(false);
		m_pButtonCross->SetEnabled(false);

		m_pImgYes->SetVisible(true);
		m_pImgNo->SetVisible(true);
		m_pImgYes->SetEnabled(true);
		m_pImgNo->SetEnabled(true);

		m_pButtonYes->SetVisible(true);
		m_pButtonNo->SetVisible(true);
		m_pButtonYes->SetEnabled(true);
		m_pButtonNo->SetEnabled(true);

		m_pImgSure->SetVisible(true);
		m_pImgSure->SetEnabled(true);
	}

	BaseClass::OnThink();
}

KeyValues *CSavingPanel::GetSaveData(const char *szFile)
{
	KeyValues *kvSaveData = new KeyValues("SaveData");
	if (kvSaveData->LoadFromFile(filesystem, VarArgs("resource/data/saves/%s.txt", szFile), "MOD"))
	{
		return kvSaveData;
	}

	kvSaveData->deleteThis();
	Warning("Save File: %s.txt couldn't be found!\n", szFile);

	return NULL;
}

void CSavingPanel::CheckRollovers(int x, int y)
{
	if (!InAlert)
	{
		for (int i = 0; i <= 3; i++)
		{
			const char *szSaveToLoad = "Save1"; // The filename of Save 1-4 in resource/data/saves...

			switch (i)
			{
			case 0:
				szSaveToLoad = "Save1";
				break;
			case 1:
				szSaveToLoad = "Save2";
				break;
			case 2:
				szSaveToLoad = "Save3";
				break;
			case 3:
				szSaveToLoad = "Save4";
				break;
			}

			if (m_pButtonSlot[i]->IsWithin(x, y))
			{
				if (!InRolloverSlot[i])
				{
					vgui::surface()->PlaySound("dialogue/buttonclick.wav");

					KeyValues *kvSvData = GetSaveData(szSaveToLoad);
					if (!kvSvData)
						m_pImgSlot[i]->SetImage("savepanel/empty_over");
					else
					{
						KeyValues *kvSnapShot = kvSvData->FindKey("SnapShot");

						if (kvSnapShot)
						{
							const char *szSnapImg = VarArgs("saves/%s_over", kvSnapShot->GetString());
							char szFullPath[256];
							Q_snprintf(szFullPath, 256, "materials/vgui/%s.vmt", szSnapImg);
							if (filesystem->FileExists(szFullPath, "MOD"))
								m_pImgSlot[i]->SetImage(szSnapImg);
							else
								m_pImgSlot[i]->SetImage("savepanel/unknown_over");
						}
						else
							m_pImgSlot[i]->SetImage("savepanel/empty_over");

						kvSvData->deleteThis();
					}

					InRolloverSlot[i] = true;
				}
			}
			else
			{
				if (InRolloverSlot[i])
				{
					KeyValues *kvSvData = GetSaveData(szSaveToLoad);
					if (!kvSvData)
						m_pImgSlot[i]->SetImage("savepanel/empty");
					else
					{
						KeyValues *kvSnapShot = kvSvData->FindKey("SnapShot");

						if (kvSnapShot)
						{
							const char *szSnapImg = VarArgs("saves/%s", kvSnapShot->GetString());
							char szFullPath[256];
							Q_snprintf(szFullPath, 256, "materials/vgui/%s.vmt", szSnapImg);
							if (filesystem->FileExists(szFullPath, "MOD"))
								m_pImgSlot[i]->SetImage(szSnapImg);
							else
								m_pImgSlot[i]->SetImage("savepanel/unknown");
						}
						else
							m_pImgSlot[i]->SetImage("savepanel/empty");

						kvSvData->deleteThis();
					}

					InRolloverSlot[i] = false;
				}
			}
		}

		if (m_pButtonCross->IsWithin(x, y))
		{
			if (!InRolloverCross)
			{
				m_pImgCross->SetImage("panel/menu_x_over");
				InRolloverCross = true;
				vgui::surface()->PlaySound("dialogue/buttonclick.wav");
			}
		}
		else
		{
			if (InRolloverCross)
			{
				m_pImgCross->SetImage("panel/menu_x");
				InRolloverCross = false;
			}
		}
	}
	else
	{
		if (m_pButtonYes->IsWithin(x, y))
		{
			if (!InRolloverYes)
			{
				vgui::surface()->PlaySound("dialogue/buttonclick.wav");
				m_pImgYes->SetImage("mainmenu/yes_over");
				InRolloverYes = true;
			}
		}
		else
		{
			if (InRolloverYes)
			{
				m_pImgYes->SetImage("mainmenu/yes");
				InRolloverYes = false;
			}
		}

		if (m_pButtonNo->IsWithin(x, y))
		{
			if (!InRolloverNo)
			{
				vgui::surface()->PlaySound("dialogue/buttonclick.wav");
				m_pImgNo->SetImage("mainmenu/no_over");
				InRolloverNo = true;
			}
		}
		else
		{
			if (InRolloverNo)
			{
				m_pImgNo->SetImage("mainmenu/no");
				InRolloverNo = false;
			}
		}
	}
}

void CSavingPanel::OnScreenSizeChanged(int iOldWide, int iOldTall)
{
	BaseClass::OnScreenSizeChanged(iOldWide, iOldTall);
	PerformDefaultLayout();
}

void CSavingPanel::OnShowPanel(bool bShow)
{
	vgui::surface()->PlaySound("hud/read_Paper.wav");
	PerformDefaultLayout();

	if (bShow)
	{
		engine->ClientCmd_Unrestricted("gameui_preventescapetoshow\n");

		// Set default snap shots...
		for (int i = 0; i <= 3; i++)
		{
			char szSaveToLoad[16];
			Q_snprintf(szSaveToLoad, 16, "Save%i", (i + 1));

			KeyValues *kvSvData = GetSaveData(szSaveToLoad);
			if (kvSvData == NULL)
				m_pImgSlot[i]->SetImage("savepanel/empty");
			else
			{
				KeyValues *kvSnapShot = kvSvData->FindKey("SnapShot");

				if (kvSnapShot)
				{
					const char *szSnapImg = VarArgs("saves/%s", kvSnapShot->GetString());
					char szFullPath[256];
					Q_snprintf(szFullPath, 256, "materials/vgui/%s.vmt", szSnapImg);
					if (filesystem->FileExists(szFullPath, "MOD"))
						m_pImgSlot[i]->SetImage(szSnapImg);
					else
						m_pImgSlot[i]->SetImage("savepanel/unknown");
				}
				else
					m_pImgSlot[i]->SetImage("savepanel/empty");

				kvSvData->deleteThis();
			}
		}
	}
	else
	{
		engine->ClientCmd_Unrestricted("gameui_allowescapetoshow\n");
	}
}

CSavingPanel::CSavingPanel(vgui::VPANEL parent) : BaseClass(NULL, "SavingPanel")
{
	SetParent(parent);

	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	SetProportional(false);
	SetTitleBarVisible(false);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);
	SetSizeable(false);
	SetMoveable(false);
	SetVisible(false);

	SetZPos(70);

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/TFOScheme.res", "TFOScheme"));

	// Save, load, continue, poop... 
	for (int i = 0; i <= 3; i++)
	{
		m_pImgSlot[i] = vgui::SETUP_PANEL(new vgui::ImagePanel(this, VarArgs("Slot%i", (i + 1))));
		m_pButtonSlot[i] = vgui::SETUP_PANEL(new vgui::Button(this, VarArgs("btnSlot%i", (i + 1)), ""));

		m_pButtonSlot[i]->SetPaintBorderEnabled(false);
		m_pButtonSlot[i]->SetPaintEnabled(false);
		m_pButtonSlot[i]->SetZPos(150);
		m_pImgSlot[i]->SetZPos(100);

		// Sizes
		m_pButtonSlot[i]->SetSize(80, 140);
		m_pImgSlot[i]->SetSize(256, 256);

		m_pButtonSlot[i]->SetCommand(VarArgs("Slot%i", (i + 1)));
	}

	m_pImgSlots = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Slots"));
	m_pImgCross = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Cross"));

	m_pImgYes = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Yes"));
	m_pImgNo = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "No"));
	m_pImgSure = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Sure"));

	m_pImgBackground = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Backgrounds"));

	m_pImgSlots->SetSize(512, 512);
	m_pImgSlots->SetImage("savepanel/slots_big");
	m_pImgSlots->SetZPos(90);

	// BG All main menu
	m_pImgBackground->SetImage("savepanel/savepanel_bg");
	m_pImgBackground->SetZPos(80);
	m_pImgBackground->SetSize(512, 512);

	// Overwriting comes underneath.
	m_pImgSure->SetSize(512, 512);
	m_pImgSure->SetImage("savepanel/overwrite");
	m_pImgSure->SetZPos(90);

	m_pButtonYes = vgui::SETUP_PANEL(new vgui::Button(this, "btnYes", ""));
	m_pButtonNo = vgui::SETUP_PANEL(new vgui::Button(this, "btnNo", ""));

	m_pButtonYes->SetSize(50, 50);
	m_pButtonYes->SetPaintBorderEnabled(false);
	m_pButtonYes->SetPaintEnabled(false);
	m_pButtonYes->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonYes->SetZPos(150);
	m_pButtonYes->SetCommand("Yes");
	m_pImgYes->SetZPos(100);
	m_pImgYes->SetImage("mainmenu/yes");
	m_pImgYes->SetSize(256, 64);

	m_pButtonNo->SetSize(50, 50);
	m_pButtonNo->SetPaintBorderEnabled(false);
	m_pButtonNo->SetPaintEnabled(false);
	m_pButtonNo->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonNo->SetZPos(150);
	m_pButtonNo->SetCommand("No");
	m_pImgNo->SetZPos(100);
	m_pImgNo->SetImage("mainmenu/no");
	m_pImgNo->SetSize(256, 64);

	// Cross
	m_pButtonCross = vgui::SETUP_PANEL(new vgui::Button(this, "btnCross", ""));
	m_pButtonCross->SetSize(32, 32);
	m_pButtonCross->SetPaintBorderEnabled(false);
	m_pButtonCross->SetPaintEnabled(false);
	m_pImgCross->SetImage("panel/menu_x");
	m_pImgCross->SetSize(32, 32);
	m_pImgCross->SetShouldScaleImage(true);
	m_pImgCross->SetZPos(100);
	m_pButtonCross->SetZPos(200);
	m_pButtonCross->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonCross->SetCommand("CloseX");

	m_pImgYes->SetPos(80, 225);
	m_pImgNo->SetPos(160, 225);
	m_pButtonYes->SetPos(195, 230);
	m_pButtonNo->SetPos(270, 230);
	m_pImgSure->SetPos(0, 0);

	m_pButtonCross->SetPos(455, 400);
	m_pImgCross->SetPos(455, 400);

	m_pImgSlot[0]->SetPos(15, 85);
	m_pImgSlot[1]->SetPos(250, 85);
	m_pImgSlot[2]->SetPos(15, 230);
	m_pImgSlot[3]->SetPos(250, 230);

	m_pButtonSlot[0]->SetPos(105, 130);
	m_pButtonSlot[1]->SetPos(340, 135);
	m_pButtonSlot[2]->SetPos(105, 270);
	m_pButtonSlot[3]->SetPos(340, 270);
	m_pImgSlots->SetPos(0, 40);

	InvalidateLayout();

	PerformDefaultLayout();
}

CSavingPanel::~CSavingPanel()
{
}

void CSavingPanel::OnKeyCodeTyped(vgui::KeyCode code)
{
	if ((code == KEY_ESCAPE) && !InAlert)
	{
		PerformDefaultLayout();

		engine->ClientCmd("tfo_gameui_command OpenSavePanel\n");
	}
	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}

void CSavingPanel::OnCommand(const char* pcCommand)
{
	for (int i = 0; i <= 3; i++)
	{
		if (!Q_stricmp(pcCommand, VarArgs("Slot%i", (i + 1))))
		{
			if (strcmp(m_pImgSlot[i]->GetImageName(), "savepanel/empty") && strcmp(m_pImgSlot[i]->GetImageName(), "savepanel/empty_over"))
			{
				InSlot[i] = true;
				InAlert = true;
			}
			else
			{
				GameBaseClient->SaveGame((i + 1), true);
				engine->ClientCmd("tfo_gameui_command OpenSavePanel\n");
			}
		}
	}

	if (!Q_stricmp(pcCommand, "Yes"))
	{
		if (InAlert)
		{
			for (int i = 0; i <= 3; i++)
			{
				if (InSlot[i])
				{
					InSlot[i] = false;
					InAlert = false;

					GameBaseClient->SaveGame((i + 1), true);
					engine->ClientCmd("tfo_gameui_command OpenSavePanel\n");
				}
			}
		}
	}

	if (!Q_stricmp(pcCommand, "No"))
	{
		if (InAlert)
		{
			for (int i = 0; i <= 3; i++)
			{
				if (InSlot[i])
				{
					InSlot[i] = false;
				}
			}

			InAlert = false;
		}
	}

	if (!Q_stricmp(pcCommand, "CloseX"))
	{
		engine->ClientCmd("tfo_gameui_command OpenSavePanel\n");
	}
}