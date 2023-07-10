//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Activates full-screen film grain FX.
//
//=============================================================================//

#include "cbase.h"
#include "ScreenSpaceEffects.h"
#include "rendertexture.h"
#include "KeyValues.h"
#include "materialsystem/itexture.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterialsystem.h"
#include "view_scene.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar tfo_fx_filmgrain("tfo_fx_filmgrain", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Enable or Disable film grain.", true, 0, true, 1);
ConVar tfo_fx_filmgrain_strength("tfo_fx_filmgrain_strength", "2", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Set film grain strength.", true, 0.75f, true, 2.0f);

class CFilmGrainEffect : public IScreenSpaceEffect
{
public:
	CFilmGrainEffect(void) { }

	void Init(void);
	void Shutdown(void);
	void SetParameters(KeyValues* params);
	void Enable(bool bEnable);
	bool IsEnabled() { return tfo_fx_filmgrain.GetBool(); }
	void Render(int x, int y, int w, int h);

private:
	CMaterialReference m_FilmGrainMaterial;
};

ADD_SCREENSPACE_EFFECT(CFilmGrainEffect, filmgrain);

void CFilmGrainEffect::Init(void)
{
	m_FilmGrainMaterial.Init(materials->FindMaterial("effects/filmgrain", TEXTURE_GROUP_CLIENT_EFFECTS));
}

void CFilmGrainEffect::Shutdown(void)
{
	m_FilmGrainMaterial.Shutdown();
}

void CFilmGrainEffect::SetParameters(KeyValues* params)
{
}

void CFilmGrainEffect::Enable(bool bEnable)
{
}

void CFilmGrainEffect::Render(int x, int y, int w, int h)
{
	if (!IsEnabled())
		return;

	if (m_FilmGrainMaterial->NeedsFullFrameBufferTexture())
		DrawScreenEffectMaterial(m_FilmGrainMaterial, x, y, w, h);
	else if (m_FilmGrainMaterial->NeedsPowerOfTwoFrameBufferTexture())
	{
		UpdateRefractTexture(x, y, w, h, true);

		// Now draw the entire screen using the material...
		CMatRenderContextPtr pRenderContext(materials);
		ITexture* pTexture = GetPowerOfTwoFrameBufferTexture();
		int sw = pTexture->GetActualWidth();
		int sh = pTexture->GetActualHeight();
		// Note - don't offset by x,y - already done by the viewport.
		pRenderContext->DrawScreenSpaceRectangle(m_FilmGrainMaterial, 0, 0, w, h,
			0, 0, sw - 1, sh - 1, sw, sh);
	}
	else
	{
		byte color[4] = { 255, 255, 255, 255 };
		render->ViewDrawFade(color, m_FilmGrainMaterial);
	}
}