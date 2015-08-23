//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: 3D Inventory Item Preview - Allows rotation & scrolling.
//
//=============================================================================//

#ifndef INVENTORY_ITEM_H
#define INVENTORY_ITEM_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include "basemodelpanel.h"
#include <vgui_controls/RichText.h>
#include "MouseInputPanel.h"

namespace vgui
{
	class InventoryItem;

	class InventoryItem : public Panel
	{
		DECLARE_CLASS_SIMPLE(InventoryItem, Panel);

	public:

		InventoryItem(vgui::Panel *parent, char const *panelName);
		virtual ~InventoryItem();

		virtual void SetItemDetails(KeyValues *pkvData);
		virtual void SetSize(int wide, int tall);
		void SetSelectedState(bool state);
		bool IsSelected(void) { return m_bIsSelected; }

		void SetControlID(int id) { m_iControlID = id; }
		int GetControlID(void) { return m_iControlID; }

	private:

		vgui::MouseInputPanel *m_pInputPanel;
		CModelPanel *m_pModelPreview;

		float m_flAngleY;
		float m_flOriginX;
		int m_iOriginalCursorXPos;
		bool m_bWantsToRotate;
		bool m_bIsSelected;

		int m_iControlID;

	protected:

		virtual void OnThink();
		virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
		virtual void PerformLayout();

		virtual void OnMouseReleased(vgui::MouseCode code);
		virtual void OnMouseDoublePressed(vgui::MouseCode code);
		virtual void OnMousePressed(vgui::MouseCode code);
		virtual void OnMouseWheeled(int delta);
		virtual void Paint(void);
	};
}

#endif // INVENTORY_ITEM_H