//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Loading Panel - Overrides the default loading panel and also hides it + reads its progress value.
//
//=============================================================================//
#include "cbase.h"
#include "LoadingPanel.h"
#include "vgui_controls/Frame.h"
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/IInput.h>
#include "hl2_gamerules.h"
#include "filesystem.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
#include "fmod_manager.h"
#include "GameBase_Client.h"
#include "ienginevgui.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define TFO_COLOR		    Color( 255, 255, 255, 255 )

//-----------------------------------------------------------------------------
// Purpose: Displays the logo panel
//-----------------------------------------------------------------------------
void CLoadingPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pTextLoadingTip->SetFont(pScheme->GetFont("TFOInventory"));
	m_pTextLoadingTip->SetFgColor(TFO_COLOR);
}

// The panel background image should be square, not rounded.
void CLoadingPanel::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBackgroundType(0);
	BaseClass::PaintBackground();
}

void CLoadingPanel::PerformLayout()
{
	SetupLayout();
	m_pTextLoadingTip->SetFgColor(TFO_COLOR);

	BaseClass::PerformLayout();
}

void CLoadingPanel::OnScreenSizeChanged(int iOldWide, int iOldTall)
{
	BaseClass::OnScreenSizeChanged(iOldWide, iOldTall);
	LoadControlSettings("resource/ui/loadingpaneltfo.res");
	PerformLayout();
}

void CLoadingPanel::SetupLayout(void)
{
	// Set Size
	SetSize(ScreenWidth(), ScreenHeight());
	m_pImgLoadingBackground->SetSize(ScreenWidth(), ScreenHeight());
	m_pImgLoadingForeground->SetSize(ScreenWidth(), ScreenHeight());

	m_pImgLoadingBar->SetSize(scheme()->GetProportionalScaledValue(400), scheme()->GetProportionalScaledValue(26));
	m_pTextLoadingTip->SetSize(ScreenWidth(), scheme()->GetProportionalScaledValueEx(GetScheme(), 30));

	// Set Pos
	SetPos(0, 0);
	m_pImgLoadingBackground->SetPos(0, 0);
	m_pImgLoadingForeground->SetPos(0, 0);

	int barW, barH;
	m_pImgLoadingBar->GetSize(barW, barH);

	int tipW, tipH;
	m_pTextLoadingTip->GetSize(tipW, tipH);

	m_pImgLoadingBar->SetPos(((ScreenWidth() / 2) - (barW / 2)), scheme()->GetProportionalScaledValue(430));
	m_pTextLoadingTip->SetPos(((ScreenWidth() / 2) - (tipW / 2)), scheme()->GetProportionalScaledValueEx(GetScheme(), 395));
}

void CLoadingPanel::OnThink()
{
	SetupLayout();
	BaseClass::OnThink();
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CLoadingPanel::CLoadingPanel(vgui::VPANEL parent) : BaseClass(NULL, "CLoadingPanel")
{
	SetParent(parent);
	SetTitleBarVisible(false);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);
	SetSizeable(false);
	SetMoveable(false);
	SetProportional(true);
	SetVisible(true);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/TFOLoadScheme.res", "TFOLoadScheme"));

	SetZPos(100);

	vgui::ivgui()->AddTickSignal(GetVPanel(), 1);

	InvalidateLayout();

	m_bIsLoadingMainMenu = true;
	m_flTipDisplayTime = 0.0f;
	m_bIsLoading = false;
	m_bIsMenuVisibleAndInGame = false;

	// Initialize images
	m_pImgLoadingBackground = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "LoadingImage"));
	m_pImgLoadingForeground = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "LoadingFG"));
	m_pImgLoadingBar = vgui::SETUP_PANEL(new ImageProgressBar(this, "LoadingImageProgress", "vgui/loading/loading_bar_fg", "vgui/loading/loading_bar_bg"));

	m_pImgLoadingBackground->SetImage("loading/default");
	m_pImgLoadingBackground->SetShouldScaleImage(true);
	m_pImgLoadingBackground->SetZPos(110);

	m_pImgLoadingForeground->SetImage("loading/loading_bg");
	m_pImgLoadingForeground->SetShouldScaleImage(true);
	m_pImgLoadingForeground->SetZPos(120);

	m_pImgLoadingBar->SetZPos(170);

	// Loading Tips
	m_pTextLoadingTip = vgui::SETUP_PANEL(new vgui::Label(this, "TextTip", ""));
	m_pTextLoadingTip->SetZPos(180);
	m_pTextLoadingTip->SetFgColor(TFO_COLOR);
	m_pTextLoadingTip->SetText("");

	LoadControlSettings("resource/ui/loadingpaneltfo.res");

	PerformLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CLoadingPanel::~CLoadingPanel()
{
}

void CLoadingPanel::SetRandomLoadingTip()
{
	KeyValues *kvLoadingTips = new KeyValues("LoadingTipData");
	if (kvLoadingTips->LoadFromFile(filesystem, "resource/data/settings/Tips.txt", "MOD"))
	{
		int iAmountTips = 0;
		for (KeyValues *sub = kvLoadingTips->GetFirstSubKey(); sub; sub = sub->GetNextKey())
			iAmountTips++;

		KeyValues *kvSelectedTip = kvLoadingTips->FindKey(VarArgs("%i", random->RandomInt(1, iAmountTips)));
		if (kvSelectedTip)
			m_pTextLoadingTip->SetText(kvSelectedTip->GetString());
	}
	else
	{
		m_pTextLoadingTip->SetText("");
	}

	kvLoadingTips->deleteThis();
}

