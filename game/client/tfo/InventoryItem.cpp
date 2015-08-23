//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: 3D Inventory Item Preview - Allows rotation & scrolling.
//
//=============================================================================//

#include "cbase.h"
#include "vgui/MouseCode.h"
#include "vgui/IInput.h"
#include "vgui/IScheme.h"
#include "vgui/ISurface.h"
#include <vgui/IVGui.h>
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/ScrollBar.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Button.h"
#include <vgui_controls/ImageList.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/ImagePanel.h>
#include "vgui_controls/Controls.h"
#include "InventoryItem.h"
#include "inputsystem/iinputsystem.h"
#include "vgui_controls/AnimationController.h"
#include "cdll_util.h"
#include "KeyValues.h"
#include "GameBase_Client.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

DECLARE_BUILD_FACTORY(InventoryItem);

InventoryItem::InventoryItem(vgui::Panel *parent, char const *panelName) : vgui::Panel(parent, panelName)
{
	AddActionSignalTarget(this);
	SetParent(parent);
	SetName(panelName);

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/TFOScheme.res", "TFOScheme"));

	m_pModelPreview = new CModelPanel(this, "ItemPreview");
	m_pModelPreview->SetZPos(5);

	m_pInputPanel = new MouseInputPanel(this, "InputPanel");
	m_pInputPanel->SetZPos(10);

	InvalidateLayout();

	m_bIsSelected = false;
	m_bWantsToRotate = false;

	PerformLayout();
}

InventoryItem::~InventoryItem()
{
}

void InventoryItem::OnThink()
{
	BaseClass::OnThink();

	// If we want to rotate then we'll keep track of the users movement with the mouse (coords) in order to update the position.
	if (m_bWantsToRotate)
	{
		// When the cursor goes out of our control bounds then OnMouseReleased will no longer record the mouse input, which means we need to use this function:
		if (!g_pInputSystem->IsButtonDown(MOUSE_LEFT))
		{
			m_bWantsToRotate = false;
			return;
		}

		int x, y;
		vgui::input()->GetCursorPos(x, y);

		if (x != m_iOriginalCursorXPos)
		{
			int iDiff = x - m_iOriginalCursorXPos;
			m_flAngleY += iDiff;

			// Clamp the angle between 0 - 360 degrees.
			if (m_flAngleY > 360)
				m_flAngleY = 0;
			else if (m_flAngleY < 0)
				m_flAngleY = 360;

			KeyValues *pkvChar = GameBaseClient->GetInventoryItemDetails(GameBaseClient->pszInventoryList[GetControlID()].inventoryItem);
			if (pkvChar)
			{
				KeyValues *pkvModelField = pkvChar->FindKey("ModelData");
				if (pkvModelField)
				{
					KeyValues *pkvModel = pkvModelField->FindKey("Model");
					if (pkvModel)
					{
						pkvModel->SetFloat("origin_x", m_flOriginX);
						pkvModel->SetFloat("angles_y", m_flAngleY);
						m_pModelPreview->ParseModelInfo(pkvModel);
					}
				}

				pkvChar->deleteThis();
			}

			m_iOriginalCursorXPos = x;
		}
	}
}

void InventoryItem::SetItemDetails(KeyValues *pkvData)
{
	KeyValues *pkvModelField = pkvData->FindKey("ModelData");
	if (pkvModelField)
	{
		KeyValues *pkvModel = pkvModelField->FindKey("Model");
		if (pkvModel)
		{
			m_pModelPreview->ParseModelInfo(pkvModel);
			m_flAngleY = pkvModel->GetFloat("angles_y");
			m_flOriginX = pkvModel->GetFloat("origin_x");
		}
	}

	m_pInputPanel->RequestFocus();

	m_bIsSelected = false;
	m_bWantsToRotate = false;
}

void InventoryItem::SetSize(int wide, int tall)
{
	BaseClass::SetSize(wide, tall);

	m_pInputPanel->SetPos(0, 0);
	m_pInputPanel->SetSize(wide, tall);

	m_pModelPreview->SetPos(0, 0);
	m_pModelPreview->SetSize(wide, tall);
}

void InventoryItem::PerformLayout()
{
	BaseClass::PerformLayout();
}

void InventoryItem::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void InventoryItem::OnMouseReleased(vgui::MouseCode code)
{
	if (code == MOUSE_LEFT)
		m_bWantsToRotate = false;
	else
		BaseClass::OnMouseReleased(code);
}

void InventoryItem::OnMouseDoublePressed(vgui::MouseCode code)
{
	if (code == MOUSE_LEFT)
	{
		Panel *pParent = GetParent();
		if (pParent)
		{
			char item[32];
			Q_snprintf(item, 32, "OnItem%i", GetControlID());
			pParent->OnCommand(item);
		}
	}
	else
		BaseClass::OnMouseDoublePressed(code);
}

void InventoryItem::OnMousePressed(vgui::MouseCode code)
{
	if (m_bWantsToRotate)
	{
		BaseClass::OnMousePressed(code);
		return;
	}

	int x, y;
	vgui::input()->GetCursorPos(x, y);

	if (code == MOUSE_LEFT)
	{
		m_bWantsToRotate = true;
		m_iOriginalCursorXPos = x;
	}
	else
		BaseClass::OnMousePressed(code);
}

void InventoryItem::OnMouseWheeled(int delta)
{
	bool bScrollingUp = (delta > 0);

	if (!m_bWantsToRotate)
	{
		m_flOriginX += (bScrollingUp ? -1 : 1);

		KeyValues *pkvChar = GameBaseClient->GetInventoryItemDetails(GameBaseClient->pszInventoryList[GetControlID()].inventoryItem);
		if (pkvChar)
		{
			KeyValues *pkvModelField = pkvChar->FindKey("ModelData");
			if (pkvModelField)
			{
				KeyValues *pkvModel = pkvModelField->FindKey("Model");
				if (pkvModel)
				{
					pkvModel->SetFloat("origin_x", m_flOriginX);
					pkvModel->SetFloat("angles_y", m_flAngleY);
					m_pModelPreview->ParseModelInfo(pkvModel);
				}
			}

			pkvChar->deleteThis();
		}
	}

	BaseClass::OnMouseWheeled(delta);
}

void InventoryItem::SetSelectedState(bool state)
{
	m_bIsSelected = state;
}

void InventoryItem::Paint(void)
{
	BaseClass::Paint();

	if (IsSelected())
	{
		DrawHollowBox(0, 0, GetWide(), GetTall(), Color(215, 175, 10, 255), 1.0f, 3, 3);
	}
}