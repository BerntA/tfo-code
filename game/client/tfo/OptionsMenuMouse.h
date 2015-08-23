//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Custom Mouse Options
//
//=============================================================================//

#ifndef OPTIONSMENUMOUSE_H
#define OPTIONSMENUMOUSE_H

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
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/cvartogglecheckbutton.h>
#include "GraphicalSlider.h"
#include "vgui_controls/CheckButton.h"
#include <vgui_controls/ImagePanel.h>
#include "GraphicalOverlayInset.h"

namespace vgui
{
	class OptionsMenuMouse;

	class OptionsMenuMouse : public vgui::Panel
	{
		DECLARE_CLASS_SIMPLE( OptionsMenuMouse, vgui::Panel );

	public:
		OptionsMenuMouse( vgui::Panel *parent, char const *panelName );
		~OptionsMenuMouse();

		void UpdateLayout();
		void ApplyAllChanges();

		void OnThink();

	private:
		vgui::Button *m_pMouseBtnOpts[2];
		vgui::ImagePanel *m_pMouseImgOpts[2];

		vgui::GraphicalSlider *m_pSensivity;
		vgui::GraphicalOverlay *m_pSliderOverlay;

		vgui::Label *m_pInfoSensivity;

	protected:

		void ApplySchemeSettings(vgui::IScheme *pScheme);
		void PaintBackground();
		void OnCommand(const char *command);
	};
}

#endif // OPTIONSMENUMOUSE_H