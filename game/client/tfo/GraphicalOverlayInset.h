//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Graphical Overlay applied on sliders. (above)
//
//=============================================================================//

#ifndef GRPAHICALOVERLAYINSET_H
#define GRPAHICALOVERLAYINSET_H

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
#include "vgui_controls/CheckButton.h"
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Divider.h>

namespace vgui
{
	class GraphicalOverlay;

	class GraphicalOverlay : public vgui::Panel
	{
		DECLARE_CLASS_SIMPLE(GraphicalOverlay, vgui::Panel);

	public:
		GraphicalOverlay(vgui::Panel *parent, char const *panelName);
		~GraphicalOverlay();

		void PositionOverlay(int w, int h, int x, int y);

	private:
		vgui::ImagePanel *m_pImgBG;
		vgui::ImagePanel *m_pImgCircle;
		vgui::Divider *m_pFG;

	protected:
		void PaintBackground();
		void ApplySchemeSettings(vgui::IScheme *pScheme);
	};
}

#endif // GRPAHICALOVERLAYINSET_H