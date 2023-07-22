//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Allows hacking focus so we get mouse inputs & key inputs to work for certain panels. (controls)
// We basically redirect all the inputs we get here to the parent. (make sure this control is in front of everything else) SetZPos + MoveToFront() & RequestFocus() = gold.
//
//=============================================================================//

#include "cbase.h"
#include <stdio.h>
#include "filesystem.h"
#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui/MouseCode.h>
#include "MouseInputPanel.h"
#include <vgui_controls/Button.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/TextImage.h>
#include <vgui_controls/ImageList.h>
#include "IGameUIFuncs.h"
#include "utlvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

MouseInputPanel::MouseInputPanel(vgui::Panel *parent, char const *panelName) : vgui::Panel(parent, panelName)
{
	AddActionSignalTarget(this);
	SetParent(parent);
	SetName(panelName);

	SetSize(285, 315);
	SetZPos(600);

	SetMouseInputEnabled(true);
	SetKeyBoardInputEnabled(true);
	SetProportional(false);

	SetScheme("TFOScheme");

	InvalidateLayout();
}

// On deletion of player class / vgui.
MouseInputPanel::~MouseInputPanel()
{
}

// Pass mouse inputs to the parent: Then check baseclass in the parent, etc...
void MouseInputPanel::OnMousePressed(MouseCode code)
{
	vgui::Panel *myParent = this->GetParent();
	if (myParent)
		myParent->OnMousePressed(code);
	else
		Warning("MouseInputPanel with no parent is trying to redirect event press to parent!\n");
}

// Pass key presses to the parent:
void MouseInputPanel::OnKeyCodePressed(KeyCode code)
{
	vgui::Panel *myParent = this->GetParent();
	if (myParent)
		myParent->OnKeyCodeTyped(code);
	else
		Warning("MouseInputPanel with no parent is trying to redirect event press to parent!\n");
}

// Pass mouse releases to the parent:
void MouseInputPanel::OnMouseReleased(MouseCode code)
{
	vgui::Panel *myParent = this->GetParent();
	if (myParent)
		myParent->OnMouseReleased(code);
	else
		Warning("MouseInputPanel with no parent is trying to redirect event press to parent!\n");
}

// Pass double mouse clicks to the parent:
void MouseInputPanel::OnMouseDoublePressed(MouseCode code)
{
	vgui::Panel *myParent = this->GetParent();
	if (myParent)
		myParent->OnMouseDoublePressed(code);
	else
		Warning("MouseInputPanel with no parent is trying to redirect event press to parent!\n");
}

void MouseInputPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
}

// Make sure that the panel is transparent...
void MouseInputPanel::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBackgroundType(0);
	BaseClass::PaintBackground();
}