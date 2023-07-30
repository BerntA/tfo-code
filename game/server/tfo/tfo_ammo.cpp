//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Ammo Clip Entities
//
//=============================================================================//

#include "cbase.h"
#include "gamerules.h"
#include "items.h"
#include "hl2_player.h"
#include "basecombatweapon.h"
#include "ammodef.h"
#include "npcevent.h"
#include "eventlist.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void SetGenericTextMessage(hudtextparms_t& params);

class CTFOAmmoBase : public CItem
{
public:
	DECLARE_CLASS(CTFOAmmoBase, CItem);

	CTFOAmmoBase()
	{
		m_GlowColor.Set({ 150, 100, 100, 200 });
		SetGenericTextMessage(m_textParms);
	}

	virtual void Spawn(void);
	virtual void Precache(void);
	virtual void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

protected:
	virtual int GetClipSize() { return 0; }
	virtual const char* GetAmmoType() { return ""; }
	virtual const char* GetAmmoModel() { return ""; }

private:
	hudtextparms_t m_textParms;
};

void CTFOAmmoBase::Spawn(void)
{
	Precache();
	SetModel(GetAmmoModel());
	BaseClass::Spawn();
	AddEffects(EF_NOSHADOW | EF_NORECEIVESHADOW);
}

void CTFOAmmoBase::Precache(void)
{
	PrecacheModel(GetAmmoModel());
}

void CTFOAmmoBase::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!pActivator || !pActivator->IsPlayer())
		return;

	CBasePlayer* pPlayer = ToBasePlayer(pActivator);
	if (!pPlayer)
		return;

	if (GiveAmmo(pPlayer, GetClipSize(), GetAmmoType(), true))
	{
		TransmitPickup(pPlayer); // Send item pickup notification.
		m_OnUse.FireOutput(this, this);
		UTIL_HudMessage(pPlayer, m_textParms, "#TFO_AMMO_PICKUP");
		EmitSound("Ammo.Pickup2");
		UTIL_Remove(this);
	}
	else
		EmitSound("ItemPickup.Reject");
}

//=============================================================================//
// Ammo Clip Ents
//=============================================================================//

class CMauserAmmo : public CTFOAmmoBase
{
public:
	DECLARE_CLASS(CMauserAmmo, CTFOAmmoBase);

	int GetClipSize() { return SIZE_AMMO_MAUSER; }
	const char* GetAmmoType() { return "Mauser"; }
	const char* GetAmmoModel() { return "models/items/mauser_mag.mdl"; }
};

LINK_ENTITY_TO_CLASS(mauser_ammo, CMauserAmmo);
PRECACHE_REGISTER(mauser_ammo);

class CThompsonAmmo : public CTFOAmmoBase
{
public:
	DECLARE_CLASS(CThompsonAmmo, CTFOAmmoBase);

	int GetClipSize() { return SIZE_AMMO_THOMPSON; }
	const char* GetAmmoType() { return "THOMPSON"; }
	const char* GetAmmoModel() { return "models/items/clips/thompson_mag.mdl"; }
};

LINK_ENTITY_TO_CLASS(thompson_ammo, CThompsonAmmo);
PRECACHE_REGISTER(thompson_ammo);

class CSVT40Ammo : public CTFOAmmoBase
{
public:
	DECLARE_CLASS(CSVT40Ammo, CTFOAmmoBase);

	int GetClipSize() { return SIZE_AMMO_SVT40; }
	const char* GetAmmoType() { return "SVT40"; }
	const char* GetAmmoModel() { return "models/items/clips/svt40_mag.mdl"; }
};

LINK_ENTITY_TO_CLASS(svt40_ammo, CSVT40Ammo);
PRECACHE_REGISTER(svt40_ammo);

class CStielAmmo : public CTFOAmmoBase
{
public:
	DECLARE_CLASS(CStielAmmo, CTFOAmmoBase);

	int GetClipSize() { return 1; }
	const char* GetAmmoType() { return "Grenade"; }
	const char* GetAmmoModel() { return "models/weapons/w_stick.mdl"; }
};

LINK_ENTITY_TO_CLASS(stiel_ammo, CStielAmmo);
PRECACHE_REGISTER(stiel_ammo);

class CG43Ammo : public CTFOAmmoBase
{
public:
	DECLARE_CLASS(CG43Ammo, CTFOAmmoBase);

	int GetClipSize() { return SIZE_AMMO_G43; }
	const char* GetAmmoType() { return "G43"; }
	const char* GetAmmoModel() { return "models/items/g43_mag.mdl"; }
};

LINK_ENTITY_TO_CLASS(g43_ammo, CG43Ammo);
PRECACHE_REGISTER(g43_ammo);

class CK98Ammo : public CTFOAmmoBase
{
public:
	DECLARE_CLASS(CK98Ammo, CTFOAmmoBase);

	int GetClipSize() { return SIZE_AMMO_K98; }
	const char* GetAmmoType() { return "K98"; }
	const char* GetAmmoModel() { return "models/items/w_k98_ammo.mdl"; }
};

LINK_ENTITY_TO_CLASS(k98_ammo, CK98Ammo);
PRECACHE_REGISTER(k98_ammo);

class CMP40Ammo : public CTFOAmmoBase
{
public:
	DECLARE_CLASS(CMP40Ammo, CTFOAmmoBase);

	int GetClipSize() { return SIZE_AMMO_MP40; }
	const char* GetAmmoType() { return "MP40"; }
	const char* GetAmmoModel() { return "models/items/mp_40_mag.mdl"; }
};

LINK_ENTITY_TO_CLASS(mp40_ammo, CMP40Ammo);
PRECACHE_REGISTER(mp40_ammo);

class CMP44Ammo : public CTFOAmmoBase
{
public:
	DECLARE_CLASS(CMP44Ammo, CTFOAmmoBase);

	int GetClipSize() { return SIZE_AMMO_STG44; }
	const char* GetAmmoType() { return "STG44"; }
	const char* GetAmmoModel() { return "models/items/stg44_mag.mdl"; }
};

LINK_ENTITY_TO_CLASS(mp44_ammo, CMP44Ammo);
PRECACHE_REGISTER(mp44_ammo);

class CP38Ammo : public CTFOAmmoBase
{
public:
	DECLARE_CLASS(CP38Ammo, CTFOAmmoBase);

	int GetClipSize() { return SIZE_AMMO_P38; }
	const char* GetAmmoType() { return "P38"; }
	const char* GetAmmoModel() { return "models/items/w_p38_ammo.mdl"; }
};

LINK_ENTITY_TO_CLASS(p38_ammo, CP38Ammo);
PRECACHE_REGISTER(p38_ammo);

class CPanzerAmmo : public CTFOAmmoBase
{
public:
	DECLARE_CLASS(CPanzerAmmo, CTFOAmmoBase);

	int GetClipSize() { return SIZE_AMMO_RPG_ROUND; }
	const char* GetAmmoType() { return "RPG_Round"; }
	const char* GetAmmoModel() { return "models/weapons/w_panzerschreck_rocket.mdl"; }
};

LINK_ENTITY_TO_CLASS(panzer_ammo, CPanzerAmmo);
PRECACHE_REGISTER(panzer_ammo);