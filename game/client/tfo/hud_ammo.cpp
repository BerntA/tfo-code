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
	CHudAmmo(const char* pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);

protected:

	virtual void Paint();
	virtual void PaintBackground();

	int m_nTexture_Divider;

private:

	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "HUD_TFO_Health");

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
CHudAmmo::CHudAmmo(const char* pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudAmmo")
{
	vgui::Panel* pParent = g_pClientMode->GetViewport();
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
	CBaseCombatWeapon* pWeapon = GetActiveWeapon();
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();

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
	C_BaseCombatWeapon* pWeapon = GetActiveWeapon();
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

	CHudTexture* iconBullets;
	CHudTexture* iconBackground;
	CHudTexture* iconMagsLeft;

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

	int iDividerX = scheme()->GetProportionalScaledValue(iconMagsLeft->scaled.x) + scheme()->GetProportionalScaledValue(iconMagsLeft->scaled.wide) + divider_x;
	int iDividerY = scheme()->GetProportionalScaledValue(iconMagsLeft->scaled.y) + (scheme()->GetProportionalScaledValue(iconMagsLeft->scaled.tall) / 2) - (divider_h / 2);

	const Color fg = GetFgColor();

	iconMagsLeft->DrawSelfCropped(fg);
	iconBackground->DrawSelfCropped(fg);
	iconBackground->DrawSelfCropped(fg, iconBullets->textureId);

	// Skip this step if we have unlimited ammo.
	if (bHasUnlimitedAmmo)
		return;

	surface()->DrawSetColor(fg);
	surface()->DrawSetTexture(m_nTexture_Divider);
	surface()->DrawTexturedRect(iDividerX, iDividerY, iDividerX + divider_w, iDividerY + divider_h);

	surface()->DrawSetColor(fg);
	surface()->DrawSetTextColor(Color(255, 255, 255, 160));
	surface()->DrawSetTextFont(m_hTextFont);

	// Draw Ammo Left Text:
	wchar_t unicode[10];

	V_swprintf_safe(unicode, L"%d", m_iClipsLeft);

	int iAmmoTextX = iDividerX + divider_w;
	int iAmmoTextY = scheme()->GetProportionalScaledValue(iconMagsLeft->scaled.y) + (scheme()->GetProportionalScaledValue(iconMagsLeft->scaled.tall) / 2) - (surface()->GetFontTall(m_hTextFont) / 2);

	surface()->DrawSetTextPos(iAmmoTextX, iAmmoTextY);
	surface()->DrawPrintText(unicode, wcslen(unicode));
}

void CHudAmmo::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBorderEnabled(false);
	BaseClass::PaintBackground();
}