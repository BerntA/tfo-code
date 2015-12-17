//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: New GamUI Main Menu + Override.
// Custom Options is also defined here + a lot of other cool things.
// Unfortunately this main menu is still quite hard coded. This project has been worked on for years so some code has remained rather untouched.
// It was worse before I promise you... Everything has been tweaked and refactored of late but to an extent, its not like I would remake everything.
// Notice: VGUI:Frame panels - Never use a vgui panel inherited from Frame as a child for another Frame or together with multiple Frames. This causes trasparency issues.
// Example: Have one main menu panel (master) inherited from Frame and all the others inherited from Panel = god like.
// Notice2: I've had some hair ripping issues with key presses, as you might know you can hit Escape or Backwards key to return (go back) etc... Some controls swallow key inputs, sometimes key inputs get disabled so be aware...
//
//=============================================================================//

#include "cbase.h"
#include "MainMenu.h"
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
#include "fmod_manager.h"
#include "hl2_gamerules.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
#include "KeyValues.h"
#include "filesystem.h"
#include "GameBase_Client.h"
#include "IGameUIFuncs.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>

// Console Helpers:
extern IGameUIFuncs *gameuifuncs; // for key binding details

// Chapter Select ConVar Lock Chapters!
ConVar cl_chapter_2_unlock("cl_chapter_2_unlock", "0", FCVAR_ARCHIVE, "Chapter 2");
ConVar cl_chapter_3_unlock("cl_chapter_3_unlock", "0", FCVAR_ARCHIVE, "Chapter 3");
ConVar cl_chapter_4_unlock("cl_chapter_4_unlock", "0", FCVAR_ARCHIVE, "Chapter 4");

// TFO HUD Toggling:
ConVar tfo_drawhud("tfo_drawhud", "1", FCVAR_CLIENTDLL, "Hide or Show TFO HUD Only...");

// Base Defs:

const char *szSlotSaves[] =
{
	"Save1",
	"Save2",
	"Save3",
	"Save4",
};

const char *szChapterMapsTFO[] =
{
	"tfo_c1_apartments",
	"tfo_c1_forest",
	"tfo_c2_basement",
	"tfo_c4",
};

int AchievementPositionsX[] =
{
	150,
	205,
	260,
	315,
	150,
	205,
	260,
	315,
	150,
	205,
	260,
	315,
};

int AchievementPositionsY[] =
{
	100,
	100,
	100,
	100,
	155,
	155,
	155,
	155,
	210,
	210,
	210,
	210,
};

int SlotsPositionsX[] =
{
	140,
	280,
	140,
	280,
};

int SlotsPositionsY[] =
{
	80,
	80,
	220,
	220,
};

int SlotsLoadImgPosX[] =
{
	50,
	195,
	50,
	195,
};

int SlotsLoadImgPosY[] =
{
	25,
	25,
	170,
	170,
};

const char *szOptionsMenuImgs[] =
{
	"mainmenu/keyboard",
	"mainmenu/mouse",
	"mainmenu/audio",
	"mainmenu/video",
	"mainmenu/graphics",
	"mainmenu/other",
};

int iOptionsMenuPosY[] =
{
	102,
	147,
	192,
	237,
	282,
	327,
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMainMenu::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pLabelVersion->SetFgColor(Color(122, 73, 57, 255));
	m_pLabelVersion->SetFont(pScheme->GetFont("TFOMenu"));

	m_pFilmGrainStrengthSlider->SetFgColor(Color(0, 0, 0, 0));
	m_pFilmGrainStrengthSlider->SetBgColor(Color(0, 0, 0, 0));
	m_pFilmGrainSliderInfo->SetFgColor(Color(100, 5, 5, 255));
	m_pFilmGrainSliderInfo->SetFont(pScheme->GetFont("TFOInventorySmall"));
}

void CMainMenu::OnShowPanel(bool state)
{
	if (state)
		MoveToFront();

	SetVisible(state);
	SetKeyBoardInputEnabled(state);
	SetMouseInputEnabled(state);

	PerformDefaultLayout();
}

// The panel background image should be square, not rounded.
void CMainMenu::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBackgroundType(0);
	BaseClass::PaintBackground();
}

void CMainMenu::PerformLayout()
{
	MoveToCenterOfScreen();

	BaseClass::PerformLayout();
}

