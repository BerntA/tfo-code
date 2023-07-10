//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Handles our render targets - for scope textures, etc...
//
//=============================================================================//

#ifndef TNE_RENDER_TARGETS_H
#define TNE_RENDER_TARGETS_H

#ifdef _WIN32
#pragma once
#endif

#include "baseclientrendertargets.h" // Base class, with interfaces called by engine and inherited members to init common render   targets

class IMaterialSystem;
class IMaterialSystemHardwareConfig;

class CTNERenderTargets : public CBaseClientRenderTargets
{
	DECLARE_CLASS_GAMEROOT(CTNERenderTargets, CBaseClientRenderTargets);

public:
	virtual void InitClientRenderTargets(IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig);
	virtual void ShutdownClientRenderTargets();

	ITexture* CreateScopeTexture(IMaterialSystem* pMaterialSystem);

private:
	CTextureReference               m_ScopeTexture;
};

extern CTNERenderTargets* TNERenderTargets;

#endif // TNE_RENDER_TARGETS_H