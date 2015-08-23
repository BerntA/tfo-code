//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Veins - Near Death HUD
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "c_basehlplayer.h"
#include "vgui_controls/Panel.h"
#include "vgui_controls/AnimationController.h"
#include "vgui/ISurface.h"
#include <vgui/ILocalize.h>
#include "ienginevgui.h"

#include "tier0/memdbgon.h" 

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Shows the veins
//-----------------------------------------------------------------------------
void cb_veinsoverlay(IConVar *pConVar, const char *pOldString, float flOldValue);
ConVar r_veinsoverlay("r_veinsoverlay", "1", FCVAR_ARCHIVE, "Toggle visibility of veins overlay.", false, 0, true, 1, cb_veinsoverlay);

static int veins_lastactive;
void cb_veinsoverlay(IConVar *pConVar, const char *pOldString, float flOldValue)
{
	veins_lastactive = gpGlobals->curtime + 2;
}

class CHudVeins : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudVeins, vgui::Panel);

public:
	CHudVeins(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *scheme);
	virtual void Paint();
	virtual void PaintBackground();

private:

	int m_nTexture_FG;
};

DECLARE_HUDELEMENT(CHudVeins);

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------
CHudVeins::CHudVeins(const char * pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudVeins")
{
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_nTexture_FG = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_nTexture_FG, "effects/veins", true, false);

	int screenWide, screenTall;
	GetHudSize(screenWide, screenTall);
	SetBounds(0, 0, screenWide, screenTall);

	r_veinsoverlay.SetValue(1);
	ConVar* flicker = cvar->FindVar("r_flashlightforceflicker");
	if (flicker)
		flicker->SetValue(0);

	SetHiddenBits(HIDEHUD_DIALOGUE);
}

void CHudVeins::ApplySchemeSettings(vgui::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------
void CHudVeins::Init()
{
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------
void CHudVeins::Reset(void)
{
	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);
	SetBgColor(Color(0, 0, 0, 0));
	SetFgColor(Color(255, 255, 255, 160));
	SetAlpha(0);
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------
void CHudVeins::OnThink(void)
{
}

void CHudVeins::Paint()
{
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
	{
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 0.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
		return;
	}

	if (veins_lastactive < gpGlobals->curtime)
		r_veinsoverlay.SetValue(1);

	if (pPlayer->GetHealth() <= 98 && r_veinsoverlay.GetBool())
	{
		float a = pPlayer->GetHealth();
		a -= 90;
		a *= 98.5;
		a *= -1;
		a += 255;

		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", a, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
	}
	else
	{
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 0.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
	}

	surface()->DrawSetColor(GetFgColor());
	surface()->DrawSetTexture(m_nTexture_FG);

	int w, h;
	GetHudSize(w, h);
	if (GetWide() != w)
		SetWide(w);

	if (GetTall() != h)
		SetTall(h);

	surface()->DrawTexturedRect(0, 0, w, h);
}

void CHudVeins::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBorderEnabled(false);
	BaseClass::PaintBackground();
}