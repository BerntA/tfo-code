//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Lists available Custom Stories.
// Notice: This has been removed in Version 2.9 and up, the code is still available here as you see.
//
//=============================================================================//

#include "cbase.h"
#include "vgui/MouseCode.h"
#include "vgui/IInput.h"
#include "vgui/IScheme.h"
#include "vgui/ISurface.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/ScrollBar.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Button.h"
#include <vgui_controls/ImageList.h>
#include <vgui_controls/ImagePanel.h>
#include "vgui_controls/Controls.h"
#include "CustomStoryList.h"
#include "KeyValues.h"
#include "filesystem.h"
#include "GameBase_Client.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

// Add us to build mode...
DECLARE_BUILD_FACTORY(CustomStoryList);

const char *szArrowImagesNormal[] =
{
	"customstory/arrowleft",
	"customstory/arrowright",
};

int iPositionArrowX[] =
{
	0,
	221,
};

int iPositionArrowY[] =
{
	215,
	215,
};

CustomStoryList::CustomStoryList(vgui::Panel *parent, char const *panelName) : vgui::Panel(parent, panelName)
{
	AddActionSignalTarget(this);
	SetParent(parent);
	SetName(panelName);

	SetSize(285, 315);

	SetMouseInputEnabled(true);
	SetKeyBoardInputEnabled(true);
	SetProportional(false);

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/TFOScheme.res", "TFOScheme"));

	m_pImagePreview = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "ImgPreview"));
	m_pImagePreview->SetShouldScaleImage(true);
	m_pImagePreview->SetVisible(true);
	m_pImagePreview->SetZPos(10);
	m_pImagePreview->SetSize(172, 114);
	m_pImagePreview->SetPos(56, 35);

	m_pImageFrame = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "ImgFrame"));
	m_pImageFrame->SetShouldScaleImage(true);
	m_pImageFrame->SetVisible(true);
	m_pImageFrame->SetZPos(20);
	m_pImageFrame->SetSize(172, 114);
	m_pImageFrame->SetImage("customstory/frame");
	m_pImageFrame->SetPos(56, 35);

	m_pLabelTitle = vgui::SETUP_PANEL(new vgui::Label(this, "LabelTitle", ""));
	m_pLabelDescription = vgui::SETUP_PANEL(new vgui::RichText(this, "LabelDescription"));

	m_pLabelTitle->SetZPos(30);
	m_pLabelDescription->SetZPos(30);
	m_pLabelTitle->SetSize(285, 25);
	m_pLabelDescription->SetSize(172, 60);

	m_pLabelTitle->SetPos(0, 0);
	m_pLabelDescription->SetPos(56, 151);

	m_pLabelTitle->SetContentAlignment(Label::a_center);

	for (int i = 0; i <= 1; i++)
	{
		m_pButtonArrow[i] = vgui::SETUP_PANEL(new vgui::Button(this, VarArgs("ArrowButton%i", (i + 1)), ""));
		m_pImageArrow[i] = vgui::SETUP_PANEL(new vgui::ImagePanel(this, VarArgs("ArrowImage%i", (i + 1))));

		m_pImageArrow[i]->SetSize(64, 64);
		m_pImageArrow[i]->SetShouldScaleImage(true);
		m_pImageArrow[i]->SetZPos(50);

		m_pButtonArrow[i]->SetSize(64, 64);
		m_pButtonArrow[i]->SetCommand(VarArgs("Arrow%i", (i + 1)));
		m_pButtonArrow[i]->AddActionSignalTarget(this);
		m_pButtonArrow[i]->SetArmedSound("dialogue/buttonclick.wav");
		m_pButtonArrow[i]->SetReleasedSound("ui/buttonclick.wav");

		m_pButtonArrow[i]->SetPaintBorderEnabled(false);
		m_pButtonArrow[i]->SetPaintEnabled(false);
		m_pButtonArrow[i]->SetZPos(100);
		m_pImageArrow[i]->SetImage(szArrowImagesNormal[i]);

		m_pImageArrow[i]->SetPos(iPositionArrowX[i], iPositionArrowY[i]);
		m_pButtonArrow[i]->SetPos(iPositionArrowX[i], iPositionArrowY[i]);

		m_bInRolloverArrow[i] = false;
	}

	bNoStories = false;

	m_pImagePlay = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "PlayImg"));
	m_pButtonPlay = vgui::SETUP_PANEL(new vgui::Button(this, "PlayBtn", ""));

	m_pButtonPlay->SetSize(50, 50);
	m_pButtonPlay->SetPaintBorderEnabled(false);
	m_pButtonPlay->SetPaintEnabled(false);
	m_pButtonPlay->SetReleasedSound("ui/buttonclick.wav");
	m_pButtonPlay->SetZPos(40);
	m_pButtonPlay->AddActionSignalTarget(this);
	m_pButtonPlay->SetCommand("Play");
	m_pImagePlay->SetZPos(30);
	m_pImagePlay->SetImage("customstory/play");
	m_pImagePlay->SetSize(256, 64);

	m_pImagePlay->SetPos(5, 215);
	m_pButtonPlay->SetPos(123, 222);

	m_bInRolloverPlay = false;

	InvalidateLayout();

	m_iActiveID = 0;

	PerformDefaultLayout();
}

