//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Handles our render targets - for scope textures, etc...
//
//=============================================================================//

#include "cbase.h"
#include "tne_RenderTargets.h"
#include "materialsystem/imaterialsystem.h"
#include "rendertexture.h"
 
ITexture* CTNERenderTargets::CreateScopeTexture( IMaterialSystem* pMaterialSystem )
{
        return pMaterialSystem->CreateNamedRenderTargetTextureEx2(
                "dev/_rt_Scope",
                1024, 1024, RT_SIZE_OFFSCREEN,
                pMaterialSystem->GetBackBufferFormat(),
                MATERIAL_RT_DEPTH_SHARED, 
                TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
                CREATERENDERTARGETFLAGS_HDR );
}
//-----------------------------------------------------------------------------
// Purpose: Called by the engine in material system init and shutdown.
//                      Clients should override this in their inherited version, but the base
//                      is to init all standard render targets for use.
// Input  : pMaterialSystem - the engine's material system (our singleton is not yet inited at the time this is called)
//                      pHardwareConfig - the user hardware config, useful for conditional render target setup
//-----------------------------------------------------------------------------
void CTNERenderTargets::InitClientRenderTargets( IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig )
{ 
        m_ScopeTexture.Init( CreateScopeTexture( pMaterialSystem ) ); 

        // Water effects & camera from the base class (standard HL2 targets) 
        BaseClass::InitClientRenderTargets( pMaterialSystem, pHardwareConfig );
}
//-----------------------------------------------------------------------------
// Purpose: Shut down each CTextureReference we created in InitClientRenderTargets.
//                      Called by the engine in material system shutdown.
// Input  :  - 
//-----------------------------------------------------------------------------
void CTNERenderTargets::ShutdownClientRenderTargets()
{ 
        m_ScopeTexture.Shutdown();
        
        // Clean up standard HL2 RTs (camera and water) 
        BaseClass::ShutdownClientRenderTargets();
}
 
//add the interface!
static CTNERenderTargets g_TNERenderTargets;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CTNERenderTargets, IClientRenderTargets, CLIENTRENDERTARGETS_INTERFACE_VERSION, g_TNERenderTargets  );
CTNERenderTargets* TNERenderTargets = &g_TNERenderTargets;