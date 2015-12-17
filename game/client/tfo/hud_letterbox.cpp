//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Fades down and up two dividers to form a 'letterbox' (no dividers going vertically) then it fades in the chapter title. Use ChapterTitle usermsg to toggle this hud.
//
//=============================================================================//

#include "cbase.h" 
#include "hud.h" 
#include "hud_macros.h" 
#include "c_baseplayer.h"
#include "c_basehlplayer.h"
#include "iclientmode.h" 
#include "vgui/ISurface.h"
#include "usermessages.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "vgui_controls/AnimationController.h"
#include <vgui/ILocalize.h>
#include "hud_letterbox.h"

#include "tier0/memdbgon.h" 

using namespace vgui;

DECLARE_HUDELEMENT(CHudLetterbox);
DECLARE_HUD_MESSAGE(CHudLetterbox, ChapterTitle);

//------------------------------------------------------------------------
// Purpose: Constructor 
//------------------------------------------------------------------------
CHudLetterbox::CHudLetterbox(const char * pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudLetterbox")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	int screenWide, screenTall;
	GetHudSize(screenWide, screenTall);
	SetBounds(0, 0, screenWide, screenTall);

	SetHiddenBits(HIDEHUD_PLAYERDEAD);
}

//------------------------------------------------------------------------
// Purpose: Init reset func duh...
//------------------------------------------------------------------------
void CHudLetterbox::Init()
{
	HOOK_HUD_MESSAGE(CHudLetterbox, ChapterTitle);
	Reset();
}

//------------------------------------------------------------------------
// Purpose: Default on spawn, keep alpha to 0!
//-----------------------------------------------------------------------
void CHudLetterbox::Reset(void)
{
	m_bFadeIn = false;
	m_bFadeOut = false;

	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);
	SetBgColor(Color(0, 0, 0, 0));
	SetFgColor(Color(255, 255, 255, 255));
	SetAlpha(0);
}

//------------------------------------------------------------------------
// Purpose: Check if we need to display this HUD:
//------------------------------------------------------------------------
void CHudLetterbox::OnThink(void)
{
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if (!pPlayer || !engine->IsInGame() || engine->IsLevelMainMenuBackground() || (!m_bFadeOut && !m_bFadeIn))
	{
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 0.0f, 0.0f, 0.25f, AnimationController::INTERPOLATOR_LINEAR);
		return;
	}
}

//------------------------------------------------------------------------
// Purpose: Paint func
//------------------------------------------------------------------------
void CHudLetterbox::Paint()
{
	if (!m_bFadeOut && !m_bFadeIn)
		return;

	int w, h;
	GetHudSize(w, h);
	if (GetWide() != w)
		SetWide(w);

	if (GetTall() != h)
		SetTall(h);

	float flHeight = ((float)h * 0.20f); // 20% of screen height
	float flWidth = ((float)GetAlpha() / 255.0f) * w;
	float y_pos = h - flHeight;

	Color textColor = m_TextColor;
	textColor[3] = GetAlpha();

	surface()->DrawSetColor(Color(0, 0, 0, GetAlpha()));
	surface()->DrawFilledRect(0, 0, flWidth, flHeight);
	surface()->DrawFilledRect(0, y_pos, flWidth, y_pos + flHeight);

	surface()->DrawSetColor(m_TextColor);
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(m_TextColor);

	// Set Lenght/Size
	vgui::surface()->DrawSetTextPos(text_xpos, text_ypos);
	surface()->DrawUnicodeString(wcsChapterTitle);

	if (m_bFadeIn && (GetAlpha() >= 255))
	{
		m_bFadeIn = false;
		m_bFadeOut = true;
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 0.0f, 2.2f, 0.85f, AnimationController::INTERPOLATOR_SIMPLESPLINE);
	}
	else if (m_bFadeOut && (GetAlpha() <= 0))
		m_bFadeOut = false;
}

void CHudLetterbox::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBorderEnabled(false);
	BaseClass::PaintBackground();
}

// Get packets from the server:
void CHudLetterbox::MsgFunc_ChapterTitle(bf_read &msg)
{
	char szTitle[64];
	msg.ReadString(szTitle, 64);
	g_pVGuiLocalize->ConvertANSIToUnicode(szTitle, wcsChapterTitle, 64);
	m_bFadeIn = true;
	g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 256.0f, 0.0f, 2.0f, AnimationController::INTERPOLATOR_SIMPLESPLINE);
}