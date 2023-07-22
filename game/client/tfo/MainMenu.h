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

#ifndef MAIN_MENU_PANEL_H
#define MAIN_MENU_PANEL_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "c_basehlplayer.h"
#include "vgui_controls/Panel.h"
#include "vgui_controls/AnimationController.h"
#include "vgui/ISurface.h"
#include <vgui/ILocalize.h>
#include <vgui_controls/Label.h>
#include <vgui/IInput.h>
#include <vgui/KeyCode.h>
#include "CreditsListing.h"
#include "OptionsMenuKeyboard.h"
#include <vgui/MouseCode.h>
#include "OptionsMenuMouse.h"
#include "OptionsMenuAudio.h"
#include "OptionsMenuVideo.h"
#include "OptionsMenuGraphics.h"

namespace vgui
{
	class Panel;
}

class CMainMenu : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CMainMenu, vgui::Frame);

public:
	CMainMenu(vgui::VPANEL parent);
	~CMainMenu();

	void OnShowPanel(bool state);
	void PaintBackground();
	void PerformDefaultLayout();
	void PerformLayout();
	void OnScreenSizeChanged(int iOldWide, int iOldTall);
	void OnThink();

	bool InGame();

	void OnCommand(const char *command);
	void ApplySchemeSettings(vgui::IScheme *pScheme);

	void OnKeyCodeTyped(vgui::KeyCode code);

	void ReturnToMainMenu();
	void PlayMenuSound();

	void OnTick();

private:

	void CheckSaveRollovers(int x, int y);

	vgui::ImagePanel *m_pImgBegin;
	vgui::ImagePanel *m_pImgContinue;
	vgui::ImagePanel *m_pImgCredits;
	vgui::ImagePanel *m_pImgOptions;
	vgui::ImagePanel *m_pImgLeave;
	vgui::ImagePanel *m_pImgAchievements;
	vgui::ImagePanel *m_pImgBack;

	// Quit
	vgui::ImagePanel *m_pImgYes;
	vgui::ImagePanel *m_pImgNo;
	vgui::ImagePanel *m_pImgSure;

	// Info Version
	vgui::Label *m_pLabelVersion;

	// Achievements
	vgui::ImagePanel *m_pImgAchievementItems[12];
	vgui::Button *m_pButtonAchievementItems[12];
	bool InRolloverAchievementItems[12];

	vgui::ImagePanel *m_pImgAchievementText;
	vgui::ImagePanel *m_pImgGotAll;
	vgui::ImagePanel *m_pImgAchievementBackground;

	// Credits
	vgui::CreditsListing *m_pCreditsPanelList;

	vgui::ImagePanel *m_pImgBackground;

	vgui::Button *m_pButtonBegin;
	vgui::Button *m_pButtonContinue;
	vgui::Button *m_pButtonCredits;
	vgui::Button *m_pButtonOptions;
	vgui::Button *m_pButtonLeave;
	vgui::Button *m_pButtonAchievements;
	vgui::Button *m_pButtonBack;

	// Quit
	vgui::Button *m_pButtonYes;
	vgui::Button *m_pButtonNo;

	// Begin/Continue
	vgui::Button *m_pButtonSlot[4];
	vgui::ImagePanel *m_pImgSlot[4];
	vgui::ImagePanel *m_pImgSlots;

	bool InOptions;
	bool InAchievements;
	bool InQuit;
	bool InCredits;

	bool InStart;
	bool InLoad;

	bool InAnyMenu;

	KeyValues *GetSaveData(const char *szFile);
	void DeleteSaveDataFile(const char *szFile);

	// TFO Options:
	void ShowBaseOtherOptions(bool bShow);

	// Base
	bool InCustomOptionMenu[6];

	vgui::ImagePanel *m_pImgCustomOptions[6];
	vgui::Button *m_pButtonCustomOptions[6];

	// Controls - Keyboard
	vgui::OptionsKeyboard *m_pOptionsKeyboardList;

	// Controls - Mouse
	vgui::OptionsMenuMouse *m_pOptionsMousePanel;

	// Controls - Audio
	vgui::OptionsMenuAudio *m_pOptionsAudioPanel;

	// Controls - Video
	vgui::OptionsMenuVideo *m_pOptionsVideoPanel;

	// Controls - Graphics
	vgui::OptionsMenuGraphics *m_pOptionsGraphicsPanel;

	// Controls - Other
	vgui::ImagePanel *m_pImgDialogue;
	vgui::ImagePanel *m_pImgViewbob;
	vgui::ImagePanel *m_pImgMirror;
	vgui::ImagePanel *m_pImgSubtitles;
	vgui::ImagePanel *m_pImgFilmGrain;

	vgui::Button *m_pButtonDialogue;
	vgui::Button *m_pButtonViewbob;
	vgui::Button *m_pButtonMirror;
	vgui::Button *m_pButtonSubtitles;
	vgui::Button *m_pButtonFilmGrain;

	// Film Grain Strength
	vgui::GraphicalSlider *m_pFilmGrainStrengthSlider;
	vgui::GraphicalOverlay *m_pFilmGrainSliderGUI;
	vgui::Label *m_pFilmGrainSliderInfo;

	MESSAGE_FUNC_PARAMS(OnSliderMove, "SliderMoved", data);
};

#endif // MAIN_MENU_PANEL_H