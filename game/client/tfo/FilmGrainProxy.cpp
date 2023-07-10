//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Handles Film Grain Strength.
//
//=============================================================================//

#include "cbase.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterialproxy.h"
#include "toolframework_client.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar tfo_fx_filmgrain;
extern ConVar tfo_fx_filmgrain_strength;

class C_FilmGrainProxy : public IMaterialProxy
{
public:
	C_FilmGrainProxy();
	virtual ~C_FilmGrainProxy();

	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues);
	C_BaseEntity* BindArgToEntity(void* pArg);
	void OnBind(void* pC_BaseEntity);
	void Release(void) { delete this; }
	IMaterial* GetMaterial(void);

private:
	IMaterialVar* blendFactor;
};

C_FilmGrainProxy::C_FilmGrainProxy()
{
	blendFactor = NULL;
}

C_FilmGrainProxy::~C_FilmGrainProxy()
{
}

bool C_FilmGrainProxy::Init(IMaterial* pMaterial, KeyValues* pKeyValues)
{
	bool found = false;
	blendFactor = pMaterial->FindVar("$basetexturetransform", &found, false);
	return found;
}

C_BaseEntity* C_FilmGrainProxy::BindArgToEntity(void* pArg)
{
	IClientRenderable* pRend = (IClientRenderable*)pArg;
	return pRend ? pRend->GetIClientUnknown()->GetBaseEntity() : NULL;
}

void C_FilmGrainProxy::OnBind(void* pC_BaseEntity)
{
	if (!tfo_fx_filmgrain.GetBool())
		return;

	const float scale = tfo_fx_filmgrain_strength.GetFloat();

	// Determine the scale of the film grain here:
	VMatrix mat(scale, 0.0f, 0.0f, 0.0f,
		0.0f, scale, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	blendFactor->SetMatrixValue(mat);

	if (ToolsEnabled())
		ToolFramework_RecordMaterialParams(GetMaterial());
}

IMaterial* C_FilmGrainProxy::GetMaterial()
{
	return blendFactor->GetOwningMaterial();
}

EXPOSE_INTERFACE(C_FilmGrainProxy, IMaterialProxy, "FilmGrain" IMATERIAL_PROXY_INTERFACE_VERSION);