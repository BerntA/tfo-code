//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Custom Keyboard Options
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
#include "OptionsMenuKeyboard.h"
#include <vgui_controls/SectionedListPanel.h>
#include <vgui/IVGui.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/TextImage.h>
#include <vgui_controls/ImageList.h>
#include "inputsystem/iinputsystem.h"
#include "utlvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

//extern IGameUIFuncs *gameuifuncs; // for key binding details

const char *szSectionNames[] =
{
	"Movement",
	"Combat",
	"Misc",
};

const char *szCollumData[] =
{
	"binding",
	"button",
};

OptionsKeyboard::OptionsKeyboard(vgui::Panel *parent, char const *panelName) : vgui::Panel(parent, panelName)
{
	SetParent(parent);
	SetName(panelName);
	SetSize(285, 315);

	SetMouseInputEnabled(true);
	SetKeyBoardInputEnabled(true);
	SetProportional(false);

	// Set Properties:
	m_pEditPanel = vgui::SETUP_PANEL(new vgui::Button(this, "btnEdit", "Edit Me:", this));
	m_pEditPanel->SetZPos(100);
	m_pEditPanel->SetVisible(false);
	m_pEditPanel->SetText("Edit Key");
	m_pEditPanel->SetContentAlignment(Label::a_center);
	m_pEditPanel->AddActionSignalTarget(this);

	m_pMousePanel = vgui::SETUP_PANEL(new vgui::MouseInputPanel(this, "MouseInputs"));
	m_pMousePanel->SetZPos(500);
	m_pMousePanel->SetVisible(false);
	m_pMousePanel->SetEnabled(false);
	m_pMousePanel->SetSize(285, 315);
	m_pMousePanel->SetPos(0, 0);
	m_pMousePanel->AddActionSignalTarget(this);

	m_pKeyBoardList = vgui::SETUP_PANEL(new vgui::SectionedListPanel(this, "KeyboardList"));
	m_pKeyBoardList->SetSize(285, 315);
	m_pKeyBoardList->SetBgColor(Color(0, 0, 0, 0));
	m_pKeyBoardList->SetBorder(NULL);
	m_pKeyBoardList->AddActionSignalTarget(this);

	m_pKeyBoardList->SetFgColor(Color(122, 73, 57, 255));

	for (int i = 0; i <= 2; i++)
	{
		m_pKeyBoardList->AddSection(i, "");
		m_pKeyBoardList->SetSectionAlwaysVisible(i);
		m_pKeyBoardList->SetSectionFgColor(i, Color(122, 73, 57, 255));

		if (i == 0)
		{
			m_pKeyBoardList->AddColumnToSection(i, szCollumData[0], szSectionNames[0], 0, 125);
			m_pKeyBoardList->AddColumnToSection(i, szCollumData[1], "Key/Button", 0, 125);
		}
		else if (i == 1)
		{
			m_pKeyBoardList->AddColumnToSection(i, szCollumData[0], szSectionNames[1], 0, 125);
			m_pKeyBoardList->AddColumnToSection(i, szCollumData[1], "Key/Button", 0, 125);
		}
		else
		{
			m_pKeyBoardList->AddColumnToSection(i, szCollumData[0], szSectionNames[2], 0, 125);
			m_pKeyBoardList->AddColumnToSection(i, szCollumData[1], "Key/Button", 0, 125);
		}
	}

	FillKeyboardList();

	SetScheme("TFOKeyboard");

	if (m_pKeyBoardList->GetScrollbar())
	{
		m_pKeyBoardList->GetScrollbar()->SetScheme("TFOKeyboard");
		m_pKeyBoardList->GetScrollbar()->InvalidateLayout(false, true);
	}

	InvalidateLayout();

	bInEditMode = false;
	iCurrentModifiedSelectedID = -1;
}

// On deletion of player class / vgui.
OptionsKeyboard::~OptionsKeyboard()
{
}

void OptionsKeyboard::OnThink()
{
	if (bInEditMode)
	{
		// Force this ID until we go out of edit mode!
		m_pKeyBoardList->SetSelectedItem(iCurrentModifiedSelectedID);

		// Force Mouse Panel to be infront! If it ain't then we can't record mouse pressing!
		m_pMousePanel->SetVisible(true);
		m_pMousePanel->SetEnabled(true);
		m_pMousePanel->RequestFocus();
		m_pMousePanel->MoveToFront();

		ButtonCode_t code = BUTTON_CODE_INVALID;
		if (engine->CheckDoneKeyTrapping(code))
		{
			bInEditMode = false;
			m_pKeyBoardList->LeaveEditMode();
			m_pKeyBoardList->ClearSelection();
			UpdateKeyboardListData(code);
		}
	}
	else
	{
		m_pMousePanel->SetVisible(false);
		m_pMousePanel->SetEnabled(false);
	}
}

