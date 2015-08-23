//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Custom Keyboard Options
//
//=============================================================================//

#ifndef OPTIONSMENUKEYBOARD_H
#define OPTIONSMENUKEYBOARD_H

#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include <utlvector.h>
#include <utllinkedlist.h>
#include <vgui/VGUI.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/PHandle.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/SectionedListPanel.h>
#include "MouseInputPanel.h"

namespace vgui
{
	class OptionsKeyboard;

	class OptionsKeyboard : public vgui::Panel
	{
		DECLARE_CLASS_SIMPLE( OptionsKeyboard, vgui::Panel );

	public:
		OptionsKeyboard( vgui::Panel *parent, char const *panelName );
		~OptionsKeyboard();

		void DoReset();
		void FillKeyboardList( void );
		void OnThink();

	private:

		vgui::SectionedListPanel *m_pKeyBoardList;
		vgui::Button *m_pEditPanel;
		vgui::MouseInputPanel *m_pMousePanel;

		bool bInEditMode;
		int iCurrentModifiedSelectedID;

		bool m_bShouldUpdate;

		void UpdateKeyboardListData( ButtonCode_t code );

	protected:

		virtual void OnTick();
		virtual void PaintBackground();
		virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
		virtual void OnKeyCodeTyped(KeyCode code);
		virtual void OnMousePressed(MouseCode code);
		virtual void OnMouseWheeled(int delta);
	};
}

#endif // OPTIONSMENUKEYBOARD_H