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

class CHudIronsightOverlay : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudIronsightOverlay, vgui::Panel);

public:
	CHudIronsightOverlay(const char* pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);
	virtual void ApplySchemeSettings(vgui::IScheme* scheme);

protected:

	virtual void Paint();
	virtual void PaintBackground();

	int m_nTexture_FG;
};

DECLARE_HUDELEMENT(CHudIronsightOverlay);

CHudIronsightOverlay::CHudIronsightOverlay(const char* pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudIronsightOverlay")
{
	vgui::Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_nTexture_FG = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_nTexture_FG, "effects/ironsight_pulse", true, false);

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_DIALOGUE | HIDEHUD_INVEHICLE);
}

void CHudIronsightOverlay::Init()
{
	Reset();
}

void CHudIronsightOverlay::Reset(void)
{
	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);
	SetBgColor(Color(0, 0, 0, 0));
	SetFgColor(Color(255, 255, 255, 255));
	SetAlpha(0);
}

void CHudIronsightOverlay::OnThink(void)
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	CBaseCombatWeapon* pWeapon = GetActiveWeapon();
	if (!pWeapon)
		return;

	if (pWeapon->IsIronsighted())
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 256.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
	else
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 0.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
}

void CHudIronsightOverlay::Paint()
{
	int w, h;
	GetSize(w, h);

	surface()->DrawSetColor(GetFgColor());
	surface()->DrawSetTexture(m_nTexture_FG);
	surface()->DrawTexturedRect(0, 0, w, h);
}

void CHudIronsightOverlay::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBorderEnabled(false);
	BaseClass::PaintBackground();
}

void CHudIronsightOverlay::ApplySchemeSettings(vgui::IScheme* scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	int screenWide, screenTall;
	GetHudSize(screenWide, screenTall);
	SetBounds(0, 0, screenWide, screenTall);
}