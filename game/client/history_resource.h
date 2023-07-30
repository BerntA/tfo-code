//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Item pickup history displayed onscreen when items are picked up.
//
// $NoKeywords: $
//=============================================================================//

#ifndef HISTORY_RESOURCE_H
#define HISTORY_RESOURCE_H
#pragma once

#include "hudelement.h"
#include "ehandle.h"

#include <vgui_controls/Panel.h>

enum 
{
	HISTSLOT_EMPTY,
	HISTSLOT_WEAP,
	HISTSLOT_ITEM,
};

namespace vgui
{
	class IScheme;
}

class C_BaseCombatWeapon;

//-----------------------------------------------------------------------------
// Purpose: Used to draw the history of ammo / weapon / item pickups by the player
//-----------------------------------------------------------------------------
class CHudHistoryResource : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudHistoryResource, vgui::Panel );
private:
	struct HIST_ITEM 
	{
		HIST_ITEM() 
		{ 
			// init this here, because the code that overwrites previous history items will use this
			// to check to see if the item is empty
			DisplayTime = 0.0f; 
		}
		int type;
		float DisplayTime;  // the time at which this item should be removed from the history
		int iId;
		CHandle< C_BaseCombatWeapon > m_hWeapon;
		CHudTexture *icon;
	};

	CUtlVector<HIST_ITEM> m_PickupHistory;

public:

	CHudHistoryResource( const char *pElementName );

	// CHudElement overrides
	virtual void Init( void );
	virtual void Reset( void );
	virtual bool ShouldDraw( void );
	virtual void Paint( void );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	void	AddToHistory(int iType, int iId);
	void	AddToHistory(int iType, const char* szName);
	void	AddToHistory( C_BaseCombatWeapon *weapon );
	void	MsgFunc_ItemPickup( bf_read &msg );
	
	void	CheckClearHistory( void );
	void	AddIconToHistory(int iType, int iId, C_BaseCombatWeapon* weapon, CHudTexture* icon);

private:
	// these vars are for hl1-port compatibility
	int		m_iHistoryGap;
	int		m_iCurrentHistorySlot;
	int		m_iSizeTall;
	bool	m_bDoNotDraw;
	bool	m_bNeedsDraw;
};

#endif // HISTORY_RESOURCE_H