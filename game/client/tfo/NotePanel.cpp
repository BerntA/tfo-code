//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Displays Notes and sometimes fades in a VO, depends what the script says.
// Notice: In TFO v2.8 and lower Notes would be stored in your Inventory so you could read them later on, this feature has been removed in TFO V2.9+
//
//=============================================================================//

#include "cbase.h"
#include "NotePanel.h"
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
#include "in_buttons.h"
#include "fmod_manager.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/RichText.h>

//-----------------------------------------------------------------------------
// Purpose: Displays the logo panel
//-----------------------------------------------------------------------------
void CNotePanel::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CNotePanel::OnScreenSizeChanged(int iOldWide, int iOldTall)
{
	BaseClass::OnScreenSizeChanged(iOldWide, iOldTall);

	LoadControlSettings("resource/ui/notepanel.res");
}

void CNotePanel::PerformDefaultLayout()
{
	// Background
	m_pBackground->SetEnabled(true);
	m_pBackground->SetVisible(true);
	// Note
	m_pNote->SetEnabled(true);
	m_pNote->SetVisible(true);
	// Button
	m_pButtonClose->SetVisible(true);
	m_pButtonClose->SetEnabled(true);

	m_pNoteText->SetVisible(true);
	m_pNoteText->SetEnabled(true);
	m_pNoteText->SetText("");

	// Reset Image
	m_pNote->SetImage("notes/none");

	PerformLayout();
}

void CNotePanel::OnThink()
{
	SetSize(ScreenWidth(), ScreenHeight());
	SetPos(0, 0);
	m_pBackground->SetSize(ScreenWidth(), ScreenHeight());
	m_pButtonClose->SetSize(ScreenWidth(), ScreenHeight());

	BaseClass::OnThink();
}

void CNotePanel::OnShowPanel(bool bShow)
{
	if (bShow)
		engine->ClientCmd_Unrestricted("gameui_preventescapetoshow\n");
	else
		engine->ClientCmd_Unrestricted("gameui_allowescapetoshow\n");
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CNotePanel::CNotePanel(vgui::VPANEL parent) : BaseClass(NULL, "NotePanel")
{
	SetParent(parent);

	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	SetProportional(false); // This might work just for note panel...
	SetTitleBarVisible(false);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);
	SetSizeable(false);
	SetMoveable(false);
	SetVisible(false);

	m_pBackground = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Background"));

	m_pBackground->SetImage("notes/background");
	m_pBackground->SetZPos(400);
	m_pBackground->SetShouldScaleImage(true);

	m_pNote = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Note"));

	m_pNote->SetImage("notes/none");
	m_pNote->SetZPos(450);
	m_pNote->SetSize(512, 512);
	m_pNote->SetShouldScaleImage(true);

	m_pButtonClose = vgui::SETUP_PANEL(new vgui::Button(this, "btnClose", ""));
	m_pButtonClose->SetZPos(500);
	m_pButtonClose->SetPaintBorderEnabled(false);
	m_pButtonClose->SetPaintEnabled(false);
	m_pButtonClose->SetCommand("Close");

	// Note Text, by default normally not used. Mostly in custom stories:
	// Allows overriding the note to use a text box + image instead of only an image.
	m_pNoteText = vgui::SETUP_PANEL(new vgui::RichText(this, "NoteText"));
	m_pNoteText->SetZPos(470);
	m_pNoteText->SetText("");
	m_pNoteText->SetSize(512, 512);
	m_pNoteText->SetUnusedScrollbarInvisible(true);
	m_pNoteText->SetVerticalScrollbar(false);

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/TFOGameScheme.res", "TFOGameScheme"));

	m_pBackground->SetSize(ScreenWidth(), ScreenHeight());
	m_pButtonClose->SetSize(ScreenWidth(), ScreenHeight());

	m_bCanFadeOutMusic = false;

	LoadControlSettings("resource/ui/notepanel.res");

	PerformDefaultLayout();

	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CNotePanel::~CNotePanel()
{
}

void CNotePanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pNoteText->SetBgColor(Color(0, 0, 0, 0));
	m_pNoteText->SetBorder(NULL);
	m_pNoteText->SetFgColor(Color(122, 73, 57, 255));
	m_pNoteText->SetFont(pScheme->GetFont("TFOMenu"));
}

void CNotePanel::OnKeyCodeTyped(vgui::KeyCode code)
{
	if (code == KEY_ESCAPE)
	{
		PerformDefaultLayout();

		if (m_bCanFadeOutMusic)
		{
			FMODManager()->StopAmbientSound(true);
			m_bCanFadeOutMusic = false;
		}

		vgui::surface()->PlaySound("hud/read_paper.wav");

		engine->ClientCmd("tfo_gameui_command OpenNotePanel\n");
	}
	else
		BaseClass::OnKeyCodeTyped(code);
}

void CNotePanel::ParseScriptFile(const char *szFile)
{
	// We set our keyvalues to read the stuff:
	KeyValues *kvItemInfo = new KeyValues("NoteData");
	if (kvItemInfo->LoadFromFile(filesystem, VarArgs("resource/data/inventory/notes/%s.txt", szFile), "MOD"))
	{
		KeyValues *pkvTexture = kvItemInfo->FindKey("Texture");
		KeyValues *pkvText = kvItemInfo->FindKey("Text");

		// Reset Text
		m_pNoteText->SetText("");
		m_pNoteText->SelectNone();
		m_bCanFadeOutMusic = false;

		const char *szVoiceOverSoundPath = ReadAndAllocStringValue(kvItemInfo, "VoiceOver");
		if (szVoiceOverSoundPath)
		{
			if (!strcmp(szVoiceOverSoundPath, "EMPTY") || (strlen(szVoiceOverSoundPath) <= 0))
				szVoiceOverSoundPath = NULL;
			else
				m_bCanFadeOutMusic = true;
		}

		if (pkvText)
		{
			m_pNote->SetImage("notes/textbg");
			m_pNoteText->SetText(ReadAndAllocStringValue(kvItemInfo, "Text"));

			if (szVoiceOverSoundPath)
				FMODManager()->TransitionAmbientSound(szVoiceOverSoundPath);

			// Scroll UP
			m_pNoteText->GotoTextStart();
		}
		else if (pkvTexture)
		{
			const char *szTexturePath = ReadAndAllocStringValue(kvItemInfo, "Texture");

			m_pNote->SetImage(szTexturePath);

			if (szVoiceOverSoundPath)
				FMODManager()->TransitionAmbientSound(szVoiceOverSoundPath);
		}
		else
		{
			Warning("No Texture or Text was found in the script!\n");
			m_pNote->SetImage("notes/none");
		}
	}
	else
	{
		Warning("Couldn't find or read the desired script file %s.txt\n", szFile);
		m_pNote->SetImage("notes/none");
	}

	kvItemInfo->deleteThis();
}

void CNotePanel::OnCommand(const char* pcCommand)
{
	if (!Q_stricmp(pcCommand, "Close"))
	{
		PerformDefaultLayout();

		if (m_bCanFadeOutMusic)
		{
			FMODManager()->StopAmbientSound(true);
			m_bCanFadeOutMusic = false;
		}

		vgui::surface()->PlaySound("hud/read_paper.wav");

		engine->ClientCmd("tfo_gameui_command OpenNotePanel\n");
	}
}