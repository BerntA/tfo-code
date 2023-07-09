//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "model_types.h"
#include "clienteffectprecachesystem.h"
#include "fx.h"
#include "c_te_effect_dispatch.h"
#include "beamdraw.h"

CLIENTEFFECT_REGISTER_BEGIN(PrecacheEffectCrossbow)
CLIENTEFFECT_MATERIAL("effects/muzzleflash1")
CLIENTEFFECT_REGISTER_END()

void CrosshairLoadCallback(const CEffectData& data)
{
	IClientRenderable* pRenderable = data.GetRenderable();
	if (!pRenderable)
		return;

	Vector	position;
	QAngle	angles;

	// If we found the attachment, emit sparks there
	if (pRenderable->GetAttachment(data.m_nAttachmentIndex, position, angles))
	{
		FX_ElectricSpark(position, 1.0f, 1.0f, NULL);
	}
}

DECLARE_CLIENT_EFFECT("CrossbowLoad", CrosshairLoadCallback);