void CMainMenu::PerformDefaultLayout()
{
	// Options
	for (int i = 0; i <= 5; i++)
	{
		m_pImgCustomOptions[i]->SetVisible(false);
		m_pButtonCustomOptions[i]->SetVisible(false);
		m_pButtonCustomOptions[i]->SetEnabled(false);

		m_pImgCustomOptions[i]->SetImage(szOptionsMenuImgs[i]);

		m_pButtonCustomOptions[i]->SetPos(120, iOptionsMenuPosY[i]);
		m_pImgCustomOptions[i]->SetPos(120, (iOptionsMenuPosY[i] - 12));
		InCustomOptionMenu[i] = false;
	}

	// Def Images:
	m_pImgBegin->SetImage("mainmenu/begin");
	m_pImgContinue->SetImage("mainmenu/continue");
	m_pImgCredits->SetImage("mainmenu/credits");
	m_pImgOptions->SetImage("mainmenu/options");
	m_pImgLeave->SetImage("mainmenu/leave");
	m_pImgAchievements->SetImage("mainmenu/achievements");
	m_pImgBack->SetImage("mainmenu/back");
	m_pImgDialogue->SetImage("mainmenu/dialogue");
	m_pImgViewbob->SetImage("mainmenu/viewbob");
	m_pImgMirror->SetImage("mainmenu/mirror");
	m_pImgSubtitles->SetImage("mainmenu/subtitles");
	m_pImgFilmGrain->SetImage("options/filmgrain_off");
	m_pImgAchievementText->SetImage("achievements/none");
	m_pImgAchievementBackground->SetImage("achievements/achievement_bg");
	m_pImgSure->SetImage("mainmenu/quit_ask");
	m_pImgYes->SetImage("mainmenu/yes");
	m_pImgNo->SetImage("mainmenu/no");
	m_pImgGotAll->SetImage("achievements/congrats");
	m_pImgSlots->SetImage("savepanel/slots");
	m_pImgBackground->SetImage("mainmenu/bg");

	// Options Custom
	m_pOptionsKeyboardList->SetVisible(false);
	m_pOptionsMousePanel->SetVisible(false);
	m_pOptionsAudioPanel->SetVisible(false);
	m_pOptionsVideoPanel->SetVisible(false);
	m_pOptionsGraphicsPanel->SetVisible(false);

	m_pButtonBack->SetPos(120, 372);
	m_pImgBack->SetPos(120, 360);

	// TFOptions
	m_pButtonDialogue->SetPos(120, 102);
	m_pImgDialogue->SetPos(120, 90);

	m_pButtonViewbob->SetPos(120, 147);
	m_pImgViewbob->SetPos(120, 135);

	m_pButtonMirror->SetPos(120, 192);
	m_pImgMirror->SetPos(120, 180);

	m_pButtonSubtitles->SetPos(120, 237);
	m_pImgSubtitles->SetPos(120, 225);

	m_pImgFilmGrain->SetPos(120, 270);
	m_pButtonFilmGrain->SetPos(120, 282);

	m_pFilmGrainStrengthSlider->SetPos(120, 360);
	m_pFilmGrainSliderGUI->SetPos(121, 358);
	m_pFilmGrainSliderInfo->SetPos(152, 335);

	// Not InOptions
	m_pButtonContinue->SetPos(120, 117);
	m_pImgContinue->SetPos(120, 105);

	m_pButtonBegin->SetPos(120, 162);
	m_pImgBegin->SetPos(120, 150);

	m_pButtonAchievements->SetPos(120, 207);
	m_pImgAchievements->SetPos(120, 195);

	m_pButtonCredits->SetPos(120, 252);
	m_pImgCredits->SetPos(120, 240);

	m_pButtonOptions->SetPos(120, 297);
	m_pImgOptions->SetPos(120, 285);

	m_pButtonLeave->SetPos(120, 342);
	m_pImgLeave->SetPos(120, 330);

	m_pButtonRedirect->SetPos(110, 440);

	// Achievements
	m_pImgAchievementBackground->SetPos(0, -50);
	m_pImgAchievementText->SetPos(0, -50);

	for (int i = 0; i <= 11; i++)
	{
		m_pImgAchievementItems[i]->SetPos(0, -50);
		m_pImgAchievementItems[i]->SetVisible(false);
		m_pImgAchievementItems[i]->SetEnabled(false);

		m_pButtonAchievementItems[i]->SetPos(AchievementPositionsX[i], AchievementPositionsY[i]);
		m_pButtonAchievementItems[i]->SetVisible(false);
		m_pButtonAchievementItems[i]->SetEnabled(false);
	}

	// Load & Start Slots...
	for (int i = 0; i <= 3; i++)
	{
		m_pImgSlot[i]->SetPos(0, 0);
		m_pButtonSlot[i]->SetPos(0, 0);

		m_pButtonSlot[i]->SetVisible(false);
		m_pButtonSlot[i]->SetEnabled(false);

		m_pImgSlot[i]->SetVisible(false);
		m_pImgSlot[i]->SetEnabled(false);

		m_pButtonSlot[i]->SetArmedSound("ui/buttonrollover.wav");
	}

	// Base / Shared:
	if (InAnyMenu)
		engine->ClientCmd_Unrestricted("gameui_allowescape\n");

	InAnyMenu = false;

	m_pImgGotAll->SetVisible(false);
	m_pImgGotAll->SetEnabled(false);
	m_pImgGotAll->SetPos(0, -50);

	m_pImgYes->SetPos(80, 225);
	m_pImgNo->SetPos(160, 225);
	m_pButtonYes->SetPos(195, 230);
	m_pButtonNo->SetPos(270, 230);
	m_pImgSure->SetPos(0, 0);

	m_pImgSlots->SetPos(0, 0);

	m_pImgAchievementBackground->SetEnabled(false);
	m_pImgAchievementText->SetEnabled(false);
	m_pImgAchievementBackground->SetVisible(false);
	m_pImgAchievementText->SetVisible(false);

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

	InOptions = false;
	InAchievements = false;
	InQuit = false;
	InCredits = false;
	InStart = false;
	InLoad = false;

	m_pImgBackground->SetVisible(true);
	m_pImgBackground->SetEnabled(true);
	m_pImgBackground->SetPos(0, 0);
	m_pImgBackground->SetImage("mainmenu/bg");
	m_pImgBackground->SetZPos(15);

	m_pButtonBack->SetVisible(false);
	m_pButtonBack->SetEnabled(false);
	m_pImgBack->SetVisible(false);
	m_pImgBack->SetEnabled(false);

	m_pImgDialogue->SetVisible(false);
	m_pImgDialogue->SetEnabled(false);
	m_pButtonDialogue->SetVisible(false);
	m_pButtonDialogue->SetEnabled(false);

	m_pImgViewbob->SetVisible(false);
	m_pImgViewbob->SetEnabled(false);
	m_pButtonViewbob->SetVisible(false);
	m_pButtonViewbob->SetEnabled(false);

	m_pImgMirror->SetVisible(false);
	m_pImgMirror->SetEnabled(false);
	m_pButtonMirror->SetVisible(false);
	m_pButtonMirror->SetEnabled(false);

	m_pImgSubtitles->SetVisible(false);
	m_pImgSubtitles->SetEnabled(false);
	m_pButtonSubtitles->SetVisible(false);
	m_pButtonSubtitles->SetEnabled(false);

	m_pImgFilmGrain->SetVisible(false);
	m_pImgFilmGrain->SetEnabled(false);
	m_pButtonFilmGrain->SetVisible(false);
	m_pButtonFilmGrain->SetEnabled(false);

	m_pFilmGrainStrengthSlider->SetVisible(false);
	m_pFilmGrainStrengthSlider->SetEnabled(false);
	m_pFilmGrainSliderGUI->SetVisible(false);
	m_pFilmGrainSliderGUI->SetEnabled(false);
	m_pFilmGrainSliderInfo->SetVisible(false);
	m_pFilmGrainSliderInfo->SetEnabled(false);

	m_pImgContinue->SetVisible(true);
	m_pImgContinue->SetEnabled(true);
	m_pButtonContinue->SetVisible(true);
	m_pButtonContinue->SetEnabled(true);

	m_pImgBegin->SetVisible(true);
	m_pImgBegin->SetEnabled(true);
	m_pButtonBegin->SetVisible(true);
	m_pButtonBegin->SetEnabled(true);

	m_pImgAchievements->SetVisible(true);
	m_pImgAchievements->SetEnabled(true);
	m_pButtonAchievements->SetVisible(true);
	m_pButtonAchievements->SetEnabled(true);

	m_pImgCredits->SetVisible(true);
	m_pImgCredits->SetEnabled(true);
	m_pButtonCredits->SetVisible(true);
	m_pButtonCredits->SetEnabled(true);

	m_pImgOptions->SetVisible(true);
	m_pImgOptions->SetEnabled(true);
	m_pButtonOptions->SetVisible(true);
	m_pButtonOptions->SetEnabled(true);

	m_pImgLeave->SetVisible(true);
	m_pImgLeave->SetEnabled(true);
	m_pButtonLeave->SetVisible(true);
	m_pButtonLeave->SetEnabled(true);

	m_pCreditsPanelList->SetVisible(false);
	m_pCreditsPanelList->SetEnabled(false);

	m_pImgSlots->SetVisible(false);
	m_pImgSlots->SetEnabled(false);

	// Re-Focus this panel!
	Activate();
	RequestFocus();
	MoveToFront();
}

void CMainMenu::OnScreenSizeChanged(int iOldWide, int iOldTall)
{
	BaseClass::OnScreenSizeChanged(iOldWide, iOldTall);
	PerformDefaultLayout();
	MoveToCenterOfScreen();
}

