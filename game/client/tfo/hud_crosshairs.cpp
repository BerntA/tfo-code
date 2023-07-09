//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Shows different crosshairs dynamically.
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
#include "basecombatweapon_shared.h"
#include "hud_crosshairs.h"

#include "tier0/memdbgon.h" 

using namespace vgui;

// ConVar Defines
ConVar cl_hand_c("cl_hand_c", "0", FCVAR_CLIENTDLL, "Toggle Hand Cursor HUD");
ConVar cl_turret_c("cl_turret_c", "0", FCVAR_CLIENTDLL, "Turret Crosshair");

DECLARE_HUDELEMENT(CHudCrosshairs);

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------
CHudCrosshairs::CHudCrosshairs(const char * pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudPointers")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_DIALOGUE | HIDEHUD_INVEHICLE);
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------
void CHudCrosshairs::Init()
{
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------
void CHudCrosshairs::Reset(void)
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
void CHudCrosshairs::OnThink(void)
{
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();

	if ((cl_hand_c.GetBool() || cl_turret_c.GetBool()) && pPlayer)
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 256.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
	else
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "alpha", 0.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
}

void CHudCrosshairs::Paint()
{
	CHudTexture *pPointerIcon = NULL;

	int x, y, w, h;
	GetBounds(x, y, w, h);

	if (cl_hand_c.GetBool())
	{
		pPointerIcon = gHUD.GetIcon("pointer_hand");
		y = scheme()->GetProportionalScaledValue(240);
	}
	else if (cl_turret_c.GetBool())
	{
		pPointerIcon = gHUD.GetIcon("pointer_turret");
		y = scheme()->GetProportionalScaledValue(232);
	}

	if (!pPointerIcon)
		return;

	SetBounds(x, y, w, h);

	surface()->DrawSetColor(GetFgColor());
	surface()->DrawSetTexture(pPointerIcon->textureId);
	surface()->DrawTexturedRect(0, 0, GetWide(), GetTall());
}

void CHudCrosshairs::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBorderEnabled(false);
	BaseClass::PaintBackground();
}

void CHudCrosshairs::Hide(void)
{
	cl_hand_c.SetValue(0);
	cl_turret_c.SetValue(0);
}