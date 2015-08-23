//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Displays the ammo for the Panzerfaust, I've separated this from hud_ammo because this one is quite different and is kind of 'unused'.
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "c_baseplayer.h"
#include "vgui_controls/Panel.h"
#include "vgui_controls/AnimationController.h"
#include "vgui/ISurface.h"
#include <vgui/ILocalize.h>
#include "hud_numericdisplay.h"
#include "vgui_controls/AnimationController.h"

#include "tier0/memdbgon.h" 

using namespace vgui;

class CHudPanzer : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudPanzer, vgui::Panel);

public:
	CHudPanzer(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);

protected:

	virtual void Paint();
	virtual void PaintBackground();

	int m_nTexture_BG;
	int m_nTexture_Bar[4];

private:

	int m_iCurrAmmo;
};

DECLARE_HUDELEMENT(CHudPanzer);

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------
CHudPanzer::CHudPanzer(const char * pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudPanzer")
{
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_nTexture_BG = surface()->CreateNewTextureID();

	surface()->DrawSetTextureFile(m_nTexture_BG, "vgui/hud/panzer/rocket_0", true, false);

	for (int i = 0; i < _ARRAYSIZE(m_nTexture_Bar); i++)
	{
		m_nTexture_Bar[i] = surface()->CreateNewTextureID();
		surface()->DrawSetTextureFile(m_nTexture_Bar[i], VarArgs("vgui/hud/panzer/rocket_%i", i), true, false);
	}

	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_DIALOGUE | HIDEHUD_INVEHICLE);
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------
void CHudPanzer::Init()
{
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------
void CHudPanzer::Reset(void)
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
void CHudPanzer::OnThink(void)
{
	CBaseCombatWeapon *pWeapon = GetActiveWeapon();
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if (!pWeapon || !pPlayer || (pWeapon && strcmp(pWeapon->GetClassname(), "weapon_panzer")))
	{
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 0.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
		return;
	}

	g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 256.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);

	m_iCurrAmmo = pPlayer->GetAmmoCount(pWeapon->GetPrimaryAmmoType());
}

//------------------------------------------------------------------------
// Purpose: Draw Bars / Bullets
//------------------------------------------------------------------------ 
void CHudPanzer::Paint()
{
	// Make sure we never go out of bounds.
	if (m_iCurrAmmo >= _ARRAYSIZE(m_nTexture_Bar))
		return;

	surface()->DrawSetColor(GetFgColor());
	surface()->DrawSetTexture(m_nTexture_Bar[m_iCurrAmmo]);
	surface()->DrawTexturedRect(0, 0, GetWide(), GetTall());
}

void CHudPanzer::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	vgui::surface()->DrawSetColor(GetFgColor());
	surface()->DrawSetTexture(m_nTexture_BG);
	surface()->DrawTexturedRect(0, 0, GetWide(), GetTall());
	SetPaintBorderEnabled(false);
	BaseClass::PaintBackground();
}