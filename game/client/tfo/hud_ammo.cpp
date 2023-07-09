//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Displays the ammo (clip wise) you have and the mags left.
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

class CHudAmmo : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudAmmo, vgui::Panel);

public:
	CHudAmmo(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);

protected:

	virtual void Paint();
	virtual void PaintBackground();

	int m_nTexture_Divider;

private:

	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "HUD_TFO_Health");

	CPanelAnimationVarAliasType(float, mag_xpos, "mag_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, mag_ypos, "mag_ypos", "0", "proportional_float");

	CPanelAnimationVarAliasType(float, mags_left_xpos, "mags_left_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, mags_left_ypos, "mags_left_ypos", "0", "proportional_float");

	CPanelAnimationVarAliasType(float, text_x, "text_x", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, divider_x, "divider_x", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, divider_w, "divider_w", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, divider_h, "divider_h", "0", "proportional_float");

	int m_iClipsLeft;
	int m_iCurrAmmo;
};

DECLARE_HUDELEMENT(CHudAmmo);

//------------------------------------------------------------------------
// Purpose: Initialize
//------------------------------------------------------------------------
CHudAmmo::CHudAmmo(const char * pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudAmmo")
{
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_nTexture_Divider = surface()->CreateNewTextureID();

	surface()->DrawSetTextureFile(m_nTexture_Divider, "vgui/hud/ammo_x", true, false);

	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_DIALOGUE | HIDEHUD_INVEHICLE);
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------
void CHudAmmo::Init()
{
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------
void CHudAmmo::Reset(void)
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
void CHudAmmo::OnThink(void)
{
	CBaseCombatWeapon *pWeapon = GetActiveWeapon();
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if (!pWeapon || !pPlayer)
	{
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 0.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
		return;
	}

	g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 256.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);

	m_iClipsLeft = (pPlayer->GetAmmoCount(pWeapon->GetPrimaryAmmoType()) / pWeapon->GetMaxClip1());
	m_iCurrAmmo = pWeapon->m_iClip1;
}

//------------------------------------------------------------------------
// Purpose: Draw Bars / Bullets
//------------------------------------------------------------------------ 
void CHudAmmo::Paint()
{
	C_BaseCombatWeapon * pWeapon = GetActiveWeapon();
	if (!pWeapon)
		return;

	// We don't draw melee weapons, nor special exceptions.
	if (pWeapon->IsRocketLauncher() || pWeapon->IsMeleeWeapon() || !pWeapon->VisibleInWeaponSelection() || pWeapon->IsGrenade())
		return;

	const char* szActiveWeapon = pWeapon->GetClassname();
	szActiveWeapon += 7;

	// Overrides:
	if (!strcmp(szActiveWeapon, "k98ns"))
		szActiveWeapon = "k98";

	bool bHasUnlimitedAmmo = (!strcmp(szActiveWeapon, "fg42"));

	CHudTexture *iconBullets;
	CHudTexture *iconBackground;
	CHudTexture *iconMagsLeft;

	// Background Icon (progress background):
	iconBackground = gHUD.GetIcon(VarArgs("%s_bullet_0", szActiveWeapon));
	if (!iconBackground)
		return;

	// Progress Icon:
	iconBullets = gHUD.GetIcon(VarArgs("%s_bullet_%i", szActiveWeapon, m_iCurrAmmo));
	if (!iconBullets)
		return;

	// Mags Left:
	iconMagsLeft = gHUD.GetIcon(VarArgs("%s_mags_left", szActiveWeapon));
	if (!iconMagsLeft)
		return;

	int iBulletsW = scheme()->GetProportionalScaledValue(iconBullets->rc.left + iconBullets->rc.width) + mag_xpos;
	int iBulletsH = scheme()->GetProportionalScaledValue(iconBullets->rc.top + iconBullets->rc.height) + mag_ypos;

	int iBackgroundW = scheme()->GetProportionalScaledValue(iconBackground->rc.left + iconBackground->rc.width) + mag_xpos;
	int iBackgroundH = scheme()->GetProportionalScaledValue(iconBackground->rc.top + iconBackground->rc.height) + mag_ypos;

	int iMagsW = scheme()->GetProportionalScaledValue(iconMagsLeft->rc.left) + scheme()->GetProportionalScaledValue(iconMagsLeft->rc.width) + mags_left_xpos;
	int iMagsH = scheme()->GetProportionalScaledValue(iconMagsLeft->rc.top) + scheme()->GetProportionalScaledValue(iconMagsLeft->rc.height) + mags_left_ypos;

	int iDividerY = (mags_left_ypos + scheme()->GetProportionalScaledValue(iconMagsLeft->rc.top)) + ((scheme()->GetProportionalScaledValue(iconMagsLeft->rc.height) / 2) - (divider_h / 2));

	surface()->DrawSetColor(GetFgColor());
	surface()->DrawSetTexture(iconBackground->textureId);
	surface()->DrawTexturedRect(mag_xpos + scheme()->GetProportionalScaledValue(iconBackground->rc.left), mag_ypos + scheme()->GetProportionalScaledValue(iconBackground->rc.top), iBackgroundW, iBackgroundH);

	surface()->DrawSetTexture(iconMagsLeft->textureId);
	surface()->DrawTexturedRect(mags_left_xpos + scheme()->GetProportionalScaledValue(iconMagsLeft->rc.left), mags_left_ypos + scheme()->GetProportionalScaledValue(iconMagsLeft->rc.top), iMagsW, iMagsH);

	surface()->DrawSetTexture(iconBullets->textureId);
	surface()->DrawTexturedRect(mag_xpos + scheme()->GetProportionalScaledValue(iconBullets->rc.left), mag_ypos + scheme()->GetProportionalScaledValue(iconBullets->rc.top), iBulletsW, iBulletsH);

	// Skip this step if we have unlimited ammo.
	if (bHasUnlimitedAmmo)
		return;

	surface()->DrawSetTexture(m_nTexture_Divider);
	surface()->DrawTexturedRect(iMagsW + divider_x, iDividerY, iMagsW + divider_w + divider_x, iDividerY + divider_h);

	surface()->DrawSetColor(GetFgColor());
	surface()->DrawSetTextColor(Color(255, 255, 255, 160));
	surface()->DrawSetTextFont(m_hTextFont);

	// Draw Ammo Left Text:
	wchar_t unicode[10];

	V_swprintf_safe(unicode, L"%d", (int)m_iClipsLeft);

	int iAmmoTextX = iMagsW + divider_w + divider_x + text_x;
	int iAmmoTextY = (mags_left_ypos + scheme()->GetProportionalScaledValue(iconMagsLeft->rc.top)) + ((scheme()->GetProportionalScaledValue(iconMagsLeft->rc.height) / 2) - (surface()->GetFontTall(m_hTextFont) / 2));

	surface()->DrawSetTextPos(iAmmoTextX, iAmmoTextY);
	surface()->DrawPrintText(unicode, wcslen(unicode));
}

void CHudAmmo::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBorderEnabled(false);
	BaseClass::PaintBackground();
}