void CMainMenu::OnThink()
{
	MoveToCenterOfScreen();
	engine->ClientCmd_Unrestricted("hideconsole\n");
	//vgui::surface()->MovePopupToBack( GetVPanel() );

	// Get mouse coords
	int x, y;
	vgui::input()->GetCursorPos(x, y);

	int fx, fy; // frame xpos, ypos

	GetPos(fx, fy);

	// Update our Version!
	m_pLabelVersion->SetText("VERSION V2.9");
	m_pLabelVersion->SetPos(286, 452);

	if (!InOptions && !InAchievements && !InQuit && !InCredits && !InLoad && !InStart && !InCustomOptionMenu[0] && !InCustomOptionMenu[1] && !InCustomOptionMenu[2] && !InCustomOptionMenu[3] && !InCustomOptionMenu[4] && !InCustomOptionMenu[5])
	{
		// Base / Shared:
		if (InAnyMenu)
			engine->ClientCmd_Unrestricted("gameui_allowescape\n");

		InAnyMenu = false;

		m_pImgBegin->SetImage((m_pButtonBegin->IsWithin(x, y) ? "mainmenu/begin_over" : "mainmenu/begin"));
		m_pImgContinue->SetImage((m_pButtonContinue->IsWithin(x, y) ? "mainmenu/continue_over" : "mainmenu/continue"));
		m_pImgCredits->SetImage((m_pButtonCredits->IsWithin(x, y) ? "mainmenu/credits_over" : "mainmenu/credits"));
		m_pImgOptions->SetImage((m_pButtonOptions->IsWithin(x, y) ? "mainmenu/options_over" : "mainmenu/options"));
		m_pImgLeave->SetImage((m_pButtonLeave->IsWithin(x, y) ? "mainmenu/leave_over" : "mainmenu/leave"));
		m_pImgAchievements->SetImage((m_pButtonAchievements->IsWithin(x, y) ? "mainmenu/achievements_over" : "mainmenu/achievements"));

		for (int i = 0; i <= 5; i++)
		{
			m_pImgCustomOptions[i]->SetVisible(false);
			m_pButtonCustomOptions[i]->SetVisible(false);
			m_pButtonCustomOptions[i]->SetEnabled(false);
		}

		m_pButtonBack->SetVisible(false);
		m_pButtonBack->SetEnabled(false);
		m_pImgBack->SetVisible(false);
		m_pImgBack->SetEnabled(false);

		m_pImgContinue->SetVisible(true);
		m_pImgContinue->SetEnabled(true);
		m_pButtonContinue->SetVisible(true);
		m_pButtonContinue->SetEnabled(true);

		m_pImgBegin->SetVisible(true);
		m_pImgBegin->SetEnabled(true);
		m_pButtonBegin->SetVisible(true);
		m_pButtonBegin->SetEnabled(true);

		m_pImgAchievements->SetVisible(true);
		m_pImgAchievements->SetEnabled(true);
		m_pButtonAchievements->SetVisible(true);
		m_pButtonAchievements->SetEnabled(true);

		m_pImgCredits->SetVisible(true);
		m_pImgCredits->SetEnabled(true);
		m_pButtonCredits->SetVisible(true);
		m_pButtonCredits->SetEnabled(true);

		m_pImgOptions->SetVisible(true);
		m_pImgOptions->SetEnabled(true);
		m_pButtonOptions->SetVisible(true);
		m_pButtonOptions->SetEnabled(true);

		m_pImgLeave->SetVisible(true);
		m_pImgLeave->SetEnabled(true);
		m_pButtonLeave->SetVisible(true);
		m_pButtonLeave->SetEnabled(true);

		m_pImgGotAll->SetVisible(false);
		m_pImgGotAll->SetEnabled(false);

		// Achiev & Slots
		for (int i = 0; i <= 11; i++)
		{
			m_pImgAchievementItems[i]->SetVisible(false);
			m_pImgAchievementItems[i]->SetEnabled(false);
			m_pButtonAchievementItems[i]->SetVisible(false);
			m_pButtonAchievementItems[i]->SetEnabled(false);
		}

		// Load & Start Slots...
		for (int i = 0; i <= 3; i++)
		{
			m_pButtonSlot[i]->SetVisible(false);
			m_pButtonSlot[i]->SetEnabled(false);
			m_pImgSlot[i]->SetVisible(false);
			m_pImgSlot[i]->SetEnabled(false);
		}

		m_pImgAchievementBackground->SetEnabled(false);
		m_pImgAchievementText->SetEnabled(false);
		m_pImgAchievementBackground->SetVisible(false);
		m_pImgAchievementText->SetVisible(false);

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

		m_pCreditsPanelList->SetVisible(false);
		m_pCreditsPanelList->SetEnabled(false);

		m_pImgSlots->SetVisible(false);
		m_pImgSlots->SetEnabled(false);
	}
	else
	{
		m_pImgContinue->SetVisible(false);
		m_pImgContinue->SetEnabled(false);
		m_pButtonContinue->SetVisible(false);
		m_pButtonContinue->SetEnabled(false);

		m_pImgBegin->SetVisible(false);
		m_pImgBegin->SetEnabled(false);
		m_pButtonBegin->SetVisible(false);
		m_pButtonBegin->SetEnabled(false);

		m_pImgAchievements->SetVisible(false);
		m_pImgAchievements->SetEnabled(false);
		m_pButtonAchievements->SetVisible(false);
		m_pButtonAchievements->SetEnabled(false);

		m_pImgCredits->SetVisible(false);
		m_pImgCredits->SetEnabled(false);
		m_pButtonCredits->SetVisible(false);
		m_pButtonCredits->SetEnabled(false);

		m_pImgOptions->SetVisible(false);
		m_pImgOptions->SetEnabled(false);
		m_pButtonOptions->SetVisible(false);
		m_pButtonOptions->SetEnabled(false);

		m_pImgLeave->SetVisible(false);
		m_pImgLeave->SetEnabled(false);
		m_pButtonLeave->SetVisible(false);
		m_pButtonLeave->SetEnabled(false);
	}

	if (InOptions)
	{
		int iAnyVis = 0;

		for (int i = 0; i <= 5; i++)
		{
			if (InCustomOptionMenu[i])
				iAnyVis++;
		}

		if (iAnyVis <= 0)
		{
			for (int i = 0; i <= 5; i++)
			{
				if (m_pButtonCustomOptions[i]->IsWithin(x, y))
					m_pImgCustomOptions[i]->SetImage(VarArgs("%s_over", szOptionsMenuImgs[i]));
				else
					m_pImgCustomOptions[i]->SetImage(szOptionsMenuImgs[i]);
			}

			m_pImgBack->SetImage((m_pButtonBack->IsWithin(x, y) ? "mainmenu/back_over" : "mainmenu/back"));

			for (int i = 0; i <= 5; i++)
			{
				m_pImgCustomOptions[i]->SetVisible(true);
				m_pButtonCustomOptions[i]->SetVisible(true);
				m_pButtonCustomOptions[i]->SetEnabled(true);
			}

			m_pButtonBack->SetVisible(true);
			m_pButtonBack->SetEnabled(true);
			m_pImgBack->SetVisible(true);
			m_pImgBack->SetEnabled(true);

			ShowBaseOtherOptions(false);
		}
	}

	for (int i = 0; i <= 5; i++)
	{
		if (InCustomOptionMenu[i])
		{
			for (int x = 0; x <= 5; x++)
			{
				m_pImgCustomOptions[x]->SetVisible(false);
				m_pButtonCustomOptions[x]->SetVisible(false);
				m_pButtonCustomOptions[x]->SetEnabled(false);
			}
		}
	}

	if (InCustomOptionMenu[1])
	{
		m_pImgBack->SetImage((m_pButtonBack->IsWithin(x, y) ? "mainmenu/back_over" : "mainmenu/back"));
		m_pOptionsMousePanel->SetVisible(true);
	}
	else
	{
		m_pOptionsMousePanel->SetVisible(false);
	}

	if (InCustomOptionMenu[2])
	{
		m_pImgBack->SetImage((m_pButtonBack->IsWithin(x, y) ? "mainmenu/back_over" : "mainmenu/back"));
		m_pOptionsAudioPanel->SetVisible(true);

	}
	else
	{
		m_pOptionsAudioPanel->SetVisible(false);
	}

	if (InCustomOptionMenu[3])
	{
		m_pImgBack->SetImage((m_pButtonBack->IsWithin(x, y) ? "mainmenu/back_over" : "mainmenu/back"));
		m_pOptionsVideoPanel->SetVisible(true);
	}
	else
	{
		m_pOptionsVideoPanel->SetVisible(false);
	}

	if (InCustomOptionMenu[4])
	{
		m_pImgBack->SetImage((m_pButtonBack->IsWithin(x, y) ? "mainmenu/back_over" : "mainmenu/back"));
		m_pOptionsGraphicsPanel->SetVisible(true);
	}
	else
	{
		m_pOptionsGraphicsPanel->SetVisible(false);
	}

	if (InCustomOptionMenu[0])
	{
		m_pImgBack->SetImage((m_pButtonBack->IsWithin(x, y) ? "mainmenu/back_over" : "mainmenu/back"));
		m_pOptionsKeyboardList->SetVisible(true);
	}
	else
	{
		m_pOptionsKeyboardList->SetVisible(false);
	}

	if (InCustomOptionMenu[5])
	{
		m_pImgBack->SetImage((m_pButtonBack->IsWithin(x, y) ? "mainmenu/back_over" : "mainmenu/back"));

		ShowBaseOtherOptions(true);

		ConVar* tfo_viewbob = cvar->FindVar("cl_viewbob_enabled");
		ConVar* tfo_mirror = cvar->FindVar("cl_player_render_mirror");
		ConVar* tfo_subtitles = cvar->FindVar("cc_subtitles");
		ConVar* tfo_dialogue = cvar->FindVar("cl_dialoguemode");
		ConVar* tfo_filmgrain = cvar->FindVar("tfo_fx_filmgrain");

		// Dynamic image changes depending on the state of the convars above...

		if (tfo_viewbob->GetBool())
			m_pImgViewbob->SetImage("mainmenu/viewbob_over");
		else
			m_pImgViewbob->SetImage("mainmenu/viewbob");

		if (tfo_mirror->GetBool())
			m_pImgMirror->SetImage("mainmenu/mirror");
		else
			m_pImgMirror->SetImage("mainmenu/mirror_over");

		if (tfo_subtitles->GetBool())
			m_pImgSubtitles->SetImage("mainmenu/subtitles");
		else
			m_pImgSubtitles->SetImage("mainmenu/subtitles_over");

		if (tfo_dialogue->GetBool())
			m_pImgDialogue->SetImage("mainmenu/dialogue_over");
		else
			m_pImgDialogue->SetImage("mainmenu/dialogue");

		m_pFilmGrainStrengthSlider->SetVisible(tfo_filmgrain->GetBool());
		m_pFilmGrainStrengthSlider->SetEnabled(tfo_filmgrain->GetBool());
		m_pFilmGrainSliderGUI->SetVisible(tfo_filmgrain->GetBool());
		m_pFilmGrainSliderGUI->SetEnabled(tfo_filmgrain->GetBool());
		m_pFilmGrainSliderInfo->SetVisible(tfo_filmgrain->GetBool());
		m_pFilmGrainSliderInfo->SetEnabled(tfo_filmgrain->GetBool());
		m_pFilmGrainSliderInfo->SetFgColor(Color(100, 5, 5, 255));
		m_pFilmGrainSliderInfo->SetBgColor(Color(255, 255, 255, 0));

		if (tfo_filmgrain->GetBool())
		{
			m_pImgFilmGrain->SetImage("options/filmgrain_on");

			int wz, hz, xz, yz;
			m_pFilmGrainStrengthSlider->GetSize(wz, hz);
			m_pFilmGrainStrengthSlider->GetNobPos(xz, yz);
			m_pFilmGrainSliderGUI->PositionOverlay(wz, hz, xz, yz);
			m_pFilmGrainSliderInfo->SetText(VarArgs("Film Grain Strength: %i", m_pFilmGrainStrengthSlider->GetValue()));
		}
		else
			m_pImgFilmGrain->SetImage("options/filmgrain_off");
	}

	if (InAchievements)
	{
		// Global = Everything no matter what... Achievements must order the images no matter what...
		for (int i = 0; i < _ARRAYSIZE(m_pImgAchievementItems); i++)
			m_pImgAchievementItems[i]->SetImage(GameBaseClient->GetAchievementForGUI(i));

		m_pButtonBack->SetVisible(true);
		m_pButtonBack->SetEnabled(true);
		m_pImgBack->SetVisible(true);
		m_pImgBack->SetEnabled(true);

		int selectedAchievements = 0;
		for (int i = 0; i < _ARRAYSIZE(m_pButtonAchievementItems); i++)
		{
			if (m_pButtonAchievementItems[i]->IsWithin(x, y))
			{
				m_pImgAchievementText->SetImage(VarArgs("achievements/achievement_%i_t", (i + 1)));
				selectedAchievements++;
			}
		}

		if (selectedAchievements <= 0)
			m_pImgAchievementText->SetImage("achievements/none");

		m_pImgBack->SetImage((m_pButtonBack->IsWithin(x, y) ? "mainmenu/back_over" : "mainmenu/back"));

		for (int i = 0; i < _ARRAYSIZE(m_pImgAchievementItems); i++)
		{
			m_pImgAchievementItems[i]->SetVisible(true);
			m_pImgAchievementItems[i]->SetEnabled(true);

			m_pButtonAchievementItems[i]->SetVisible(true);
			m_pButtonAchievementItems[i]->SetEnabled(true);
		}

		m_pImgAchievementBackground->SetEnabled(true);
		m_pImgAchievementText->SetEnabled(true);
		m_pImgAchievementBackground->SetVisible(true);
		m_pImgAchievementText->SetVisible(true);

		if (GameBaseClient->HasAllAchievements())
		{
			m_pImgGotAll->SetVisible(true);
			m_pImgGotAll->SetEnabled(true);
		}
		else
		{
			m_pImgGotAll->SetVisible(false);
			m_pImgGotAll->SetEnabled(false);
		}
	}

	if (InQuit)
	{
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

		m_pImgSure->SetImage("mainmenu/quit_ask");

		m_pImgYes->SetImage((m_pButtonYes->IsWithin(x, y) ? "mainmenu/yes_over" : "mainmenu/yes"));
		m_pImgNo->SetImage((m_pButtonNo->IsWithin(x, y) ? "mainmenu/no_over" : "mainmenu/no"));
	}

	if (InCredits)
	{
		m_pCreditsPanelList->SetVisible(true);
		m_pCreditsPanelList->SetEnabled(true);

		m_pButtonBack->SetVisible(true);
		m_pButtonBack->SetEnabled(true);
		m_pImgBack->SetVisible(true);
		m_pImgBack->SetEnabled(true);
		m_pImgBack->SetImage((m_pButtonBack->IsWithin(x, y) ? "mainmenu/back_over" : "mainmenu/back"));
	}

	if (InLoad)
	{
		m_pButtonBack->SetVisible(true);
		m_pButtonBack->SetEnabled(true);
		m_pImgBack->SetVisible(true);
		m_pImgBack->SetEnabled(true);

		m_pButtonBack->SetVisible(true);
		m_pButtonBack->SetEnabled(true);
		m_pImgBack->SetVisible(true);
		m_pImgBack->SetEnabled(true);

		m_pImgBack->SetImage((m_pButtonBack->IsWithin(x, y) ? "mainmenu/back_over" : "mainmenu/back"));

		bool bHasSaves = false;
		for (int i = 0; i < _ARRAYSIZE(szSlotSaves); i++)
		{
			if (filesystem->FileExists(VarArgs("resource/data/saves/%s.txt", szSlotSaves[i]), "MOD"))
			{
				bHasSaves = true;
			}
		}

		if (!bHasSaves)
		{
			m_pImgSure->SetVisible(true);
			m_pImgSure->SetEnabled(true);

			m_pImgSure->SetImage("savepanel/nosaves");

			m_pImgSlots->SetEnabled(false);
			m_pImgSlots->SetVisible(false);

			for (int i = 0; i < _ARRAYSIZE(m_pButtonSlot); i++)
			{
				m_pButtonSlot[i]->SetVisible(false);
				m_pButtonSlot[i]->SetEnabled(false);
				m_pImgSlot[i]->SetVisible(false);
				m_pImgSlot[i]->SetEnabled(false);
			}
		}
		else
		{
			for (int i = 0; i < _ARRAYSIZE(m_pButtonSlot); i++)
			{
				m_pButtonSlot[i]->SetVisible(true);
				m_pButtonSlot[i]->SetEnabled(true);
				m_pImgSlot[i]->SetVisible(true);
				m_pImgSlot[i]->SetEnabled(true);

				if (!strcmp(m_pImgSlot[i]->GetImageName(), "savepanel/empty") || !strcmp(m_pImgSlot[i]->GetImageName(), "savepanel/empty_over"))
				{
					m_pButtonSlot[i]->SetReleasedSound("common/wpn_denyselect.wav");
					m_pButtonSlot[i]->SetArmedSound("common/null.wav");
				}
				else
				{
					m_pButtonSlot[i]->SetReleasedSound("ui/buttonclick.wav");
					m_pButtonSlot[i]->SetArmedSound("ui/buttonrollover.wav");
				}

				m_pImgSlot[i]->SetPos(SlotsLoadImgPosX[i], SlotsLoadImgPosY[i]);
				m_pButtonSlot[i]->SetPos(SlotsPositionsX[i], SlotsPositionsY[i]);
			}

			m_pImgSlots->SetEnabled(true);
			m_pImgSlots->SetVisible(true);

			m_pImgSure->SetVisible(false);
			m_pImgSure->SetEnabled(false);

			// Check for save hovers
			CheckSaveRollovers(x, y);
		}
	}

	if (InStart)
	{
		m_pImgContinue->SetVisible(false);
		m_pImgContinue->SetEnabled(false);
		m_pButtonContinue->SetVisible(false);
		m_pButtonContinue->SetEnabled(false);

		for (int i = 0; i < _ARRAYSIZE(m_pButtonSlot); i++)
		{
			m_pButtonSlot[i]->SetVisible(true);
			m_pButtonSlot[i]->SetEnabled(true);
			m_pImgSlot[i]->SetVisible(true);
			m_pImgSlot[i]->SetEnabled(true);
		}

		m_pButtonBack->SetVisible(true);
		m_pButtonBack->SetEnabled(true);
		m_pImgBack->SetVisible(true);
		m_pImgBack->SetEnabled(true);

		CheckSaveRollovers(x, y);
		m_pImgBack->SetImage((m_pButtonBack->IsWithin(x, y) ? "mainmenu/back_over" : "mainmenu/back"));

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

		m_pButtonSlot[0]->SetReleasedSound("ui/buttonclick.wav");

		for (int i = 0; i < _ARRAYSIZE(m_pButtonSlot); i++)
		{
			ConVar *chapter_var = cvar->FindVar(VarArgs("cl_chapter_%i_unlock", (i + 1)));
			if (chapter_var)
			{
				if (chapter_var->GetBool())
				{
					m_pButtonSlot[i]->SetReleasedSound("ui/buttonclick.wav");
					m_pButtonSlot[i]->SetArmedSound("ui/buttonrollover.wav");
				}
				else
				{
					m_pButtonSlot[i]->SetReleasedSound("common/wpn_denyselect.wav");
					m_pButtonSlot[i]->SetArmedSound("common/null.wav");
				}
			}

			m_pImgSlot[i]->SetPos(0, 0);
			m_pButtonSlot[i]->SetPos(SlotsPositionsX[i], SlotsPositionsY[i]);
		}
	}

	BaseClass::OnThink();
}

