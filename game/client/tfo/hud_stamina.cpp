//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Stamina Exhaustion HUD
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

//-----------------------------------------------------------------------------
// Purpose: Draw Stam Bar:
//-----------------------------------------------------------------------------
class CHudStamina : public CHudElement, public vgui::Panel
{ 
	DECLARE_CLASS_SIMPLE(CHudStamina, vgui::Panel);

public:

	CHudStamina(const char * pElementName);

	virtual void Init (void);
	virtual void Reset (void);
	virtual void OnThink (void);

protected:
	virtual void Paint();
	virtual void PaintBackground();

	int m_nTexture_FG;

private:

	bool bShouldShow;
};

DECLARE_HUDELEMENT (CHudStamina);

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------
CHudStamina::CHudStamina (const char * pElementName) : CHudElement (pElementName), BaseClass (NULL, "HudExhaustion")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent (pParent);

	m_nTexture_FG = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_nTexture_FG, "effects/sprint_pulse", true, false );

	int screenWide, screenTall;
	GetHudSize(screenWide, screenTall);
	SetBounds(0, 0, screenWide, screenTall);

	SetHiddenBits ( HIDEHUD_PLAYERDEAD | HIDEHUD_DIALOGUE);
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------
void CHudStamina::Init()
{
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------- 
void CHudStamina::Reset (void)
{
	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);
	SetBgColor ( Color (0, 0, 0, 0) );
	SetFgColor ( Color( 255, 255, 255, 255 ) );
	SetAlpha( 0 );
	bShouldShow = false;
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------
void CHudStamina::OnThink (void)
{
	float flStamina = 0;
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer*)C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
	{
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand( this, "alpha", 0.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
		return;
	}

	flStamina = pPlayer->m_HL2Local.m_flSuitPower;

	if (flStamina < 20.0f)
		bShouldShow = true;
	else if (flStamina > 80.0f)
		bShouldShow = false;

	if ( bShouldShow )
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand( this, "alpha", 256.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
	else
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand( this, "alpha", 0.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
}

//------------------------------------------------------------------------
// Purpose: Draw Bars
//------------------------------------------------------------------------
void CHudStamina::Paint()
{
	vgui::surface()->DrawSetColor(GetFgColor());
	surface()->DrawSetTexture( m_nTexture_FG );

	int w,h;
	GetHudSize( w, h);
	if (GetWide() != w)
		SetWide(w);

	if (GetTall() != h)
		SetTall(h);

	surface()->DrawTexturedRect( 0, 0, w, h );
}

void CHudStamina::PaintBackground()
{
	SetBgColor(Color(0,0,0,0));
	SetPaintBorderEnabled(false);
	BaseClass::PaintBackground();
}