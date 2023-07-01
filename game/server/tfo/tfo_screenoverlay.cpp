//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Handles Notes, parses a linked script in the resource/data/ folder and sends it as a usermessage to the client for display.
//
//=============================================================================//

#include "cbase.h"
#include "gamerules.h"
#include "baseanimating.h"
#include "items.h"
#include "hl2_player.h"
#include "basecombatweapon.h"
#include "player.h"
#include "ammodef.h"
#include "npcevent.h"
#include "eventlist.h"
#include "filesystem.h"

class CTFOScreen : public CItem
{
public:
	DECLARE_CLASS(CTFOScreen, CItem);
	DECLARE_DATADESC();

	CTFOScreen()
	{
		color32 col32 = { 255, 25, 25, 220 };
		m_GlowColor = col32;
	}

	void Spawn(void);
	void Precache(void);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

private:
	string_t szFileName;
};

LINK_ENTITY_TO_CLASS(tfo_screenoverlay, CTFOScreen);
PRECACHE_REGISTER(tfo_screenoverlay);

BEGIN_DATADESC(CTFOScreen)
DEFINE_KEYFIELD(szFileName, FIELD_STRING, "ScriptFile"),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFOScreen::Spawn(void)
{
	Precache();
	SetModel("models/props/note.mdl");
	BaseClass::Spawn();
	AddEffects(EF_NOSHADOW | EF_NORECEIVESHADOW);
}

void CTFOScreen::Precache(void)
{
	PrecacheModel("models/props/note.mdl");
}

void CTFOScreen::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) //If player used item
{
	if (!pActivator || !pActivator->IsPlayer())
		return;

	CBasePlayer* pClient = ToBasePlayer(pActivator);
	if (!pClient)
		return;

	m_OnUse.FireOutput(this, this);
	EmitSound("ItemDraw.Paper");

	// Open journal / save station.
	engine->ClientCommand(pClient->edict(), "tfo_gameui_command OpenNotePanel\n");

	// This passes in our actual texture to display and not to mention it is catched in the inventory too, so it will be added for storage there... OR we could do a direct access from the note on the client ?
	g_pGameRules->ShowNote(szFileName.ToCStr());
}