void CMainMenu::CheckSaveRollovers(int x, int y)
{
	for (int i = 0; i <= 3; i++)
	{
		if (InLoad)
		{
			if (!strcmp(m_pImgSlot[i]->GetImageName(), "savepanel/empty") || !strcmp(m_pImgSlot[i]->GetImageName(), "savepanel/empty_over"))
				continue;
		}

		if (InStart)
		{
			ConVar *chapter_var = cvar->FindVar(VarArgs("cl_chapter_%i_unlock", (i + 1)));
			if (chapter_var)
			{
				if (!chapter_var->GetBool())
					continue;
			}
		}

		bool bIsSelected = m_pButtonSlot[i]->IsWithin(x, y);

		if (InStart)
		{
			m_pImgSlot[i]->SetImage(bIsSelected ? VarArgs("mainmenu/chapter%i_over", (i + 1)) : VarArgs("mainmenu/chapter%i", (i + 1)));
		}
		else if (InLoad)
		{
			KeyValues *kvSvData = GetSaveData(szSlotSaves[i]);
			if (!kvSvData)
				m_pImgSlot[i]->SetImage(bIsSelected ? "savepanel/empty_over" : "savepanel/empty");
			else
			{
				char szFullPath[80];

				if (bIsSelected)
					Q_snprintf(szFullPath, 80, "materials/vgui/saves/%s_over.vmt", kvSvData->GetString("SnapShot"));
				else
					Q_snprintf(szFullPath, 80, "materials/vgui/saves/%s.vmt", kvSvData->GetString("SnapShot"));

				if (filesystem->FileExists(szFullPath, "MOD"))
					m_pImgSlot[i]->SetImage(bIsSelected ? VarArgs("saves/%s_over", kvSvData->GetString("SnapShot")) : VarArgs("saves/%s", kvSvData->GetString("SnapShot")));
				else
					m_pImgSlot[i]->SetImage(bIsSelected ? "savepanel/unknown_over" : "savepanel/unknown");

				kvSvData->deleteThis();
			}
		}
	}
}

