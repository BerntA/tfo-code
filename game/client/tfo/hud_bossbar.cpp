//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Displays the bossbar when fighting npcs. This is sent from tfo_boss_engager. (usermessage)
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
#include "vgui_controls/AnimationController.h"
#include <vgui/ILocalize.h>

#include "tier0/memdbgon.h" 

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: TFO Boss HUD
//-----------------------------------------------------------------------------

class CHudBossBar : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudBossBar, vgui::Panel);

public:
	CHudBossBar(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);

	void MsgFunc_BossData(bf_read &msg);

protected:
	virtual void Paint();
	virtual void PaintBackground();

	// Base Textures	
	int m_nTextureBarBG;
	int m_nTextureBarFG;

	// Logic
	float m_flHealthFraction;
	bool m_bShouldShow;
	wchar_t szEntName[64];

private:

	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "HUD_TFO_Health");
	CPanelAnimationVar(Color, m_TextColor, "TextColor", "255 255 255 160");

	CPanelAnimationVarAliasType(float, m_flBarInsetX, "BarInsetX", "26", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarInsetY, "BarInsetY", "3", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarWidth, "BarWidth", "84", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarHeight, "BarHeight", "4", "proportional_float");

	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "0", "proportional_float");
};

DECLARE_HUDELEMENT(CHudBossBar);
DECLARE_HUD_MESSAGE(CHudBossBar, BossData);

//------------------------------------------------------------------------
// Purpose: Constructor - Precached images
//------------------------------------------------------------------------
CHudBossBar::CHudBossBar(const char * pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudBossBar")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_nTextureBarBG = surface()->CreateNewTextureID();
	m_nTextureBarFG = surface()->CreateNewTextureID();

	surface()->DrawSetTextureFile(m_nTextureBarBG, "vgui/hud/bossbar/bg", true, false);
	surface()->DrawSetTextureFile(m_nTextureBarFG, "vgui/hud/bossbar/fg", true, false);

	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_DIALOGUE);
}

//------------------------------------------------------------------------
// Purpose: Init reset func duh...
//------------------------------------------------------------------------
void CHudBossBar::Init()
{
	HOOK_HUD_MESSAGE(CHudBossBar, BossData);
	Reset();
}

//------------------------------------------------------------------------
// Purpose: Default on spawn, keep alpha to 0!
//-----------------------------------------------------------------------
void CHudBossBar::Reset(void)
{
	m_bShouldShow = false;
	m_flHealthFraction = 0.0f;

	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);
	SetBgColor(Color(0, 0, 0, 0));
	SetFgColor(Color(255, 255, 255, 255));
	SetAlpha(0);
}

//------------------------------------------------------------------------
// Purpose: Check if we need to display this HUD:
//------------------------------------------------------------------------
void CHudBossBar::OnThink(void)
{
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if (!pPlayer || !engine->IsInGame() || engine->IsLevelMainMenuBackground())
	{
		m_bShouldShow = false;
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 0.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
		return;
	}

	if (m_bShouldShow && pPlayer->IsAlive())
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 256.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
	else
	{
		m_bShouldShow = false;
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 0.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
		return;
	}
}

//------------------------------------------------------------------------
// Purpose: Paint func
//------------------------------------------------------------------------
void CHudBossBar::Paint()
{
	surface()->DrawSetColor(GetFgColor());
	surface()->DrawSetTexture(m_nTextureBarBG);
	surface()->DrawTexturedRect(m_flBarInsetX, m_flBarInsetY, m_flBarWidth + m_flBarInsetX, m_flBarHeight + m_flBarInsetY);

	surface()->DrawSetColor(GetFgColor());
	surface()->DrawSetTexture(m_nTextureBarFG);
	surface()->DrawTexturedSubRect(m_flBarInsetX, m_flBarInsetY, (m_flBarWidth * m_flHealthFraction) + m_flBarInsetX, m_flBarHeight + m_flBarInsetY, 0.0f, 0.0f, m_flHealthFraction, 1.0f);

	surface()->DrawSetColor(GetFgColor());
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(m_TextColor);

	// Set Lenght/Size
	int positionX = m_flBarInsetX + ((m_flBarWidth / 2) - (UTIL_ComputeStringWidth(m_hTextFont, szEntName) / 2));
	vgui::surface()->DrawSetTextPos(positionX, text_ypos);
	surface()->DrawUnicodeString(szEntName);
}

void CHudBossBar::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBorderEnabled(false);
	BaseClass::PaintBackground();
}

// Get packets from the server:
void CHudBossBar::MsgFunc_BossData(bf_read& msg)
{
	int iShouldShow = msg.ReadByte();
	float flCurrHP = msg.ReadFloat();
	float flMaxHP = msg.ReadFloat();

	m_flHealthFraction = clamp((flCurrHP / flMaxHP), 0.0f, 1.0f);

	// Read the string(s)
	char szString[64];
	msg.ReadString(szString, 64);
	g_pVGuiLocalize->ConvertANSIToUnicode(szString, szEntName, 64);

	m_bShouldShow = (iShouldShow >= 1);
}