void CLoadingPanel::SetCustomLoadingImage(const char *szImage, bool bVisible)
{
	m_pImgLoadingForeground->SetVisible(bVisible);
	m_pImgLoadingBackground->SetVisible(bVisible);
	m_pImgLoadingBackground->SetImage(szImage);
}

void CLoadingPanel::SetIsLoadingMainMenu(bool bValue)
{
	m_bIsLoadingMainMenu = bValue;
	m_pImgLoadingForeground->SetVisible(bValue);
	m_pImgLoadingBackground->SetVisible(bValue);
	m_pImgLoadingBar->SetVisible(bValue);
	m_pTextLoadingTip->SetVisible(bValue);
	m_pImgLoadingBackground->SetImage("transparency");
}

void CLoadingPanel::OnTick()
{
	BaseClass::OnTick();

	// Update FMOD Fading..
	FMODManager()->FadeThink();

	GameBaseClient->MoveConsoleToFront();

	// Are we loading main menu stuff? 
	if (m_bIsLoadingMainMenu)
	{
		m_pImgLoadingForeground->SetVisible(false);
		m_pImgLoadingBackground->SetVisible(false);
		m_pImgLoadingBar->SetVisible(false);
		m_pTextLoadingTip->SetVisible(false);
		return;
	}
	else
	{
		m_pImgLoadingForeground->SetVisible(true);
		m_pImgLoadingBackground->SetVisible(true);
		m_pImgLoadingBar->SetVisible(true);
		m_pTextLoadingTip->SetVisible(true);
	}

	if (!IsVisible())
	{
		if (m_bIsLoading)
		{
			m_flTipDisplayTime = 0.0f;
			SetCustomLoadingImage("loading/default", false);
			m_pImgLoadingBar->SetProgress(0.0f);
			SetScreenBlurState(false);

			C_BasePlayer *pClient = C_BasePlayer::GetLocalPlayer();
			if (pClient && !engine->IsLevelMainMenuBackground())
			{
				FMODManager()->StopAmbientSound(true);
				GameBaseClient->Initialize(true);
			}
			else
			{
				GameBaseClient->Initialize();
			}

			m_bIsLoading = false;
		}

		if (!m_bIsMenuVisibleAndInGame)
		{
			if (enginevgui->IsGameUIVisible() && !engine->IsLevelMainMenuBackground() && engine->IsInGame())
			{
				SetScreenBlurState(true);
				FMODManager()->PlayLoadingSound("musics/tfo_monster.wav");
				m_bIsMenuVisibleAndInGame = true;
			}
		}
		else if (m_bIsMenuVisibleAndInGame)
		{
			if (!enginevgui->IsGameUIVisible() && !engine->IsLevelMainMenuBackground() && engine->IsInGame())
			{
				SetScreenBlurState(false);
				FMODManager()->StopAmbientSound(true);
				m_bIsMenuVisibleAndInGame = false;
			}
		}
	}
	else
	{
		SetLoadingAttributes();
		Activate();
		RequestFocus();
		MoveToFront();

		if (!m_bIsLoading)
		{
			m_bIsLoading = true;
			m_bIsMenuVisibleAndInGame = false;

			FileFindHandle_t findHandle;
			const char *pFilename = filesystem->FindFirstEx("materials/vgui/loading/*.vmt", "MOD", &findHandle);
			while (pFilename)
			{
				char textureFile[256];
				byte fileLenght = strlen(pFilename) - 3;
				Q_strncpy(textureFile, pFilename, fileLenght);
				Q_strlower(textureFile);

				if (!strcmp(textureFile, HL2GameRules()->GetCurrentLoadingImage()))
				{
					SetCustomLoadingImage(VarArgs("loading/%s", HL2GameRules()->GetCurrentLoadingImage()), true);
					break;
				}
				else
					SetCustomLoadingImage("loading/default", true);

				pFilename = filesystem->FindNext(findHandle);
			}
			filesystem->FindClose(findHandle);
		}

		// Tips / Poems / Conclusions:
		if (m_flTipDisplayTime <= engine->Time())
		{
			m_flTipDisplayTime = engine->Time() + 1.5f;
			SetRandomLoadingTip();
		}
	}
}

void CLoadingPanel::SetLoadingAttributes(void)
{
	vgui::VPANEL panel = GetVParent();
	if (panel)
	{
		int NbChilds = vgui::ipanel()->GetChildCount(panel);

		for (int i = 0; i < NbChilds; ++i)
		{
			VPANEL tmppanel = vgui::ipanel()->GetChild(panel, i);
			int newChilds = vgui::ipanel()->GetChildCount(tmppanel);
			for (int z = 0; z < newChilds; ++z)
			{
				VPANEL prPan = vgui::ipanel()->GetChild(tmppanel, z);
				Panel *myPanel = vgui::ipanel()->GetPanel(prPan, "GameUI");
				if (myPanel)
				{
					// Bernt - Get Progress Value:
					if (!strcmp(myPanel->GetName(), "Progress"))
					{
						ProgressBar *pBar = dynamic_cast<ProgressBar*> (myPanel);
						if (pBar)
							m_pImgLoadingBar->SetProgress(pBar->GetProgress());
					}
				}
			}
		}
	}
}