bool CMainMenu::InGame()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if (pPlayer)
		return true;
	else
		return false;
}

KeyValues *CMainMenu::GetSaveData(const char *szFile)
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

void CMainMenu::DeleteSaveDataFile(const char *szFile)
{
	// Remove our stored text file.
	const char *szPath = VarArgs("resource/data/saves/%s.txt", szFile);
	if (filesystem->FileExists(szPath, "MOD"))
	{
		filesystem->RemoveFile(szPath, "MOD");
	}

	// Remove actual save.
	const char *szSavePath = VarArgs("save/%s.sav", szFile);
	if (filesystem->FileExists(szSavePath, "MOD"))
	{
		filesystem->RemoveFile(szSavePath, "MOD");
	}
}

void CMainMenu::ShowBaseOtherOptions(bool bShow)
{
	m_pImgDialogue->SetVisible(bShow);
	m_pImgDialogue->SetEnabled(bShow);
	m_pButtonDialogue->SetVisible(bShow);
	m_pButtonDialogue->SetEnabled(bShow);

	m_pImgViewbob->SetVisible(bShow);
	m_pImgViewbob->SetEnabled(bShow);
	m_pButtonViewbob->SetVisible(bShow);
	m_pButtonViewbob->SetEnabled(bShow);

	m_pImgMirror->SetVisible(bShow);
	m_pImgMirror->SetEnabled(bShow);
	m_pButtonMirror->SetVisible(bShow);
	m_pButtonMirror->SetEnabled(bShow);

	m_pImgSubtitles->SetVisible(bShow);
	m_pImgSubtitles->SetEnabled(bShow);
	m_pButtonSubtitles->SetVisible(bShow);
	m_pButtonSubtitles->SetEnabled(bShow);

	m_pImgFilmGrain->SetVisible(bShow);
	m_pImgFilmGrain->SetEnabled(bShow);
	m_pButtonFilmGrain->SetVisible(bShow);
	m_pButtonFilmGrain->SetEnabled(bShow);

	if (!bShow)
	{
		m_pFilmGrainStrengthSlider->SetVisible(false);
		m_pFilmGrainStrengthSlider->SetEnabled(false);
		m_pFilmGrainSliderGUI->SetVisible(false);
		m_pFilmGrainSliderGUI->SetEnabled(false);
		m_pFilmGrainSliderInfo->SetVisible(false);
		m_pFilmGrainSliderInfo->SetEnabled(false);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMainMenu::CMainMenu(vgui::VPANEL parent) : BaseClass(NULL, "MainMenu")
{
	//MakePopup();

	SetParent(parent);

	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	SetTitleBarVisible(false);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);
	SetSizeable(false);
	SetMoveable(false);
	SetProportional(false);
	SetVisible(false);

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/TFOScheme.res", "TFOScheme"));

	SetSize(512, 512);
	SetZPos(10);

	// Controls:
	m_pImgBegin = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Begin"));
	m_pImgContinue = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Continue"));
	m_pImgCredits = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Credits"));
	m_pImgOptions = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Options"));
	m_pImgLeave = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Leave"));
	m_pImgAchievements = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Achievements"));
	m_pImgBack = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Back"));
	m_pImgBackground = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Background"));

	m_pImgSlots = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Slots"));

	m_pImgGotAll = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "GotAll"));
	m_pImgAchievementText = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "AchievementText"));
	m_pImgAchievementBackground = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "AchievementBackground"));

	// CUSTOM Options
	for (int i = 0; i <= 5; i++)
	{
		m_pImgCustomOptions[i] = vgui::SETUP_PANEL(new vgui::ImagePanel(this, VarArgs("IMGOptionsCustom%i", (i + 1))));
		m_pButtonCustomOptions[i] = vgui::SETUP_PANEL(new vgui::Button(this, VarArgs("BTNOptionsCustom%i", (i + 1)), ""));
		m_pButtonCustomOptions[i]->AddActionSignalTarget(this);
		m_pButtonCustomOptions[i]->SetSize(256, 28);
		m_pButtonCustomOptions[i]->SetPaintBorderEnabled(false);
		m_pButtonCustomOptions[i]->SetPaintEnabled(false);
		m_pButtonCustomOptions[i]->SetArmedSound("ui/buttonrollover.wav");
		m_pButtonCustomOptions[i]->SetReleasedSound("ui/buttonclick.wav");
		m_pButtonCustomOptions[i]->SetZPos(30);
		m_pButtonCustomOptions[i]->SetCommand(VarArgs("CustomOption%i", (i + 1)));

		m_pImgCustomOptions[i]->SetImage(szOptionsMenuImgs[i]);
		m_pImgCustomOptions[i]->SetSize(256, 64);
		m_pImgCustomOptions[i]->SetZPos(20);
	}

	// Version:
	m_pLabelVersion = vgui::SETUP_PANEL(new vgui::Label(this, "VersionText", ""));
	m_pLabelVersion->SetZPos(800);
	m_pLabelVersion->SetSize(110, 25);
	m_pLabelVersion->SetVisible(true);

	//TFO Options
	m_pImgDialogue = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Dialogue"));
	m_pImgViewbob = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Viewbob"));
	m_pImgMirror = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Mirror"));
	m_pImgSubtitles = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Subtitles"));
	m_pImgFilmGrain = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "FilmGrain"));

	// Quit
	m_pImgYes = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Yes"));
	m_pImgNo = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "No"));
	m_pImgSure = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Sure"));

	// Credits
	m_pCreditsPanelList = vgui::SETUP_PANEL(new vgui::CreditsListing(this, "CreditsImage"));

	// New game
	m_pButtonBegin = vgui::SETUP_PANEL(new vgui::Button(this, "btnBegin", ""));
	m_pButtonBegin->SetSize(256, 28);
	m_pButtonBegin->SetPaintBorderEnabled(false);
	m_pButtonBegin->SetPaintEnabled(false);
	m_pButtonBegin->SetArmedSound("ui/buttonrollover.wav");
	m_pImgBegin->SetImage("mainmenu/begin");
	m_pImgBegin->SetSize(256, 64);
	m_pButtonBegin->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonBegin->SetZPos(30);
	m_pButtonBegin->SetCommand("Begin");
	m_pButtonBegin->AddActionSignalTarget(this);
	m_pImgBegin->SetZPos(20);

	// Continue From Last Saved Game
	m_pButtonContinue = vgui::SETUP_PANEL(new vgui::Button(this, "btnContinue", ""));
	m_pButtonContinue->SetSize(256, 28);
	m_pButtonContinue->SetPaintBorderEnabled(false);
	m_pButtonContinue->SetPaintEnabled(false);
	m_pButtonContinue->SetArmedSound("ui/buttonrollover.wav");
	m_pImgContinue->SetImage("mainmenu/continue");
	m_pImgContinue->SetSize(256, 64);
	m_pButtonContinue->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonContinue->SetZPos(30);
	m_pButtonContinue->SetCommand("Continue");
	m_pButtonContinue->AddActionSignalTarget(this);
	m_pImgContinue->SetZPos(20);

	// Credits

	m_pButtonCredits = vgui::SETUP_PANEL(new vgui::Button(this, "btnCredits", ""));
	m_pButtonCredits->SetSize(256, 28);
	m_pButtonCredits->SetPaintBorderEnabled(false);
	m_pButtonCredits->SetPaintEnabled(false);
	m_pButtonCredits->AddActionSignalTarget(this);
	m_pButtonCredits->SetArmedSound("ui/buttonrollover.wav");
	m_pImgCredits->SetImage("mainmenu/credits");
	m_pImgCredits->SetSize(256, 64);
	m_pButtonCredits->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonCredits->SetZPos(30);
	m_pButtonCredits->SetCommand("Credits");
	m_pButtonCredits->AddActionSignalTarget(this);
	m_pImgCredits->SetZPos(20);

	// Options
	m_pButtonOptions = vgui::SETUP_PANEL(new vgui::Button(this, "btnOptions", ""));
	m_pButtonOptions->SetSize(256, 28);
	m_pButtonOptions->SetPaintBorderEnabled(false);
	m_pButtonOptions->SetArmedSound("ui/buttonrollover.wav");
	m_pButtonOptions->SetPaintEnabled(false);
	m_pImgOptions->SetImage("mainmenu/options");
	m_pImgOptions->SetSize(256, 64);
	m_pButtonOptions->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonOptions->SetZPos(30);
	m_pButtonOptions->SetCommand("Options");
	m_pButtonOptions->AddActionSignalTarget(this);
	m_pImgOptions->SetZPos(20);

	// Leave
	m_pButtonLeave = vgui::SETUP_PANEL(new vgui::Button(this, "btnLeave", ""));
	m_pButtonLeave->SetSize(256, 28);
	m_pButtonLeave->SetPaintBorderEnabled(false);
	m_pButtonLeave->SetPaintEnabled(false);
	m_pButtonLeave->SetArmedSound("ui/buttonrollover.wav");
	m_pImgLeave->SetImage("mainmenu/leave");
	m_pImgLeave->SetSize(256, 64);
	m_pButtonLeave->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonLeave->SetZPos(30);
	m_pButtonLeave->SetCommand("Leave");
	m_pButtonLeave->AddActionSignalTarget(this);
	m_pImgLeave->SetZPos(20);

	// Achievements
	m_pButtonAchievements = vgui::SETUP_PANEL(new vgui::Button(this, "btnAchievements", ""));
	m_pButtonAchievements->SetSize(256, 28);
	m_pButtonAchievements->SetPaintBorderEnabled(false);
	m_pButtonAchievements->SetPaintEnabled(false);
	m_pButtonAchievements->SetArmedSound("ui/buttonrollover.wav");
	m_pImgAchievements->SetImage("mainmenu/achievements");
	m_pImgAchievements->SetSize(256, 64);
	m_pButtonAchievements->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonAchievements->SetZPos(30);
	m_pButtonAchievements->SetCommand("Achievements");
	m_pButtonAchievements->AddActionSignalTarget(this);
	m_pImgAchievements->SetZPos(20);

	// Back
	m_pButtonBack = vgui::SETUP_PANEL(new vgui::Button(this, "btnBack", ""));
	m_pButtonBack->SetSize(256, 28);
	m_pButtonBack->SetPaintBorderEnabled(false);
	m_pButtonBack->SetPaintEnabled(false);
	m_pButtonBack->SetArmedSound("ui/buttonrollover.wav");
	m_pImgBack->SetImage("mainmenu/back");
	m_pImgBack->SetSize(256, 64);
	m_pButtonBack->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonBack->SetZPos(30);
	m_pButtonBack->AddActionSignalTarget(this);
	m_pButtonBack->SetCommand("Back");
	m_pImgBack->SetZPos(20);

	// Dialogue
	m_pButtonDialogue = vgui::SETUP_PANEL(new vgui::Button(this, "btnDialogue", ""));
	m_pButtonDialogue->SetSize(256, 28);
	m_pButtonDialogue->SetPaintBorderEnabled(false);
	m_pButtonDialogue->SetPaintEnabled(false);
	m_pButtonDialogue->AddActionSignalTarget(this);
	m_pButtonDialogue->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonDialogue->SetZPos(30);
	m_pButtonDialogue->SetCommand("Dialogue");
	m_pImgDialogue->SetZPos(20);
	m_pImgDialogue->SetImage("mainmenu/dialogue");
	m_pImgDialogue->SetSize(256, 64);

	// Viewbob
	m_pButtonViewbob = vgui::SETUP_PANEL(new vgui::Button(this, "btnViewbob", ""));
	m_pButtonViewbob->SetSize(256, 28);
	m_pButtonViewbob->SetPaintBorderEnabled(false);
	m_pButtonViewbob->SetPaintEnabled(false);
	m_pButtonViewbob->AddActionSignalTarget(this);
	m_pButtonViewbob->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonViewbob->SetZPos(30);
	m_pButtonViewbob->SetCommand("Viewbob");
	m_pImgViewbob->SetZPos(20);
	m_pImgViewbob->SetImage("mainmenu/viewbob");
	m_pImgViewbob->SetSize(256, 64);

	// Mirror
	m_pButtonMirror = vgui::SETUP_PANEL(new vgui::Button(this, "btnMirror", ""));
	m_pButtonMirror->SetSize(256, 28);
	m_pButtonMirror->SetPaintBorderEnabled(false);
	m_pButtonMirror->SetPaintEnabled(false);
	m_pButtonMirror->AddActionSignalTarget(this);
	m_pButtonMirror->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonMirror->SetZPos(30);
	m_pButtonMirror->SetCommand("Mirror");
	m_pImgMirror->SetZPos(20);
	m_pImgMirror->SetImage("mainmenu/mirror");
	m_pImgMirror->SetSize(256, 64);

	// Subtitles
	m_pButtonSubtitles = vgui::SETUP_PANEL(new vgui::Button(this, "btnSubtitles", ""));
	m_pButtonSubtitles->SetSize(256, 28);
	m_pButtonSubtitles->SetPaintBorderEnabled(false);
	m_pButtonSubtitles->SetPaintEnabled(false);
	m_pButtonSubtitles->AddActionSignalTarget(this);
	m_pButtonSubtitles->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonSubtitles->SetZPos(30);
	m_pButtonSubtitles->SetCommand("Subtitles");
	m_pImgSubtitles->SetZPos(20);
	m_pImgSubtitles->SetImage("mainmenu/subtitles");
	m_pImgSubtitles->SetSize(256, 64);

	// Film Grain FX
	m_pButtonFilmGrain = vgui::SETUP_PANEL(new vgui::Button(this, "btnFilmGrain", ""));
	m_pButtonFilmGrain->SetSize(256, 28);
	m_pButtonFilmGrain->SetPaintBorderEnabled(false);
	m_pButtonFilmGrain->SetPaintEnabled(false);
	m_pButtonFilmGrain->AddActionSignalTarget(this);
	m_pButtonFilmGrain->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonFilmGrain->SetZPos(30);
	m_pButtonFilmGrain->SetCommand("FilmGrain");
	m_pImgFilmGrain->SetZPos(20);
	m_pImgFilmGrain->SetImage("options/filmgrain_off");
	m_pImgFilmGrain->SetSize(256, 64);

	// Film Grain Str Slider
	m_pFilmGrainStrengthSlider = vgui::SETUP_PANEL(new vgui::GraphicalSlider(this, "FilmGrainStrengthSlider"));
	m_pFilmGrainSliderGUI = vgui::SETUP_PANEL(new vgui::GraphicalOverlay(this, "FilmGrainGUISlider"));
	m_pFilmGrainSliderInfo = vgui::SETUP_PANEL(new vgui::Label(this, "InfoFilmGrainSlider", ""));

	m_pFilmGrainStrengthSlider->AddActionSignalTarget(this);
	m_pFilmGrainStrengthSlider->SetRange(0, 100);
	m_pFilmGrainStrengthSlider->SetSize(256, 40);
	m_pFilmGrainStrengthSlider->SetTickCaptions("Weak", "Strong");
	m_pFilmGrainStrengthSlider->SetZPos(45);

	m_pFilmGrainSliderGUI->SetSize(300, 40);
	m_pFilmGrainSliderGUI->SetZPos(35);

	m_pFilmGrainSliderInfo->SetZPos(50);
	m_pFilmGrainSliderInfo->SetText("");
	m_pFilmGrainSliderInfo->SetSize(170, 24);
	m_pFilmGrainSliderInfo->SetContentAlignment(Label::a_center);

	// Set slider to default value!
	ConVar *str_var = cvar->FindVar("tfo_fx_filmgrain_strength");
	float flMax, flMin, flScale;
	if (str_var)
	{
		str_var->GetMax(flMax);
		str_var->GetMin(flMin);
		flScale = ((flMax - str_var->GetFloat()) * (100 / (flMax - flMin)));
		m_pFilmGrainStrengthSlider->SetValue((int)flScale);
	}

	// Redirect
	m_pButtonRedirect = vgui::SETUP_PANEL(new vgui::Button(this, "btnRedirect", ""));
	m_pButtonRedirect->SetSize(142, 28);
	m_pButtonRedirect->SetPaintBorderEnabled(false);
	m_pButtonRedirect->SetPaintEnabled(false);
	m_pButtonRedirect->AddActionSignalTarget(this);
	m_pButtonRedirect->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonRedirect->SetZPos(100);
	m_pButtonRedirect->SetCommand("Redirect");

	// Achievementzs
	for (int i = 0; i <= 11; i++)
	{
		m_pButtonAchievementItems[i] = vgui::SETUP_PANEL(new vgui::Button(this, VarArgs("btnAchievement%i", (i + 1)), ""));
		m_pImgAchievementItems[i] = vgui::SETUP_PANEL(new vgui::ImagePanel(this, VarArgs("Achievement%i", (i + 1))));
		m_pButtonAchievementItems[i]->AddActionSignalTarget(this);
		m_pButtonAchievementItems[i]->SetSize(50, 50);
		m_pButtonAchievementItems[i]->SetPaintBorderEnabled(false);
		m_pButtonAchievementItems[i]->SetPaintEnabled(false);
		m_pButtonAchievementItems[i]->SetZPos(30);
		m_pImgAchievementItems[i]->SetZPos(20);
		m_pImgAchievementItems[i]->SetImage(VarArgs("achievements/achievement_%i", (i + 1)));
		m_pImgAchievementItems[i]->SetSize(512, 512);

		m_pButtonAchievementItems[i]->SetArmedSound("dialogue/buttonclick.wav");
	}

	m_pImgAchievementText->SetSize(512, 512);
	m_pImgAchievementText->SetImage("achievements/none");
	m_pImgAchievementText->SetZPos(20);

	m_pImgAchievementBackground->SetSize(512, 512);
	m_pImgAchievementBackground->SetImage("achievements/achievement_bg");
	m_pImgAchievementBackground->SetZPos(18);

	// Quit
	m_pImgSure->SetSize(512, 512);
	m_pImgSure->SetImage("mainmenu/quit_ask");
	m_pImgSure->SetZPos(20);

	m_pButtonYes = vgui::SETUP_PANEL(new vgui::Button(this, "btnYes", ""));
	m_pButtonNo = vgui::SETUP_PANEL(new vgui::Button(this, "btnNo", ""));

	m_pButtonYes->SetSize(50, 50);
	m_pButtonYes->SetPaintBorderEnabled(false);
	m_pButtonYes->SetPaintEnabled(false);
	m_pButtonYes->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonYes->SetArmedSound("ui/buttonrollover.wav");
	m_pButtonYes->SetZPos(30);
	m_pButtonYes->SetCommand("Yes");
	m_pButtonYes->AddActionSignalTarget(this);
	m_pImgYes->SetZPos(20);
	m_pImgYes->SetImage("mainmenu/yes");
	m_pImgYes->SetSize(256, 64);

	m_pButtonNo->SetSize(50, 50);
	m_pButtonNo->SetPaintBorderEnabled(false);
	m_pButtonNo->SetPaintEnabled(false);
	m_pButtonNo->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonNo->SetArmedSound("ui/buttonrollover.wav");
	m_pButtonNo->SetZPos(30);
	m_pButtonNo->SetCommand("No");
	m_pButtonNo->AddActionSignalTarget(this);
	m_pImgNo->SetZPos(20);
	m_pImgNo->SetImage("mainmenu/no");
	m_pImgNo->SetSize(256, 64);

	// Credits
	m_pCreditsPanelList->SetSize(285, 315);
	m_pCreditsPanelList->SetPos(114, 80);
	m_pCreditsPanelList->SetZPos(20);

	// Got all achievements
	m_pImgGotAll->SetSize(512, 512);
	m_pImgGotAll->SetImage("achievements/congrats");
	m_pImgGotAll->SetZPos(20);

	m_pOptionsKeyboardList = vgui::SETUP_PANEL(new vgui::OptionsKeyboard(this, "KeyboardList"));
	m_pOptionsKeyboardList->SetSize(285, 315);
	m_pOptionsKeyboardList->SetZPos(20);
	m_pOptionsKeyboardList->SetPos(114, 80);
	m_pOptionsKeyboardList->AddActionSignalTarget(this);

	m_pOptionsMousePanel = vgui::SETUP_PANEL(new vgui::OptionsMenuMouse(this, "MouseList"));
	m_pOptionsMousePanel->SetSize(285, 315);
	m_pOptionsMousePanel->SetZPos(20);
	m_pOptionsMousePanel->SetPos(114, 80);
	m_pOptionsMousePanel->AddActionSignalTarget(this);

	m_pOptionsAudioPanel = vgui::SETUP_PANEL(new vgui::OptionsMenuAudio(this, "AudioList"));
	m_pOptionsAudioPanel->SetSize(285, 315);
	m_pOptionsAudioPanel->SetZPos(20);
	m_pOptionsAudioPanel->SetPos(114, 80);
	m_pOptionsAudioPanel->AddActionSignalTarget(this);

	m_pOptionsVideoPanel = vgui::SETUP_PANEL(new vgui::OptionsMenuVideo(this, "VideoList"));
	m_pOptionsVideoPanel->SetSize(285, 315);
	m_pOptionsVideoPanel->SetZPos(20);
	m_pOptionsVideoPanel->SetPos(114, 80);
	m_pOptionsVideoPanel->AddActionSignalTarget(this);

	m_pOptionsGraphicsPanel = vgui::SETUP_PANEL(new vgui::OptionsMenuGraphics(this, "GraphicsList"));
	m_pOptionsGraphicsPanel->SetSize(285, 315);
	m_pOptionsGraphicsPanel->SetZPos(20);
	m_pOptionsGraphicsPanel->SetPos(114, 80);
	m_pOptionsGraphicsPanel->AddActionSignalTarget(this);

	// Save, load, continue. 
	for (int i = 0; i <= 3; i++)
	{
		m_pImgSlot[i] = vgui::SETUP_PANEL(new vgui::ImagePanel(this, VarArgs("Slot%i", (i + 1))));
		m_pButtonSlot[i] = vgui::SETUP_PANEL(new vgui::Button(this, VarArgs("btnSlot%i", (i + 1)), ""));
		m_pButtonSlot[i]->AddActionSignalTarget(this);
		m_pButtonSlot[i]->SetSize(80, 140);
		m_pButtonSlot[i]->SetPaintBorderEnabled(false);
		m_pButtonSlot[i]->SetPaintEnabled(false);
		m_pButtonSlot[i]->SetZPos(30);
		m_pImgSlot[i]->SetZPos(20);

		m_pButtonSlot[i]->SetCommand(VarArgs("Slot%i", (i + 1)));

		m_pButtonSlot[i]->SetArmedSound("ui/buttonrollover.wav");
	}

	m_pImgSlots->SetSize(512, 512);
	m_pImgSlots->SetImage("savepanel/slots");
	m_pImgSlots->SetZPos(18);

	// BG All main menu
	m_pImgBackground->SetSize(512, 512);
	m_pImgBackground->SetImage("mainmenu/bg");
	m_pImgBackground->SetZPos(15);

	PerformDefaultLayout();

	InvalidateLayout();

	vgui::ivgui()->AddTickSignal(GetVPanel(), 18);

	Activate();
	RequestFocus();
}