// Load all available key binds from a script file in the data folder! 
void OptionsKeyboard::FillKeyboardList(void)
{
	// RemoveItem
	m_pKeyBoardList->DeleteAllItems(); // Reset...

	// Create a new keyvalue, load our keyboard button def file. THIS IS THE NEW KB_ACT.LST FILE ( SEE MOD/SCRIPTS FOR EXAMPLE OF OLD STUFF ).
	// NEW KEYS GOES IN KeyboardButtons.TXT!
	KeyValues *kvGetData = new KeyValues("KeyboardData");
	if (kvGetData->LoadFromFile(filesystem, "data/settings/KeyboardButtons.txt", "MOD"))
	{
		KeyValues *pkvCombat = kvGetData->FindKey("Combat");
		KeyValues *pkvMovement = kvGetData->FindKey("Movement");
		KeyValues *pkvMisc = kvGetData->FindKey("Misc");

		for (KeyValues *sub = pkvCombat->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		{
			char szBinding[256];

			const char *szFindBinding = engine->Key_LookupBinding(sub->GetName());

			if (szFindBinding)
				Q_strncpy(szBinding, szFindBinding, 256);
			else
				Q_strncpy(szBinding, "NONE", 256);

			// Force UPPERCASE!
			if (Q_strcmp(szBinding, "SEMICOLON") == 0)
			{
				Q_strncpy(szBinding, ";", 256);
			}
			else if (strlen(szBinding) == 1 && szBinding[0] >= 'a' && szBinding[0] <= 'z')
			{
				szBinding[0] += ('A' - 'a');
			}

			KeyValues *pkvDataItem = new KeyValues("KeyboardButton");
			pkvDataItem->SetString(szCollumData[0], sub->GetString());
			pkvDataItem->SetString(szCollumData[1], szBinding);
			m_pKeyBoardList->AddItem(1, pkvDataItem);
			pkvDataItem->deleteThis();

			// GetActualKeyFromButtonCode( gameuifuncs->GetButtonCodeForBind( sub->GetName() ) )
		}

		for (KeyValues *sub = pkvMovement->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		{
			char szBinding[256];

			const char *szFindBinding = engine->Key_LookupBinding(sub->GetName());

			if (szFindBinding)
				Q_strncpy(szBinding, szFindBinding, 256);
			else
				Q_strncpy(szBinding, "NONE", 256);

			// Force UPPERCASE!
			if (Q_strcmp(szBinding, "SEMICOLON") == 0)
			{
				Q_strncpy(szBinding, ";", 256);
			}
			else if (strlen(szBinding) == 1 && szBinding[0] >= 'a' && szBinding[0] <= 'z')
			{
				szBinding[0] += ('A' - 'a');
			}

			KeyValues *pkvDataItem = new KeyValues("KeyboardButton");
			pkvDataItem->SetString(szCollumData[0], sub->GetString());
			pkvDataItem->SetString(szCollumData[1], szBinding);
			m_pKeyBoardList->AddItem(0, pkvDataItem);
			pkvDataItem->deleteThis();
		}

		for (KeyValues *sub = pkvMisc->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		{
			char szBinding[256];

			const char *szFindBinding = engine->Key_LookupBinding(sub->GetName());

			if (szFindBinding)
				Q_strncpy(szBinding, szFindBinding, 256);
			else
				Q_strncpy(szBinding, "NONE", 256);

			// Force UPPERCASE!
			if (Q_strcmp(szBinding, "SEMICOLON") == 0)
			{
				Q_strncpy(szBinding, ";", 256);
			}
			else if (strlen(szBinding) == 1 && szBinding[0] >= 'a' && szBinding[0] <= 'z')
			{
				szBinding[0] += ('A' - 'a');
			}

			KeyValues *pkvDataItem = new KeyValues("KeyboardButton");
			pkvDataItem->SetString(szCollumData[0], sub->GetString());
			pkvDataItem->SetString(szCollumData[1], szBinding);
			m_pKeyBoardList->AddItem(2, pkvDataItem);
			pkvDataItem->deleteThis();
		}
	}

	kvGetData->deleteThis();

	bInEditMode = false;
	iCurrentModifiedSelectedID = -1;

	m_bShouldUpdate = false;

	m_pMousePanel->SetVisible(false);
	m_pMousePanel->SetEnabled(false);

	m_pKeyBoardList->SetBorder(NULL);
}

void OptionsKeyboard::UpdateKeyboardListData(ButtonCode_t code)
{
	if (code != KEY_ESCAPE)
	{
		char szCodeToBind[256];
		Q_strncpy(szCodeToBind, g_pInputSystem->ButtonCodeToString(code), 256);

		char szOldKey[256];
		char szNewKey[256];

		engine->ClientCmd(VarArgs("unbind %s\n", szCodeToBind));

		KeyValues *pkvKeyData = m_pKeyBoardList->GetItemData(iCurrentModifiedSelectedID);
		pkvKeyData = pkvKeyData->FindKey("binding");
		if (pkvKeyData)
			Q_strncpy(szOldKey, pkvKeyData->GetString(), 256);

		// We load the keyboard file to find the binding related to the text in szOldKey.
		KeyValues *kvGetData = new KeyValues("KeyboardData");
		if (kvGetData->LoadFromFile(filesystem, "data/settings/KeyboardButtons.txt", "MOD"))
		{
			KeyValues *pkvCombat = kvGetData->FindKey("Combat");
			KeyValues *pkvMovement = kvGetData->FindKey("Movement");
			KeyValues *pkvMisc = kvGetData->FindKey("Misc");

			for (KeyValues *sub = pkvCombat->GetFirstSubKey(); sub; sub = sub->GetNextKey())
			{
				if (!strcmp(sub->GetString(), szOldKey))
				{
					engine->ClientCmd(VarArgs("unbind %s\n", engine->Key_LookupBinding(sub->GetName())));
					Q_strncpy(szNewKey, sub->GetName(), 256);
					break;
				}
			}

			for (KeyValues *sub = pkvMovement->GetFirstSubKey(); sub; sub = sub->GetNextKey())
			{
				if (!strcmp(sub->GetString(), szOldKey))
				{
					engine->ClientCmd(VarArgs("unbind %s\n", engine->Key_LookupBinding(sub->GetName())));
					Q_strncpy(szNewKey, sub->GetName(), 256);
					break;
				}
			}

			for (KeyValues *sub = pkvMisc->GetFirstSubKey(); sub; sub = sub->GetNextKey())
			{
				if (!strcmp(sub->GetString(), szOldKey))
				{
					engine->ClientCmd(VarArgs("unbind %s\n", engine->Key_LookupBinding(sub->GetName())));
					Q_strncpy(szNewKey, sub->GetName(), 256);
					break;
				}
			}
		}
		kvGetData->deleteThis();

		byte szNewKey_len = strlen(szNewKey);
		if (szNewKey_len > 0)
		{
			const char *szBinding = VarArgs("bind %s \"%s\"\n", szCodeToBind, szNewKey);
			engine->ClientCmd_Unrestricted(szBinding);
		}

		engine->ClientCmd("host_writeconfig\n"); // update config.cfg

		m_bShouldUpdate = true;
		vgui::ivgui()->AddTickSignal(GetVPanel(), 100); // There has been some issues uploading the keyboard list (getting the updatet keybinds) to we have to wait a few milli sec before we can fetch the updated binds and update the keyboard menu.
		//FillKeyboardList(); // ReDraw with new settings/bindings! Do multiple times, it's being slow..
	}
	else
	{
		iCurrentModifiedSelectedID = -1;
	}
}

void OptionsKeyboard::OnKeyCodeTyped(KeyCode code)
{
	if (bInEditMode)
		return;

	if (code == KEY_ENTER)
	{
		if (!bInEditMode && IsVisible() && (m_pKeyBoardList->GetSelectedItem() != -1))
		{
			iCurrentModifiedSelectedID = m_pKeyBoardList->GetSelectedItem();
			bInEditMode = true;
			m_pKeyBoardList->EnterEditMode(iCurrentModifiedSelectedID, 1, m_pEditPanel);
			engine->StartKeyTrapMode();
		}
	}
	else
		BaseClass::OnKeyCodeTyped(code);
}

void OptionsKeyboard::PaintBackground()
{
	m_pKeyBoardList->SetBorder(NULL);
	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBackgroundType(0);
	BaseClass::PaintBackground();
}

void OptionsKeyboard::OnTick()
{
	if (m_bShouldUpdate)
	{
		FillKeyboardList();
		vgui::ivgui()->RemoveTickSignal(GetVPanel());
	}

	BaseClass::OnTick();
}

void OptionsKeyboard::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	//BaseClass::ApplySchemeSettings(pScheme);

	m_pKeyBoardList->SetBorder(NULL);

	for (int i = 0; i <= m_pKeyBoardList->GetHighestItemID(); i++)
	{
		m_pKeyBoardList->SetItemFgColor(i, Color(122, 73, 57, 255));
		m_pKeyBoardList->SetItemFont(i, pScheme->GetFont("TFOInventorySmall"));
	}

	for (int i = 0; i <= 2; i++)
	{
		m_pKeyBoardList->SetFontSection(i, pScheme->GetFont("TFOInventorySmall"));
	}

	m_pEditPanel->SetFgColor(Color(122, 73, 57, 255));
	m_pEditPanel->SetBgColor(Color(255, 155, 0, 255));
	m_pEditPanel->SetFont(pScheme->GetFont("TFOInventorySmall"));

	m_pKeyBoardList->SetFgColor(Color(122, 73, 57, 255));
}