CustomStoryList::~CustomStoryList()
{
}

void CustomStoryList::OnUpdate(void)
{
	// Get mouse coords
	int x, y;
	vgui::input()->GetCursorPos(x, y);

	if (bNoStories)
	{
		m_pLabelTitle->SetText("");
		m_pLabelDescription->SetText("");
		m_pImageFrame->SetVisible(false);
		m_pLabelDescription->SetVisible(false);
		m_pImagePreview->SetImage("customstory/none");
		m_pImagePreview->SetSize(285, 315);
		m_pImagePreview->SetPos(0, 0);

		for (int i = 0; i <= 1; i++)
		{
			m_pImageArrow[i]->SetImage(szArrowImagesNormal[i]);
			m_pImageArrow[i]->SetVisible(false);
			m_pButtonArrow[i]->SetVisible(false);
			m_pButtonArrow[i]->SetEnabled(false);
			m_bInRolloverArrow[i] = false;
		}

		m_pImageFrame->SetImage("customstory/frame");
		m_pImagePlay->SetImage("customstory/play");
		m_bInRolloverPlay = false;

		m_pImagePlay->SetVisible(false);
		m_pButtonPlay->SetEnabled(false);
		m_pButtonPlay->SetVisible(false);

		return;
	}

	CheckRolloverArrows(x, y);
	CheckRolloverPlay(x, y);
}

void CustomStoryList::CheckRolloverArrows(int x, int y)
{
	for (int i = 0; i <= 1; i++)
	{
		if (m_pButtonArrow[i]->IsWithin(x, y))
		{
			if (!m_bInRolloverArrow[i])
			{
				m_pImageArrow[i]->SetImage(VarArgs("%s_over", szArrowImagesNormal[i]));
				m_bInRolloverArrow[i] = true;
			}
		}
		else
		{
			if (m_bInRolloverArrow[i])
			{
				m_pImageArrow[i]->SetImage(szArrowImagesNormal[i]);
				m_bInRolloverArrow[i] = false;
			}
		}
	}
}

void CustomStoryList::CheckRolloverPlay(int x, int y)
{
	if (m_pButtonPlay->IsWithin(x, y))
	{
		if (!m_bInRolloverPlay)
		{
			m_pImagePlay->SetImage("customstory/play_over");
			m_bInRolloverPlay = true;
		}
	}
	else
	{
		if (m_bInRolloverPlay)
		{
			m_pImagePlay->SetImage("customstory/play");
			m_bInRolloverPlay = false;
		}
	}
}

void CustomStoryList::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CustomStoryList::PerformDefaultLayout()
{
	PerformLayout();

	for (int i = 0; i <= 1; i++)
	{
		m_pImageArrow[i]->SetImage(szArrowImagesNormal[i]);
		m_pImageArrow[i]->SetVisible(true);
		m_pButtonArrow[i]->SetVisible(true);
		m_pButtonArrow[i]->SetEnabled(true);
		m_bInRolloverArrow[i] = false;
	}

	m_pImagePreview->SetSize(172, 114);
	m_pImagePreview->SetPos(56, 35);
	m_pImageFrame->SetImage("customstory/frame");
	m_pImagePlay->SetImage("customstory/play");
	m_pImageFrame->SetVisible(true);
	m_pLabelDescription->SetVisible(true);
	m_bInRolloverPlay = false;
	m_pImagePlay->SetVisible(true);
	m_pButtonPlay->SetEnabled(true);
	m_pButtonPlay->SetVisible(true);
}