// OnTick calls stuff every defined interval instead of constantly all the time.
void CMainMenu::OnTick()
{
	BaseClass::OnTick();

	if (InCredits)
	{
		m_pCreditsPanelList->DoAnimate();
	}
}

// Figure out which sound should be played!
void CMainMenu::PlayMenuSound()
{
	if (InCredits)
	{
		if (engine->IsLevelMainMenuBackground() || !InGame())
			FMODManager()->TransitionAmbientSound("musics/yabloch_theme.wav");
	}
	else
	{
		if (engine->IsLevelMainMenuBackground() || !InGame())
			FMODManager()->TransitionAmbientSound("horror/ambient/menu_loop.wav");
	}
}

void CMainMenu::ReturnToMainMenu()
{
	if (InOptions)
	{
		int iInCustomOpt = -1;
		for (int i = 0; i <= 5; i++)
		{
			if (InCustomOptionMenu[i])
				iInCustomOpt = i;
		}

		if (iInCustomOpt < 0)
		{
			InOptions = false;
			m_pButtonBack->SetPos(120, 372);
			m_pImgBack->SetPos(120, 360);
		}
		else
		{
			if (iInCustomOpt == 1)
				m_pOptionsMousePanel->ApplyAllChanges();
			else if (iInCustomOpt == 2)
				m_pOptionsAudioPanel->ApplyAllChanges();
			else if (iInCustomOpt == 3)
				m_pOptionsVideoPanel->ApplyAllChanges();
			else if (iInCustomOpt == 4)
				m_pOptionsGraphicsPanel->ApplyAllChanges();

			InCustomOptionMenu[iInCustomOpt] = false;
		}
	}
	else if (InAchievements)
		InAchievements = false;
	else if (InCredits)
	{
		InCredits = false;
		m_pButtonBack->SetPos(120, 372);
		m_pImgBack->SetPos(120, 360);

		PlayMenuSound();
	}
	else if (InStart)
		InStart = false;
	else if (InLoad)
		InLoad = false;
	else if (InQuit)
		InQuit = false;
}

