//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Graphical Overlay applied on sliders. (above)
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
#include "GraphicalOverlayInset.h"
#include <vgui_controls/Button.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/TextImage.h>
#include <vgui_controls/ImageList.h>
#include "vgui_controls/CheckButton.h"
#include <vgui_controls/Divider.h>
#include "utlvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

GraphicalOverlay::GraphicalOverlay(vgui::Panel *parent, char const *panelName) : vgui::Panel(parent, panelName)
{
	SetParent(parent);
	SetName(panelName);

	m_pImgBG = new vgui::ImagePanel(this, "BG");
	m_pImgCircle = new vgui::ImagePanel(this, "Circle");

	m_pImgCircle->SetSize(32, 32);
	m_pImgCircle->SetShouldScaleImage(true);
	m_pImgCircle->SetImage("options/sliderknob");
	m_pImgCircle->SetZPos(15);

	m_pImgBG->SetShouldScaleImage(true);
	m_pImgBG->SetPos(0, 0);
	m_pImgBG->SetImage("options/sliderbg");
	m_pImgBG->SetZPos(5);

	m_pFG = new vgui::Divider(this, "Divider");
	m_pFG->SetZPos(10);
	m_pFG->SetBorder(NULL);
	m_pFG->SetPaintBorderEnabled(false);
}

// On deletion of player class / vgui.
GraphicalOverlay::~GraphicalOverlay()
{
}

// Position stuff to follow the actual slider which this overlay is overlayed to.
void GraphicalOverlay::PositionOverlay(int w, int h, int x, int y)
{
	m_pImgBG->SetSize(w - 5, 8);
	m_pImgBG->SetPos(0, 8);

	int iNew = (y - x);
	m_pImgCircle->SetPos(x + (iNew / 2) - 8, -4);
	Repaint();

	m_pFG->SetPos(0, 8);
	m_pFG->SetSize(x, 8);
}

// Make sure panel is transparent...
void GraphicalOverlay::PaintBackground()
{
	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBackgroundType(0);
	BaseClass::PaintBackground();
}

void GraphicalOverlay::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	SetFgColor(Color(0, 0, 0, 0));

	m_pFG->SetFgColor(Color(255, 0, 0, 255));
	m_pFG->SetBgColor(Color(255, 0, 0, 75));
	m_pFG->SetAlpha(75);
}