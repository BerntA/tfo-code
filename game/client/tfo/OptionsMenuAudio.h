//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Custom Audio Options
//
//=============================================================================//

#ifndef OPTIONSMENUAUDIO_H
#define OPTIONSMENUAUDIO_H

#ifdef _WIN32
#pragma once
#endif

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
	class OptionsMenuAudio;

	class OptionsMenuAudio : public vgui::Panel
	{
		DECLARE_CLASS_SIMPLE(OptionsMenuAudio, vgui::Panel);

	public:
		OptionsMenuAudio(vgui::Panel *parent, char const *panelName);
		~OptionsMenuAudio();

		void UpdateLayout();
		void ApplyAllChanges();
		void OnThink();

	private:

		vgui::GraphicalSlider *m_pSlider[2];
		vgui::GraphicalOverlay *m_pSliderGUI[2];
		vgui::Label *m_pSliderInfo[2];

		vgui::ComboBox *m_pSpeakerCombo;
		vgui::Label *m_pSpeakerInfo;

		const char *szGetName(int iIndex);
		const char *szCheckVars(int iIndex);

	protected:

		void ApplySchemeSettings(vgui::IScheme *pScheme);
		void PaintBackground();

	};
}

#endif // OPTIONSMENUAUDIO_H