void CMainMenu::OnKeyCodeTyped(vgui::KeyCode code)
{
	if ((gameuifuncs->GetButtonCodeForBind("OpenGameConsole") == code) && (!InGame() || engine->IsLevelMainMenuBackground()))
	{
		GameBaseClient->ShowConsole(true, false, false);
	}
	else if (code == KEY_ESCAPE || code == KEY_BACKSPACE)
	{
		if (InAnyMenu)
		{
			ReturnToMainMenu(); // Go to prev page
			vgui::surface()->PlaySound("dialogue/button_over.wav");
		}
	}
	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}

void CMainMenu::OnCommand(const char *command)
{
	if (!Q_stricmp(command, "Redirect"))
	{
		steamapicontext->SteamFriends()->ActivateGameOverlayToWebPage("http://www.tfo-mod.com");
	}

	if (!Q_stricmp(command, "Begin"))
	{
		InStart = true;

		for (int i = 0; i <= 3; i++)
		{
			m_pImgSlot[i]->SetImage(VarArgs("mainmenu/chapter%i", (i + 1)));
			m_pImgSlot[i]->SetSize(512, 512);
		}
	}

	if (!Q_stricmp(command, "Continue"))
	{
		InLoad = true;

		for (int i = 0; i <= 3; i++)
		{
			m_pImgSlot[i]->SetSize(256, 256);

			KeyValues *kvSvData = GetSaveData(szSlotSaves[i]);
			if (kvSvData == NULL)
			{
				m_pImgSlot[i]->SetImage("savepanel/empty");
			}
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

	if (!Q_stricmp(command, "Credits"))
	{
		InCredits = true;
		m_pButtonBack->SetPos(120, 402);
		m_pImgBack->SetPos(120, 390);
		m_pCreditsPanelList->DoReset();

		PlayMenuSound();
	}

	if (!Q_stricmp(command, "Options"))
	{
		InOptions = true;
		m_pButtonBack->SetPos(120, 402);
		m_pImgBack->SetPos(120, 390);
	}

	for (int i = 0; i <= 3; i++)
	{
		if (!Q_stricmp(command, VarArgs("Slot%i", (i + 1))))
		{
			if (!m_pButtonSlot[i]->IsEnabled())
				return;

			if (InStart)
			{
				ConVar *chapter_var = cvar->FindVar(VarArgs("cl_chapter_%i_unlock", (i + 1)));
				if (chapter_var)
				{
					if (chapter_var->GetBool())
					{
						InStart = false;
						engine->ClientCmd_Unrestricted("tfo_selected_save \"\"\n");
						GameBaseClient->MapLoad(szChapterMapsTFO[i]);
					}
				}
				else // Chapter 1 has no unlock convar...
				{
					InStart = false;
					engine->ClientCmd_Unrestricted("tfo_selected_save \"\"\n");
					GameBaseClient->MapLoad(szChapterMapsTFO[i]);
				}
			}
			else if (InLoad)
			{
				// Only allow loading from this slot if the slot is not empty!
				if (strcmp(m_pImgSlot[i]->GetImageName(), "savepanel/empty") && strcmp(m_pImgSlot[i]->GetImageName(), "savepanel/empty_over"))
				{
					InLoad = false;
					GameBaseClient->MapLoad(szSlotSaves[i], true);
				}
			}
		}
	}

	if (!Q_stricmp(command, "Yes"))
	{
		if (InQuit)
		{
			InQuit = false;
			engine->ClientCmd("gamemenucommand quitnoconfirm\n");
		}
	}

	if (!Q_stricmp(command, "No"))
	{
		if (InQuit)
			InQuit = false;
	}

	if (!Q_stricmp(command, "Back"))
	{
		ReturnToMainMenu();
	}

	if (!Q_stricmp(command, "Dialogue"))
	{
		if (InOptions)
		{
			if (InCustomOptionMenu[5])
			{
				ConVar* tfo_dialogue = cvar->FindVar("cl_dialoguemode");

				if (tfo_dialogue->GetBool())
					tfo_dialogue->SetValue(0);
				else
					tfo_dialogue->SetValue(1);

				engine->ClientCmd_Unrestricted("host_writeconfig\n"); // update config file...
			}
		}
	}

	if (!Q_stricmp(command, "Subtitles"))
	{
		if (InOptions)
		{
			if (InCustomOptionMenu[5])
			{
				ConVar* tfo_subtitles = cvar->FindVar("cc_subtitles");
				ConVar* tfo_caption = cvar->FindVar("closecaption");
				ConVar* tfo_language = cvar->FindVar("cc_lang");

				if (tfo_subtitles->GetBool())
				{
					tfo_subtitles->SetValue(0);
					tfo_caption->SetValue(0);
				}
				else
				{
					tfo_subtitles->SetValue(1);
					tfo_caption->SetValue(1);
					tfo_language->SetValue("english");
				}

				engine->ClientCmd_Unrestricted("host_writeconfig\n"); // update config file...
			}
		}
	}

	if (!Q_stricmp(command, "FilmGrain"))
	{
		if (InOptions)
		{
			if (InCustomOptionMenu[5])
			{
				ConVar* tfo_filmgrain = cvar->FindVar("tfo_fx_filmgrain");

				tfo_filmgrain->SetValue(!tfo_filmgrain->GetBool());

				engine->ClientCmd_Unrestricted("host_writeconfig\n"); // update config file...
			}
		}
	}

	if (!Q_stricmp(command, "Mirror"))
	{
		if (InOptions)
		{
			if (InCustomOptionMenu[5])
			{
				ConVar* tfo_mirror = cvar->FindVar("cl_player_render_mirror");

				if (tfo_mirror->GetBool())
					tfo_mirror->SetValue(0);
				else
					tfo_mirror->SetValue(1);

				engine->ClientCmd_Unrestricted("host_writeconfig\n"); // update config file...
			}
		}
	}

	if (!Q_stricmp(command, "Viewbob"))
	{
		if (InOptions)
		{
			if (InCustomOptionMenu[5])
			{
				ConVar* tfo_viewbob = cvar->FindVar("cl_viewbob_enabled");

				if (tfo_viewbob->GetBool())
					tfo_viewbob->SetValue(0);
				else
					tfo_viewbob->SetValue(1);

				engine->ClientCmd_Unrestricted("host_writeconfig\n"); // update config file...
			}
		}
	}

	if (!Q_stricmp(command, "Leave"))
	{
		InQuit = true;
	}

	for (int i = 0; i <= 5; i++)
	{
		if (!Q_stricmp(command, VarArgs("CustomOption%i", (i + 1))))
		{
			InCustomOptionMenu[i] = true;

			// Is this menu active? 
			if (InCustomOptionMenu[0])
			{
				m_pOptionsKeyboardList->SetVisible(true);
				m_pOptionsKeyboardList->FillKeyboardList();
			}

			if (InCustomOptionMenu[1])
			{
				m_pOptionsMousePanel->SetVisible(true);
				m_pOptionsMousePanel->UpdateLayout();
			}

			if (InCustomOptionMenu[2])
			{
				m_pOptionsAudioPanel->SetVisible(true);
				m_pOptionsAudioPanel->UpdateLayout();
			}

			if (InCustomOptionMenu[3])
			{
				m_pOptionsVideoPanel->SetVisible(true);
				m_pOptionsVideoPanel->UpdateLayout();
			}

			if (InCustomOptionMenu[4])
			{
				m_pOptionsGraphicsPanel->SetVisible(true);
				m_pOptionsGraphicsPanel->UpdateLayout();
			}
		}
	}

	if (!Q_stricmp(command, "Achievements"))
	{
		InAchievements = true;
	}

	// If a command was issued:
	// You now have to return to the very main main menu before you can escape to go back in-game.
	if (Q_stricmp(command, ""))
	{
		GameBaseClient->SetLoadingScreen(true);
		if (!InAnyMenu)
		{
			InAnyMenu = true;
			engine->ClientCmd_Unrestricted("gameui_preventescape\n");
		}
	}

	BaseClass::OnCommand(command);
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMainMenu::~CMainMenu()
{
}

void CMainMenu::OnSliderMove(KeyValues *data)
{
	int min, max;
	m_pFilmGrainStrengthSlider->GetRange(min, max);
	ConVar *str_var = cvar->FindVar("tfo_fx_filmgrain_strength");
	float flMax, flMin, flScale;
	if (str_var)
	{
		str_var->GetMax(flMax);
		str_var->GetMin(flMin);
		if (m_pFilmGrainStrengthSlider->GetValue() > 0)
			flScale = flMax - (((float)m_pFilmGrainStrengthSlider->GetValue() / (float)max) * (flMax - flMin));
		else
			flScale = 2;
		str_var->SetValue(flScale);
	}

	engine->ClientCmd_Unrestricted("host_writeconfig\n"); // update config file...
}