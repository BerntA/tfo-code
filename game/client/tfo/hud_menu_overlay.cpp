//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Fades in an overlay on death.
//
//=============================================================================//

#include "cbase.h" 
#include "hud.h" 
#include "hud_macros.h" 
#include "c_baseplayer.h"
#include "c_basehlplayer.h"
#include "iclientmode.h" 
#include "vgui/ISurface.h"
#include "ienginevgui.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "vgui_controls/AnimationController.h"
#include "basecombatweapon_shared.h"

#include "tier0/memdbgon.h" 

using namespace vgui;

class CHudMenuOverlay : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudMenuOverlay, vgui::Panel);

public:
	CHudMenuOverlay(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);

protected:
	virtual void Paint();
	virtual void PaintBackground();

	int m_nTexture_FG;
};

DECLARE_HUDELEMENT(CHudMenuOverlay);

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------
CHudMenuOverlay::CHudMenuOverlay(const char * pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudMenuOverlay")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_nTexture_FG = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_nTexture_FG, "vgui/mainmenu/gameoverlay", true, false);

	int screenWide, screenTall;
	GetHudSize(screenWide, screenTall);
	SetBounds(0, 0, screenWide, screenTall);

	SetHiddenBits(NULL);
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------
void CHudMenuOverlay::Init()
{
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------
void CHudMenuOverlay::Reset(void)
{
	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);
	SetBgColor(Color(0, 0, 0, 0));
	SetFgColor(Color(255, 255, 255, 255));
	SetAlpha(0);
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------
void CHudMenuOverlay::OnThink(void)
{
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();

	if (pPlayer)
	{
		if (!pPlayer->IsAlive())
			g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 150.0f, 0.0f, 1.0f, AnimationController::INTERPOLATOR_LINEAR);
		else
			g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 0.0f, 0.0f, 1.0f, AnimationController::INTERPOLATOR_LINEAR);
	}
	else
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 0.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
}

void CHudMenuOverlay::Paint()
{
}

void CHudMenuOverlay::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	surface()->DrawSetColor(GetFgColor());
	surface()->DrawSetTexture(m_nTexture_FG);

	int w, h;
	GetHudSize(w, h);
	if (GetWide() != w)
		SetWide(w);

	if (GetTall() != h)
		SetTall(h);

	surface()->DrawTexturedRect(0, 0, w, h);

	SetPaintBorderEnabled(false);
	BaseClass::PaintBackground();
}