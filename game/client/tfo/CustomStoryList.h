//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Lists available Custom Stories.
// Notice: This has been removed in Version 2.9 and up, the code is still available here as you see.
//
//=============================================================================//

#ifndef CUSTOMSTORYLIST_H
#define CUSTOMSTORYLIST_H

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
	class CustomStoryList;

	class CustomStoryList : public vgui::Panel
	{
		DECLARE_CLASS_SIMPLE(CustomStoryList, vgui::Panel);

	public:
		CustomStoryList(vgui::Panel *parent, char const *panelName);
		~CustomStoryList();

		int m_iActiveID;
		vgui::ImagePanel *m_pImageFrame;
		vgui::ImagePanel *m_pImagePreview;

		vgui::Label *m_pLabelTitle;
		vgui::RichText *m_pLabelDescription;

		vgui::ImagePanel *m_pImageArrow[2];
		vgui::Button *m_pButtonArrow[2];
		bool m_bInRolloverArrow[2];

		bool m_bInRolloverPlay;
		vgui::ImagePanel *m_pImagePlay;
		vgui::Button *m_pButtonPlay;

		// Intervals :
		void OnUpdate(void);

		void ShowStory(int iID);
		void ShowRandomStory();

		void PerformDefaultLayout();

		// Check if we're over one of the arrows or btns:
		void CheckRolloverArrows(int x, int y);
		void CheckRolloverPlay(int x, int y);

		void NextStory();
		void PrevStory();

	private:

		KeyValues *GetStoryManifestData();
		KeyValues *GetStoryData(const char *szFile);
		int GetNumCustomStories();
		bool bNoStories;

	protected:

		void OnCommand(const char* pcCommand);
		void ApplySchemeSettings(vgui::IScheme *pScheme);
		void PerformLayout();
	};
}

#endif // CUSTOMSTORYLIST_H