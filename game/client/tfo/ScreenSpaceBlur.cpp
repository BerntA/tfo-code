//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Activates a full screenspace motion blur (overrides motion blur in viewpostprocess, thus we do not init any material, or do any explicit rendering here!!!).
//
//=============================================================================//

#include "cbase.h"
#include "ScreenSpaceEffects.h"
#include "rendertexture.h"
#include "KeyValues.h"
#include "GameBase_Client.h"

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
		m_flFadeStart(0.0f) {}

	void Init(void);
	void Shutdown(void);
	void SetParameters(KeyValues* params);
	void Enable(bool bEnable);
	bool IsEnabled() { return true; }
	void Render(int x, int y, int w, int h);

private:
	float		m_flFadeStart;
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
}

void CScreenSpaceBlurEffect::Render(int x, int y, int w, int h)
{
	const bool bWantsToDraw = GameBaseClient->ShouldDrawBlur();
	const float flTimeNow = Plat_FloatTime();

	if (bWantsToDraw != g_bScreenBlurEnabled)
	{
		m_flFadeStart = flTimeNow;
		g_bScreenBlurEnabled = bWantsToDraw;
	}

	float fraction = (flTimeNow - m_flFadeStart) / mat_screen_blur_fade.GetFloat();
	fraction = clamp(fraction, 0.0f, 1.0f);
	g_fScreenBlurValue = mat_screen_blur_scale.GetFloat() * (g_bScreenBlurEnabled ? fraction : (1.0f - fraction));
}