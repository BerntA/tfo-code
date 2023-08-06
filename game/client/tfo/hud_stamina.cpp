//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Stamina Exhaustion HUD
//
//=============================================================================//

#include "cbase.h" 
#include "hud.h" 
#include "hud_macros.h" 
#include "c_baseplayer.h"
#include "c_basehlplayer.h"
#include "iclientmode.h" 
#include "vgui/ISurface.h"
#include "hudelement.h"
#include "vgui_controls/AnimationController.h"

#include "tier0/memdbgon.h"

using namespace vgui;

class CHudStamina : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudStamina, vgui::Panel);

public:

	CHudStamina(const char* pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);
	virtual void ApplySchemeSettings(vgui::IScheme* scheme);

protected:
	virtual void Paint();
	virtual void PaintBackground();

	int m_nTexture_FG;

private:

	bool bShouldShow;
};

DECLARE_HUDELEMENT(CHudStamina);

CHudStamina::CHudStamina(const char* pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudExhaustion")
{
	vgui::Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_nTexture_FG = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_nTexture_FG, "effects/sprint_pulse", true, false);

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_DIALOGUE);
}

void CHudStamina::Init()
{
	Reset();
}

void CHudStamina::Reset(void)
{
	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);
	SetBgColor(Color(0, 0, 0, 0));
	SetFgColor(Color(255, 255, 255, 255));
	SetAlpha(0);
	bShouldShow = false;
}

void CHudStamina::ApplySchemeSettings(vgui::IScheme* scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	int screenWide, screenTall;
	GetHudSize(screenWide, screenTall);
	SetBounds(0, 0, screenWide, screenTall);
}

void CHudStamina::OnThink(void)
{
	float flStamina = 0;
	C_BaseHLPlayer* pPlayer = (C_BaseHLPlayer*)C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
	{
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 0.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
		return;
	}

	flStamina = pPlayer->m_HL2Local.m_flSuitPower;

	if (flStamina < 20.0f)
		bShouldShow = true;
	else if (flStamina > 80.0f)
		bShouldShow = false;

	if (bShouldShow)
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 256.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
	else
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 0.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
}

void CHudStamina::Paint()
{
	int w, h;
	GetSize(w, h);

	surface()->DrawSetColor(GetFgColor());
	surface()->DrawSetTexture(m_nTexture_FG);
	surface()->DrawTexturedRect(0, 0, w, h);
}

void CHudStamina::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBorderEnabled(false);
	BaseClass::PaintBackground();
}