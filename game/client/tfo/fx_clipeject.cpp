//========= Copyright © 2010-2015 Bernt Andreas Eide ============//
//
// Purpose: Drop magazine on reload. Fades out after x sec.
// TODO: The 'physics' here is terrible, it hops around in slow motion.. However it is very performance friendly.
//
//=============================================================================//

#include "cbase.h"
#include "fx.h"
#include "c_te_effect_dispatch.h"
#include "c_te_legacytempents.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClipEjectCallback( const CEffectData &data )
{
	// Use the gun angles to orient the shell
	IClientRenderable *pRenderable = data.GetRenderable();
	if ( pRenderable )
	{
		model_t *pModel = (model_t *)engine->LoadModel( C_BasePlayer::GetLocalPlayer()->GetActiveWeapon()->GetWpnData().szClipModel );
		tempents->EjectClip( data.m_vOrigin, data.m_vAngles, pRenderable->GetRenderAngles(), pModel );
	}
}

DECLARE_CLIENT_EFFECT( "ClipEject", ClipEjectCallback );