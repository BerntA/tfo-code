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
#include "filesystem.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
#include "fmod_manager.h"
#include "GameBase_Client.h"
#include "ienginevgui.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar tfo_loading_image;

#define TFO_COLOR Color( 255, 255, 255, 255 )
#define TFO_BOTTOM_DIVIDER Color(25, 25, 25, 150)

void CLoadingPanel::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pTextLoadingTip->SetFont(pScheme->GetFont("LoadingText"));
	m_pTextLoadingTip->SetFgColor(TFO_COLOR);
}

void CLoadingPanel::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBackgroundType(0);
	BaseClass::PaintBackground();
}

void CLoadingPanel::PerformLayout()
{
	SetupLayout();
	BaseClass::PerformLayout();
}

void CLoadingPanel::OnScreenSizeChanged(int iOldWide, int iOldTall)
{
	BaseClass::OnScreenSizeChanged(iOldWide, iOldTall);
	PerformLayout();
}

void CLoadingPanel::SetupLayout(void)
{
	int w = ScreenWidth(), h = ScreenHeight();

	SetPos(0, 0);
	SetSize(w, h);

	m_pImgLoadingBackground->SetSize(w, h);
	m_pImgLoadingForeground->SetSize(w, h);

	m_pImgLoadingBackground->SetPos(0, 0);
	m_pImgLoadingForeground->SetPos(0, 0);

	int size = scheme()->GetProportionalScaledValue(40);
	m_pBottom->SetSize(w, size);
	m_pBottom->SetPos(0, h - size);

	m_pImgLoadingBar->SetSize(w, scheme()->GetProportionalScaledValue(12));
	m_pImgLoadingBar->SetPos(0, h - size - scheme()->GetProportionalScaledValue(10));

	m_pTextLoadingTip->SetPos(scheme()->GetProportionalScaledValue(76), h - size);
	m_pTextLoadingTip->SetSize(w, size);

	m_pImgEagle->SetSize(scheme()->GetProportionalScaledValue(60), scheme()->GetProportionalScaledValue(30));
	m_pImgEagle->SetPos(scheme()->GetProportionalScaledValue(6), h - size + scheme()->GetProportionalScaledValue(5));
}

void CLoadingPanel::OnThink()
{
	SetupLayout();
	BaseClass::OnThink();
}

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
	SetScheme("TFOLoadScheme");

	SetZPos(100);

	vgui::ivgui()->AddTickSignal(GetVPanel(), 1);

	InvalidateLayout();

	m_bIsLoadingMainMenu = true;
	m_bIsLoading = false;
	m_bIsMenuVisibleAndInGame = false;

	// Initialize images
	m_pImgLoadingBackground = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "LoadingImage"));
	m_pImgLoadingForeground = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "LoadingFG"));
	m_pImgEagle = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "EagleIcon"));

	m_pImgLoadingBar = vgui::SETUP_PANEL(new ImageProgressBar(this, "LoadingImageProgress", "vgui/loading/loading_bar_fg", "vgui/loading/loading_bar_bg"));

	m_pImgLoadingBackground->SetImage("loading/default");
	m_pImgLoadingBackground->SetShouldScaleImage(true);
	m_pImgLoadingBackground->SetZPos(110);

	m_pImgLoadingForeground->SetImage("loading/loading_bg");
	m_pImgLoadingForeground->SetShouldScaleImage(true);
	m_pImgLoadingForeground->SetZPos(120);

	m_pImgEagle->SetImage("loading/loading_eagle");
	m_pImgEagle->SetShouldScaleImage(true);
	m_pImgEagle->SetZPos(160);

	m_pImgLoadingBar->SetZPos(170);

	// Loading Tips
	m_pTextLoadingTip = vgui::SETUP_PANEL(new vgui::Label(this, "TextTip", ""));
	m_pTextLoadingTip->SetZPos(180);
	m_pTextLoadingTip->SetFgColor(TFO_COLOR);
	m_pTextLoadingTip->SetText("");
	m_pTextLoadingTip->SetContentAlignment(Label::Alignment::a_west);

	// Bottom divider part
	m_pBottom = vgui::SETUP_PANEL(new vgui::Divider(this, "BottomDivider"));
	m_pBottom->SetZPos(150);
	m_pBottom->SetFgColor(TFO_BOTTOM_DIVIDER);
	m_pBottom->SetBgColor(TFO_BOTTOM_DIVIDER);
	m_pBottom->SetBorder(NULL);
	m_pBottom->SetPaintBorderEnabled(false);

	PerformLayout();
}

CLoadingPanel::~CLoadingPanel()
{
}

