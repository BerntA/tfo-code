//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Custom Video Options
//
//=============================================================================//

#ifndef OPTIONSMENUVIDEO_H
#define OPTIONSMENUVIDEO_H

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
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ImagePanel.h>
#include "GraphicalOverlayInset.h"

namespace vgui
{
	class OptionsMenuVideo;

	class OptionsMenuVideo : public vgui::Panel
	{
		DECLARE_CLASS_SIMPLE(OptionsMenuVideo, vgui::Panel);

	public:
		OptionsMenuVideo(vgui::Panel *parent, char const *panelName);
		~OptionsMenuVideo();

		void UpdateLayout();
		void ApplyAllChanges();

		void OnThink();

	private:

		vgui::GraphicalSlider *m_pSlider;
		vgui::GraphicalOverlay *m_pSliderGUI;
		vgui::Label *m_pSliderInfo;

		// FOV Slider:
		vgui::GraphicalSlider *m_pFOVSlider;
		vgui::GraphicalOverlay *m_pFOVSliderGUI;
		vgui::Label *m_pFOVSliderInfo;

		vgui::ComboBox *m_pResolutionCombo[2];
		vgui::Label *m_pResolutionInfo[2];

		vgui::Button *m_pVideoScreenModBtn;
		vgui::ImagePanel *m_pVideoScreenModeImg;

		int iItems(int iIndex);
		void PrepareResolutionList();
		void SetCurrentResolutionComboItem();

		int m_nSelectedMode;
		int iCurrentAspectItem;

	protected:

		void OnCommand(const char *command);
		void ApplySchemeSettings(vgui::IScheme *pScheme);
		void PaintBackground();
	};
}

#endif // OPTIONSMENUVIDEO_H