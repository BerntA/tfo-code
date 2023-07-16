//========= Copyright © 2014 Bernt A Eide, All rights reserved. ============//
//
// Purpose: Bernt - End Game Credit Panel: Same as CreditsListing.cpp but this is a standalone panel for in game purposes...
//
//=============================================================================//

#include "cbase.h"
#include "CreditsPanel.h"
#include "vgui_controls/Frame.h"
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/IInput.h>
#include "interface.h"
#include <gameui/IGameUI.h>
#include "hl2_gamerules.h"
#include "filesystem.h"
#include "c_baseplayer.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
#include "fmod_manager.h"
#include "ienginevgui.h"
#include "iclientmode.h"
#include "vgui_controls/AnimationController.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// The panel background image should be square, not rounded.
void CCreditsPanel::PaintBackground()
{
	SetBgColor(Color(0,0,0,0));
	SetPaintBackgroundType( 0 );
	BaseClass::PaintBackground();
}

void CCreditsPanel::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CCreditsPanel::PerformDefaultLayout()
{
	MoveToCenterOfScreen();
	m_pCreditsList->DoReset();
	m_pCreditsList->SetVisible( false );
	PerformLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CCreditsPanel::CCreditsPanel( vgui::VPANEL parent ) : BaseClass( NULL, "EndGameCredits" )
{
	SetParent( parent );
	SetTitleBarVisible( false );
	SetMinimizeButtonVisible( false );
	SetMaximizeButtonVisible( false );
	SetCloseButtonVisible( false );
	SetSizeable( false );
	SetMoveable( false );
	SetProportional( false );
	SetVisible( false );
	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/TFOScheme.res", "TFOScheme"));

	SetSize( 512, 512 );
	SetPos( 0, 0 );
	SetZPos(5);

	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);
	SetBgColor ( Color (0, 0, 0, 0) );
	SetFgColor ( Color( 255, 255, 255, 255 ) );
	SetAlpha( 0 );

	// Initialize images
	m_pImgBackground = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Background"));

	m_pImgBackground->SetImage( "mainmenu/bg" );
	m_pImgBackground->SetFgColor ( Color( 255, 255, 255, 255 ) );
	m_pImgBackground->SetShouldScaleImage( false );
	m_pImgBackground->SetZPos( 10 );
	m_pImgBackground->SetSize( 512, 512 );
	m_pImgBackground->SetPos( 0, 0 );

	m_pCreditsList = vgui::SETUP_PANEL(new vgui::CreditsListing(this, "CreditsListsz"));
	m_pCreditsList->SetSize( 285, 315 );
	m_pCreditsList->SetPos( 114, 80 );
	m_pCreditsList->SetZPos( 20 );

	InvalidateLayout();

	PerformDefaultLayout();

	MoveToCenterOfScreen();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CCreditsPanel::~CCreditsPanel()
{
}

void CCreditsPanel::OnShowPanel( bool bShow )
{
	if (bShow)
	{
		vgui::ivgui()->AddTickSignal(GetVPanel(), 18);
		engine->ClientCmd_Unrestricted("gameui_preventescapetoshow\n");
		FMODManager()->TransitionAmbientSound("music/credits_theme_end.mp3");
	}
	else
	{
		vgui::ivgui()->RemoveTickSignal(GetVPanel());
		engine->ClientCmd_Unrestricted("gameui_allowescapetoshow\n");
	}

	m_pCreditsList->DoReset();
	m_pCreditsList->SetVisible(false);
}

void CCreditsPanel::OnTick()
{
	BaseClass::OnTick();

	MoveToCenterOfScreen();

	int iAlpha = GetAlpha();

	if (iAlpha >= 255)
	{
		m_pCreditsList->SetVisible(true);
		m_pCreditsList->DoAnimate();
	}
}

void CCreditsPanel::OnKeyCodeTyped(vgui::KeyCode code)
{
	if (code == KEY_ESCAPE)
	{
		OnShowPanel(false);
		Close();
	}
	else
		BaseClass::OnKeyCodeTyped(code);
}

void CCreditsPanel::OnFinishedClose()
{
	BaseClass::OnFinishedClose();

	engine->ClientCmd_Unrestricted("tfo_mainmenu\n");
}