void CLoadingPanel::SetRandomLoadingTip()
{
	KeyValues* kvLoadingTips = new KeyValues("LoadingTipData");

	if (kvLoadingTips->LoadFromFile(filesystem, "data/settings/Tips.txt", "MOD"))
	{
		int iAmountTips = 0;
		for (KeyValues* sub = kvLoadingTips->GetFirstSubKey(); sub; sub = sub->GetNextKey())
			iAmountTips++;

		KeyValues* kvSelectedTip = kvLoadingTips->FindKey(VarArgs("%i", random->RandomInt(1, iAmountTips)));
		if (kvSelectedTip)
			m_pTextLoadingTip->SetText(kvSelectedTip->GetString());
	}
	else
	{
		m_pTextLoadingTip->SetText("");
	}

	kvLoadingTips->deleteThis();
}

void CLoadingPanel::SetCustomLoadingImage(const char* szImage, bool bVisible)
{
	m_pImgLoadingForeground->SetVisible(bVisible);
	m_pImgLoadingBackground->SetVisible(bVisible);
	m_pImgLoadingBackground->SetImage(szImage);
}

void CLoadingPanel::SetIsLoadingMainMenu(bool bValue)
{
	m_bIsLoadingMainMenu = bValue;
	m_pImgLoadingBackground->SetImage("transparency");
}

void CLoadingPanel::OnTick()
{
	BaseClass::OnTick();

	FMODManager()->Think();
	GameBaseClient->MoveConsoleToFront();

	// Are we loading main menu stuff? 
	if (m_bIsLoadingMainMenu)
	{
		m_pImgLoadingForeground->SetVisible(false);
		m_pImgLoadingBackground->SetVisible(false);
		m_pImgEagle->SetVisible(false);
		m_pImgLoadingBar->SetVisible(false);
		m_pTextLoadingTip->SetVisible(false);
		m_pBottom->SetVisible(false);
		return;
	}
	else
	{
		m_pImgLoadingForeground->SetVisible(true);
		m_pImgLoadingBackground->SetVisible(true);
		m_pImgEagle->SetVisible(true);
		m_pImgLoadingBar->SetVisible(true);
		m_pTextLoadingTip->SetVisible(true);
		m_pBottom->SetVisible(true);
	}

	if (!IsVisible())
	{
		if (m_bIsLoading)
		{
			SetCustomLoadingImage("loading/default", false);
			m_pImgLoadingBar->SetProgress(0.0f);
			SetScreenBlurState(false);

			C_BasePlayer* pClient = C_BasePlayer::GetLocalPlayer();
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
				FMODManager()->PlayLoadingSound("music/tfo_monster.mp3");
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
			HideLegacyLoadingLayout();

			m_bIsLoading = true;
			m_bIsMenuVisibleAndInGame = false;

			char pchTargetImage[MAX_PATH];
			Q_snprintf(pchTargetImage, MAX_PATH, "materials/vgui/loading/%s.vmt", tfo_loading_image.GetString());

			if (filesystem->FileExists(pchTargetImage, "MOD"))
				SetCustomLoadingImage(VarArgs("loading/%s", tfo_loading_image.GetString()), true);
			else
				SetCustomLoadingImage("loading/default", true);

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
				Panel* myPanel = vgui::ipanel()->GetPanel(prPan, "GameUI");
				if (myPanel)
				{
					// Bernt - Get Progress Value:
					if (!strcmp(myPanel->GetName(), "Progress"))
					{
						ProgressBar* pBar = dynamic_cast<ProgressBar*> (myPanel);
						if (pBar)
							m_pImgLoadingBar->SetProgress(pBar->GetProgress());
					}
				}
			}
		}
	}
}

void CLoadingPanel::HideLegacyLoadingLayout(void)
{
	vgui::VPANEL panel = GetVParent();
	if (panel)
	{
		int NbChilds = vgui::ipanel()->GetChildCount(panel);
		for (int i = 0; i < NbChilds; ++i)
		{
			VPANEL gameUIPanel = vgui::ipanel()->GetChild(panel, i);
			Panel* basePanel = vgui::ipanel()->GetPanel(gameUIPanel, "GameUI");
			if (basePanel)
			{
				// We load a different scheme file for the actual engine loading dialog so we don't screw anything up, the scheme has all colors set to BLANK so the dialog will be invisible.
				if (!strcmp(basePanel->GetName(), "LoadingDialog"))
				{
					basePanel->SetScheme("TFOLoadScheme");
					basePanel->InvalidateLayout(false, true);
				}
			}
		}
	}
}