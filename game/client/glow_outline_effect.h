//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Functionality to render a glowing outline around client renderable objects.
//
//===============================================================================

#ifndef GLOW_OUTLINE_EFFECT_H
#define GLOW_OUTLINE_EFFECT_H

#if defined( COMPILER_MSVC )
#pragma once
#endif

#include "utlvector.h"
#include "mathlib/vector.h"

class C_BaseEntity;
class CViewSetup;
class CMatRenderContextPtr;

class CGlowObjectManager
{
public:
	CGlowObjectManager() { }

	void AddEntity(C_BaseEntity* pEntity);
	void RemoveEntity(C_BaseEntity* pEntity);
	void Shutdown(void);
	void RenderGlowEffects(const CViewSetup* pSetup);

private:
	void RenderGlowModels(const CViewSetup* pSetup, CMatRenderContextPtr& pRenderContext);
	void ApplyEntityGlowEffects(const CViewSetup* pSetup, CMatRenderContextPtr& pRenderContext, float flBloomScale, int x, int y, int w, int h);
};

extern CGlowObjectManager g_GlowObjectManager;

#endif // GLOW_OUTLINE_EFFECT_H