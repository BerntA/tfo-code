//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Custom Graphic Options
//
//=============================================================================//

#ifndef OPTIONSMENUGRAPHICS_H
#define OPTIONSMENUGRAPHICS_H

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
	class OptionsMenuGraphics;

	class OptionsMenuGraphics : public vgui::Panel
	{
		DECLARE_CLASS_SIMPLE(OptionsMenuGraphics, vgui::Panel);

	public:
		OptionsMenuGraphics(vgui::Panel *parent, char const *panelName);
		~OptionsMenuGraphics();

		void UpdateLayout();
		void ApplyAllChanges();

		void OnThink();

	private:

		vgui::GraphicalSlider *m_pSlider;
		vgui::GraphicalOverlay *m_pSliderGUI;
		vgui::Label *m_pSliderInfo;

		vgui::ComboBox *m_pGraphicsCombo[8];
		vgui::Label *m_pGraphicsComboInfo[8];

		struct AAMode_t
		{
			int m_nNumSamples;
			int m_nQualityLevel;
		};

		AAMode_t m_nAAModes[16];

		int iItems(int iIndex);

		int m_nNumAAModes;

		void SetComboItemAsRecommended(vgui::ComboBox *combo, int iItem);

		int FindMSAAMode(int nAASamples, int nAAQuality);

		void ApplyChangesToConVar(const char *pConVarName, int value);
		void MarkDefaultSettingsAsRecommended();

	protected:

		void ApplySchemeSettings(vgui::IScheme *pScheme);
		void PaintBackground();
	};
}

#endif // OPTIONSMENUGRAPHICS_H