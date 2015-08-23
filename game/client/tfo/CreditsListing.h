//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Credits Panel - Lists the credits. (animates)
//
//=============================================================================//

#ifndef CREDISTSLISTING_H
#define CREDISTSLISTING_H

#ifdef _WIN32
#pragma once
#endif

#include <UtlLinkedList.h>
#include <UtlVector.h>
#include <vgui/VGUI.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/PHandle.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/RichText.h>

namespace vgui
{
	class CreditsListing;

	class CreditsListing : public vgui::Panel
	{
		DECLARE_CLASS_SIMPLE(CreditsListing, vgui::Panel);

	public:
		CreditsListing(vgui::Panel *parent, char const *panelName);
		~CreditsListing();

		vgui::Label *m_pLabelCredits[2];
		int m_iSizeH[2];

		void DoAnimate();
		void DoReset();

	protected:

		void ApplySchemeSettings(vgui::IScheme *pScheme);

	};
}

#endif // CREDISTSLISTING_H