// Show Story!
void CustomStoryList::ShowStory(int iID)
{
	if (GetNumCustomStories() <= -1)
		return;

	char szFile[256];
	int iTempID = -1;

	// Quick redraw...
	PerformDefaultLayout();

	KeyValues *pkvManifData = GetStoryManifestData();
	if (!pkvManifData)
		return;

	// Loop through all keys and find the one with our ID then get the string related to et...!
	for (KeyValues *sub = pkvManifData->GetFirstSubKey(); sub; sub = sub->GetNextKey())
	{
		iTempID++;
		if (iTempID == iID)
			Q_strncpy(szFile, sub->GetString(), sizeof(szFile));
	}

	pkvManifData->deleteThis();

	if (!(szFile && szFile[0]))
		return;

	KeyValues *pkvData = GetStoryData(szFile);
	if (!pkvData)
		return;

	KeyValues *pkvInfoField = pkvData->FindKey("Info");

	m_pLabelTitle->SetText(ReadAndAllocStringValue(pkvInfoField, "Title"));
	m_pLabelDescription->SetText(VarArgs("Description:\n%s", ReadAndAllocStringValue(pkvInfoField, "Description")));

	const char *szNewImg = ReadAndAllocStringValue(pkvInfoField, "ImagePreview");
	if (filesystem->FileExists(VarArgs("materials/vgui/%s.vmt", szNewImg), "MOD"))
		m_pImagePreview->SetImage(szNewImg);
	else
		m_pImagePreview->SetImage("customstory/unknown");

	pkvData->deleteThis();

	m_pLabelDescription->GotoTextStart(); // Move to top of text field... (scroll)
}

void CustomStoryList::ShowRandomStory()
{
	if (GetNumCustomStories() > -1)
	{
		bNoStories = false;
		int iStory = random->RandomInt(0, GetNumCustomStories());
		m_iActiveID = iStory;
		ShowStory(m_iActiveID);
	}
	else
		bNoStories = true;
}

KeyValues *CustomStoryList::GetStoryManifestData()
{
	KeyValues *pkvStoryData = new KeyValues("StoryData");
	if (pkvStoryData->LoadFromFile(filesystem, "data/custom_stories/stories_manifest.txt", "MOD"))
		return pkvStoryData;
	else
		Warning("Can't find the manifest file for custom stories!\n");

	pkvStoryData->deleteThis();
	return NULL;
}

KeyValues *CustomStoryList::GetStoryData(const char *szFile)
{
	KeyValues *pkvStoryData = new KeyValues("StoryData");
	if (pkvStoryData->LoadFromFile(filesystem, VarArgs("data/custom_stories/%s.txt", szFile), "MOD"))
		return pkvStoryData;
	else
		Warning("Can't find the specified custom story file %s.txt!\n", szFile);

	pkvStoryData->deleteThis();
	return NULL;
}

int CustomStoryList::GetNumCustomStories()
{
	int iVal = -1;

	KeyValues *pkvData = GetStoryManifestData();
	if (pkvData == NULL)
		return -1;

	// Loop through all keys!
	for (KeyValues *sub = pkvData->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		iVal++;

	pkvData->deleteThis();
	return iVal;
}

void CustomStoryList::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pLabelTitle->SetFgColor(Color(90, 40, 40, 245));
	m_pLabelTitle->SetFont(pScheme->GetFont("TFOInventory"));

	m_pLabelDescription->SetFgColor(Color(90, 40, 40, 245));
	m_pLabelDescription->SetFont(pScheme->GetFont("TFOInventorySmall"));
}

void CustomStoryList::NextStory()
{
	if (GetNumCustomStories() > -1)
	{
		if (m_iActiveID < GetNumCustomStories())
			m_iActiveID++;

		ShowStory(m_iActiveID);
	}
}

void CustomStoryList::PrevStory()
{
	if (GetNumCustomStories() > -1)
	{
		if (m_iActiveID > 0)
			m_iActiveID--;

		ShowStory(m_iActiveID);
	}
}

void CustomStoryList::OnCommand(const char* pcCommand)
{
	if (!Q_stricmp(pcCommand, "Play"))
	{
		if (GetNumCustomStories() <= -1)
			return;

		KeyValues *pkvData = GetStoryManifestData();
		if (pkvData == NULL)
			return;

		KeyValues *pkvCurrentStory = pkvData->FindKey(VarArgs("%i", m_iActiveID));
		if (pkvCurrentStory)
			GameBaseClient->MapLoad(pkvCurrentStory->GetString());
		else
			Warning("Couldn't find the desired map!\n");

	}

	if (!Q_stricmp(pcCommand, "Arrow1"))
	{
		PrevStory();
	}

	if (!Q_stricmp(pcCommand, "Arrow2"))
	{
		NextStory();
	}

	BaseClass::OnCommand(pcCommand);
}