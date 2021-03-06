//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Achievement HUD. This HUD will display whenever you receive an achievement. Check c_achievement_manager.cpp for the call to this HUD element.
//
//=============================================================================//

#include "cbase.h" 
#include "hud.h" 
#include "hud_macros.h" 
#include "c_baseplayer.h"
#include "iclientmode.h" 
#include "vgui/ISurface.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "vgui_controls/AnimationController.h"
#include "basecombatweapon_shared.h"
#include "hud_achievement.h"

#include "tier0/memdbgon.h" 

using namespace vgui;

DECLARE_HUDELEMENT(CHudAchievement);

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------

CHudAchievement::CHudAchievement(const char * pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudAchievement")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_nTexture_FG = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_nTexture_FG, "vgui/achievements/achievement_earned", true, false);

	SetHiddenBits(HIDEHUD_PLAYERDEAD);
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------
void CHudAchievement::Init()
{
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------
void CHudAchievement::Reset(void)
{
	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);
	SetBgColor(Color(0, 0, 0, 0));
	SetFgColor(Color(255, 255, 255, 255));
	SetAlpha(0);

	ShouldShow = false;
	flShowDuration = 0.0f;
}

//------------------------------------------------------------------------
// Purpose: Show the HUD for a few seconds.
//------------------------------------------------------------------------
void CHudAchievement::ShowAchievement(void)
{
	flShowDuration = gpGlobals->curtime + 2.0f;
	ShouldShow = true;
	vgui::surface()->PlaySound("hud/achievement_earned.wav");
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------
void CHudAchievement::OnThink(void)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	if (ShouldShow)
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 256.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);

	if (gpGlobals->curtime >= flShowDuration)
	{
		flShowDuration = 0.0f;
		ShouldShow = false;
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 0.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
	}
}

//------------------------------------------------------------------------
// Purpose: Nothin
//------------------------------------------------------------------------
void CHudAchievement::Paint()
{
}

void CHudAchievement::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	surface()->DrawSetColor(GetFgColor());
	surface()->DrawSetTexture(m_nTexture_FG);
	surface()->DrawTexturedRect(0, 0, GetWide(), GetTall());
	SetPaintBorderEnabled(false);
	BaseClass::PaintBackground();
}