//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Stamina/Sprint Bar HUD
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
#include "hud_numericdisplay.h"
#include "vgui_controls/AnimationController.h"

#include "tier0/memdbgon.h" 

using namespace vgui;

class CHudSprint : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudSprint, vgui::Panel);

public:

	CHudSprint(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);

protected:
	virtual void Paint();
	virtual void PaintBackground();

	int m_nTexture_Bar[58];
	int m_nTexture_BG;

private:

	float m_flStamina;
};

DECLARE_HUDELEMENT(CHudSprint);

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------
CHudSprint::CHudSprint(const char * pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudSprint")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_nTexture_BG = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_nTexture_BG, "vgui/hud/sprint/bar_0", true, false);

	for (int i = 0; i < _ARRAYSIZE(m_nTexture_Bar); i++)
	{
		m_nTexture_Bar[i] = surface()->CreateNewTextureID();
		surface()->DrawSetTextureFile(m_nTexture_Bar[i], VarArgs("vgui/hud/sprint/bar_%i", i), true, false);
	}

	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_DIALOGUE | HIDEHUD_INVEHICLE);
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------
void CHudSprint::Init()
{
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------
void CHudSprint::Reset(void)
{
	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);
	m_flStamina = 100;
	SetBgColor(Color(0, 0, 0, 0));
	SetFgColor(Color(255, 255, 255, 255));
	SetAlpha(255);
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------
void CHudSprint::OnThink(void)
{
	float newStamina = 0;
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	newStamina = pPlayer->m_HL2Local.m_flSuitPower;

	if (newStamina == m_flStamina)
		return;

	m_flStamina = newStamina;
}

//------------------------------------------------------------------------
// Purpose: Bar:
//------------------------------------------------------------------------
void CHudSprint::Paint()
{
	float flStaminaPercent = (m_flStamina / 100);
	float flBar = (flStaminaPercent * (_ARRAYSIZE(m_nTexture_Bar) - 1));
	flBar = round(flBar);

	// Clamp:
	if (flBar < 0)
		flBar = 0;
	else if (flBar >= _ARRAYSIZE(m_nTexture_Bar))
		flBar = (_ARRAYSIZE(m_nTexture_Bar) - 1);

	surface()->DrawSetColor(GetFgColor());
	surface()->DrawSetTexture(m_nTexture_Bar[(int)flBar]);
	surface()->DrawTexturedRect(0, 0, GetWide(), GetTall());
}

void CHudSprint::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	surface()->DrawSetColor(GetFgColor());
	surface()->DrawSetTexture(m_nTexture_BG);
	surface()->DrawTexturedRect(0, 0, GetWide(), GetTall());
	SetPaintBorderEnabled(false);
	BaseClass::PaintBackground();
}