//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Allows hacking focus so we get mouse inputs & key inputs to work for certain panels. (controls)
// We basically redirect all the inputs we get here to the parent. (make sure this control is in front of everything else) SetZPos + MoveToFront() & RequestFocus() = gold.
//
//=============================================================================//

#ifndef MOUSEINPUTPANEL_H
#define MOUSEINPUTPANEL_H

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
#include <vgui_controls/Button.h>

namespace vgui
{
	class MouseInputPanel;

	class MouseInputPanel : public vgui::Panel
	{
		DECLARE_CLASS_SIMPLE(MouseInputPanel, vgui::Panel);

	public:
		MouseInputPanel(vgui::Panel *parent, char const *panelName);
		~MouseInputPanel();

	protected:

		void ApplySchemeSettings(vgui::IScheme *pScheme);
		void OnMouseReleased(MouseCode code);
		void OnMousePressed(MouseCode code);
		void OnMouseDoublePressed(MouseCode code);
		void OnKeyCodePressed(KeyCode code);
		void PaintBackground();
	};
}

#endif // MOUSEINPUTPANEL_H