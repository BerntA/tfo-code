//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Health Bar HUD
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
#include "vgui_controls/AnimationController.h"

#include "tier0/memdbgon.h" 

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Show Health Bar
//-----------------------------------------------------------------------------
class CHudHealth : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudHealth, vgui::Panel);

public:

	CHudHealth(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);

protected:

	virtual void Paint();
	virtual void PaintBackground();

	int m_nTexture_BG;
	int m_nTexture_Bar[11];

private:

	CPanelAnimationVarAliasType(float, m_flHealthkitX, "healthkit_x", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flHealthkitY, "healthkit_y", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flHealthkitW, "healthkit_w", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flHealthkitH, "healthkit_h", "0", "proportional_float");

	float m_flHealth;
};

DECLARE_HUDELEMENT(CHudHealth);

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------
CHudHealth::CHudHealth(const char * pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudHealth")
{
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_nTexture_BG = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_nTexture_BG, "vgui/hud/health/bar_0", true, false);

	for (int i = 0; i < _ARRAYSIZE(m_nTexture_Bar); i++)
	{
		m_nTexture_Bar[i] = surface()->CreateNewTextureID();
		surface()->DrawSetTextureFile(m_nTexture_Bar[i], VarArgs("vgui/hud/health/bar_%i", i), true, false);
	}

	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_DIALOGUE | HIDEHUD_INVEHICLE);
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------
void CHudHealth::Init()
{
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------
void CHudHealth::Reset(void)
{
	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);
	m_flHealth = 100;
	SetBgColor(Color(0, 0, 0, 0));
	SetFgColor(Color(255, 255, 255, 255));
	SetAlpha(255);
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------
void CHudHealth::OnThink(void)
{
	float newHealth = 0;
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	// Never below zero 
	newHealth = max(pPlayer->GetHealth(), 0);

	if (newHealth == m_flHealth)
		return;

	m_flHealth = newHealth;
}

//------------------------------------------------------------------------
// Purpose: Draw the hp bar:
//------------------------------------------------------------------------
void CHudHealth::Paint()
{
	float flHPPercent = (m_flHealth / 100);
	float flBar = (flHPPercent * (_ARRAYSIZE(m_nTexture_Bar) - 1));
	flBar = round(flBar);

	// Clamp:
	if (flBar < 0)
		flBar = 0;
	else if (flBar >= _ARRAYSIZE(m_nTexture_Bar))
		flBar = (_ARRAYSIZE(m_nTexture_Bar) - 1);

	surface()->DrawSetColor(GetFgColor());
	surface()->DrawSetTexture(m_nTexture_Bar[(int)flBar]);
	surface()->DrawTexturedRect(0, 0, GetWide(), GetTall());

	// Draw the healthkit status to indicate if we have a healthkit or not.
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	CHudTexture* pHealthkitIcon = NULL;

	if (pPlayer->m_bHasHealthkit)
		pHealthkitIcon = gHUD.GetIcon((pPlayer->m_iHealth <= 35) ? "healthkit_critical" : "healthkit");
	else
		pHealthkitIcon = gHUD.GetIcon("healthkit_inactive");

	if (!pHealthkitIcon)
		return;

	surface()->DrawSetTexture(pHealthkitIcon->textureId);
	surface()->DrawTexturedRect(m_flHealthkitX, m_flHealthkitY, m_flHealthkitX + m_flHealthkitW, m_flHealthkitY + m_flHealthkitH);
}

void CHudHealth::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	surface()->DrawSetColor(GetFgColor());
	surface()->DrawSetTexture(m_nTexture_BG);
	surface()->DrawTexturedRect(0, 0, GetWide(), GetTall());
	SetPaintBorderEnabled(false);
	BaseClass::PaintBackground();
}