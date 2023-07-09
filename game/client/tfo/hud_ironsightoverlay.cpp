//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Dynamic Ironsight Blur / Screen Darkener
//
//=============================================================================//

#include "cbase.h" 
#include "hud.h" 
#include "hud_macros.h" 
#include "c_baseplayer.h"
#include "iclientmode.h" 
#include "vgui/ISurface.h"
#include "hudelement.h"
#include "vgui_controls/AnimationController.h"
#include "basecombatweapon_shared.h"

#include "tier0/memdbgon.h" 

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Draw Ironsight Overlay
//-----------------------------------------------------------------------------
class CHudIronsightOverlay : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudIronsightOverlay, vgui::Panel);

public:
	CHudIronsightOverlay(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);

protected:

	virtual void Paint();
	virtual void PaintBackground();

	int m_nTexture_FG;
};

DECLARE_HUDELEMENT(CHudIronsightOverlay);

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------
CHudIronsightOverlay::CHudIronsightOverlay(const char * pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudIronsightOverlay")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_nTexture_FG = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_nTexture_FG, "effects/ironsight_pulse", true, false);

	int screenWide, screenTall;
	GetHudSize(screenWide, screenTall);
	SetBounds(0, 0, screenWide, screenTall);

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_DIALOGUE | HIDEHUD_INVEHICLE);
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------
void CHudIronsightOverlay::Init()
{
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------
void CHudIronsightOverlay::Reset(void)
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
void CHudIronsightOverlay::OnThink(void)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	CBaseCombatWeapon *pWeapon = GetActiveWeapon();
	if (!pWeapon)
		return;

	if (pWeapon->IsIronsighted())
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 256.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
	else
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 0.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
}

//------------------------------------------------------------------------
// Purpose: Draw Overlay
//------------------------------------------------------------------------
void CHudIronsightOverlay::Paint()
{
	// Force Fullscreen:
	vgui::surface()->DrawSetColor(GetFgColor());
	surface()->DrawSetTexture(m_nTexture_FG);

	int w, h;
	GetHudSize(w, h);
	if (GetWide() != w)
		SetWide(w);

	if (GetTall() != h)
		SetTall(h);

	surface()->DrawTexturedRect(0, 0, w, h);
}

void CHudIronsightOverlay::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBorderEnabled(false);
	BaseClass::PaintBackground();
}