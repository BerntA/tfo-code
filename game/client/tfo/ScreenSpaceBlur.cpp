//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Activates a full screenspace motion blur (overrides motion blur in viewpostprocess, thus we do not init any material, or do any explicit rendering here!!!).
//
//=============================================================================//

#include "cbase.h"
#include "ScreenSpaceEffects.h"
#include "rendertexture.h"
#include "KeyValues.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar mat_screen_blur_fade("mat_screen_blur_fade", "0.2"); // Time to fade in/out.
static ConVar mat_screen_blur_scale("mat_screen_blur_scale", "0.01"); // Blur Scale (box blur style).

bool g_bScreenBlurEnabled = false;
float g_fScreenBlurValue = 0.0f;

class CScreenSpaceBlurEffect : public IScreenSpaceEffect
{
public:
	CScreenSpaceBlurEffect(void) :
		m_flFadeStart(0.0f),
		m_bFadeOut(false) {}

	void Init(void);
	void Shutdown(void);
	void SetParameters(KeyValues* params);
	void Enable(bool bEnable);
	bool IsEnabled() { return g_bScreenBlurEnabled; }
	void Render(int x, int y, int w, int h);

private:
	float		m_flFadeStart;
	bool		m_bFadeOut;
};

ADD_SCREENSPACE_EFFECT(CScreenSpaceBlurEffect, screen_blur);

void CScreenSpaceBlurEffect::Init(void)
{
}

void CScreenSpaceBlurEffect::Shutdown(void)
{
}

void CScreenSpaceBlurEffect::SetParameters(KeyValues* params)
{
}

void CScreenSpaceBlurEffect::Enable(bool bEnable)
{
	m_flFadeStart = Plat_FloatTime();

	if (bEnable)
	{
		g_bScreenBlurEnabled = true;
		m_bFadeOut = false;
	}
	else
		m_bFadeOut = true;
}

void CScreenSpaceBlurEffect::Render(int x, int y, int w, int h)
{
	if (!IsEnabled())
		return;

	float fraction = (Plat_FloatTime() - m_flFadeStart) / mat_screen_blur_fade.GetFloat();
	fraction = clamp(fraction, 0.0f, 1.0f);
	g_fScreenBlurValue = mat_screen_blur_scale.GetFloat() * (m_bFadeOut ? (1.0f - fraction) : fraction);

	// Cancelled effect.
	if (m_bFadeOut && (fraction >= 1.0f))
	{
		g_bScreenBlurEnabled = false;
		m_bFadeOut = false;
	}
}

void SetScreenBlurState(bool value)
{
	if (value)
		g_pScreenSpaceEffects->EnableScreenSpaceEffect("screen_blur");
	else
		g_pScreenSpaceEffects->DisableScreenSpaceEffect("screen_blur");
}