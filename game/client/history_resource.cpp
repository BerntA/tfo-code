//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Item pickup history displayed onscreen when items are picked up.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "history_resource.h"
#include "hud_macros.h"
#include <vgui_controls/Controls.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include "iclientmode.h"
#include "vgui_controls/AnimationController.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

extern ConVar hud_drawhistory_time;

DECLARE_HUDELEMENT(CHudHistoryResource);
DECLARE_HUD_MESSAGE(CHudHistoryResource, ItemPickup);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHistoryResource::CHudHistoryResource(const char* pElementName) :
	CHudElement(pElementName), BaseClass(NULL, "HudHistoryResource")
{
	vgui::Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	m_bDoNotDraw = true;
	m_bNeedsDraw = false;
	m_iSizeTall = 0;
	SetHiddenBits(HIDEHUD_MISCSTATUS | HIDEHUD_INSELECTION | HIDEHUD_DIALOGUE);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHistoryResource::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(false);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHistoryResource::Init(void)
{
	HOOK_HUD_MESSAGE(CHudHistoryResource, ItemPickup);
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHistoryResource::Reset(void)
{
	m_PickupHistory.RemoveAll();
	m_iCurrentHistorySlot = 0;
	m_iSizeTall = 0;
	m_bDoNotDraw = true;
}

//-----------------------------------------------------------------------------
// Purpose: adds an element to the history
//-----------------------------------------------------------------------------
void CHudHistoryResource::AddToHistory(C_BaseCombatWeapon* weapon)
{
	// don't draw exhaustable weapons (grenades) since they'll have an ammo pickup icon as well
	if (weapon->GetWpnData().iFlags & ITEM_FLAG_EXHAUSTIBLE)
		return;

	int iId = weapon->entindex();

	// don't show the same weapon twice
	for (int i = 0; i < m_PickupHistory.Count(); i++)
	{
		if (m_PickupHistory[i].iId == iId)
		{
			// it's already in list
			return;
		}
	}

	AddIconToHistory(HISTSLOT_WEAP, iId, weapon, NULL);
}

//-----------------------------------------------------------------------------
// Purpose: Add a new entry to the pickup history
//-----------------------------------------------------------------------------
void CHudHistoryResource::AddToHistory(int iType, int iId)
{
	AddIconToHistory(iType, iId, NULL, NULL);
}

//-----------------------------------------------------------------------------
// Purpose: Add a new entry to the pickup history
//-----------------------------------------------------------------------------
void CHudHistoryResource::AddToHistory(int iType, const char* szName)
{
	if (iType != HISTSLOT_ITEM)
		return;

	// Get the item's icon
	CHudTexture* i = gHUD.GetIcon(szName);
	if (i == NULL)
		return;

	AddIconToHistory(iType, 1, NULL, i);
}

//-----------------------------------------------------------------------------
// Purpose: adds a history icon
//-----------------------------------------------------------------------------
void CHudHistoryResource::AddIconToHistory(int iType, int iId, C_BaseCombatWeapon* weapon, CHudTexture* icon)
{
	m_bNeedsDraw = true;

	// Check to see if the pic would have to be drawn too high. If so, start again from the bottom
	if ((m_iSizeTall + (icon ? scheme()->GetProportionalScaledValue(icon->rc.height) : 0)) > GetTall())	
		m_iCurrentHistorySlot = 0;	

	// If the history resource is appearing, slide the hint message element down
	if (m_iCurrentHistorySlot == 0)
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HintMessageLower");

	// ensure the size 
	m_PickupHistory.EnsureCount(m_iCurrentHistorySlot + 1);

	// default to just writing to the first slot
	HIST_ITEM* freeslot = &m_PickupHistory[m_iCurrentHistorySlot];

	freeslot->iId = iId;
	freeslot->icon = icon;
	freeslot->type = iType;
	freeslot->m_hWeapon = weapon;
	freeslot->DisplayTime = gpGlobals->curtime + hud_drawhistory_time.GetFloat();

	++m_iCurrentHistorySlot;
}

//-----------------------------------------------------------------------------
// Purpose: Handle an item pickup event from the server
//-----------------------------------------------------------------------------
void CHudHistoryResource::MsgFunc_ItemPickup(bf_read& msg)
{
	char szName[200];
	msg.ReadString(szName, sizeof(szName));
	AddToHistory(HISTSLOT_ITEM, szName);
}

//-----------------------------------------------------------------------------
// Purpose: If there aren't any items in the history, clear it out.
//-----------------------------------------------------------------------------
void CHudHistoryResource::CheckClearHistory(void)
{
	for (int i = 0; i < m_PickupHistory.Count(); i++)
	{
		if (m_PickupHistory[i].type)
			return;
	}

	m_iCurrentHistorySlot = 0;

	// Slide the hint message element back up
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HintMessageRaise");
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHudHistoryResource::ShouldDraw(void)
{
	return ((m_iCurrentHistorySlot > 0 || m_bNeedsDraw) && CHudElement::ShouldDraw());
}

//-----------------------------------------------------------------------------
// Purpose: Draw the pickup history
//-----------------------------------------------------------------------------
void CHudHistoryResource::Paint(void)
{
	if (m_bDoNotDraw)
	{
		// this is to not draw things until the first rendered
		m_bDoNotDraw = false;
		return;
	}

	// set when drawing should occur
	// will be set if valid drawing does occur
	m_bNeedsDraw = false;
	m_iSizeTall = 0;

	int wide, tall;
	GetSize(wide, tall);

	int ypos = tall;

	for (int i = 0; i < m_PickupHistory.Count(); i++)
	{
		if (m_PickupHistory[i].type)
		{
			m_PickupHistory[i].DisplayTime = MIN(m_PickupHistory[i].DisplayTime, gpGlobals->curtime + hud_drawhistory_time.GetFloat());
			if (m_PickupHistory[i].DisplayTime <= gpGlobals->curtime)
			{
				// pic drawing time has expired
				memset(&m_PickupHistory[i], 0, sizeof(HIST_ITEM));
				CheckClearHistory();
				continue;
			}

			float elapsed = m_PickupHistory[i].DisplayTime - gpGlobals->curtime;
			float scale = elapsed * 80;
			Color clr = gHUD.m_clrNormal;
			clr[3] = MIN(scale, 255);

			// get the icon and number to draw
			const CHudTexture* itemIcon = NULL;

			switch (m_PickupHistory[i].type)
			{

			case HISTSLOT_WEAP:
			{
				C_BaseCombatWeapon* pWeapon = m_PickupHistory[i].m_hWeapon;
				if (!pWeapon)
					return;

				if (!pWeapon->HasAmmo())
				{
					// if the weapon doesn't have ammo, display it as red
					clr = gHUD.m_clrCaution;
					clr[3] = MIN(scale, 255);
				}

				itemIcon = pWeapon->GetWpnData().iconWeapon;
			}
			break;
			case HISTSLOT_ITEM:
			{
				if (!m_PickupHistory[i].iId)
					continue;

				itemIcon = m_PickupHistory[i].icon;
			}
			break;
			default:
				// unknown history type
				Assert(0);
				break;

			}

			if (!itemIcon)
				continue;

			if (clr[3])
			{
				// valid drawing will occur
				m_bNeedsDraw = true;
			}

			int iconWide, iconTall;
			iconWide = scheme()->GetProportionalScaledValue(itemIcon->rc.width);
			iconTall = scheme()->GetProportionalScaledValue(itemIcon->rc.height);

			ypos -= iconTall;
			int xpos = wide - iconWide;
			int offset = scheme()->GetProportionalScaledValue(4);

			vgui::surface()->DrawSetTexture(itemIcon->textureId);
			vgui::surface()->DrawSetColor(clr);
			vgui::surface()->DrawTexturedRect(xpos, ypos, xpos + iconWide, ypos + iconTall);

			ypos -= offset;
			m_iSizeTall += (iconTall + offset);
		}
	}
}