//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Credits Panel - Lists the credits. (animates)
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
#include <vgui_controls/TextImage.h>
#include "vgui_controls/Controls.h"
#include "CreditsListing.h"
#include <igameresources.h>
#include "KeyValues.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

CreditsListing::CreditsListing(vgui::Panel *parent, char const *panelName) : vgui::Panel(parent, panelName)
{
	AddActionSignalTarget(this);
	SetParent(parent);
	SetName(panelName);

	SetSize(285, 315);

	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);
	SetProportional(false);

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/TFOScheme.res", "TFOScheme"));

	const char *szText[] =
	{
		"Developers:\nBernt Andreas Eide \"Pretador\"\n-Programming\n-Scripting\n-Level Design\n-Choreography\n-Animation\n-2D Arts\n\nCubeMaster3D Team\n-Hadi and Mehdi Khosrokiani\n-Modelling, Texturing and Rigging\n\nCaliyr Vulpovar\n-2D Arts\n\nNathan Vess\n-Modelling and Texturing\n\nAdam Desmarais\n-Modelling\n\nSergey Pogonin\n-Sound Engineer\n-FX Engineer\n-Concept Artist\n\nPaul Graham\n-Composing and Ambiance Soundtracks\n\nSIX5MUSIC\n-Composing and Ambiance Soundtracks\n\nNate Del Duca and Brandon Savage\n-Composing and Mixing\n\nDreamachine (Shannon Lawlor)\n-Composing and Ambiance Soundtracks\n\nJason Sutherland\n-Sound Mastering",
		"Actors:\nChris Schenk\n-Voice Acting for Gregor and Storyteller\n\nLuca Borgonovo\n-Voice Acting for the mysterious man chapt1\n\nQANTL\n-Voice Acting for the Russian Brothers\n\nChris Marcellus\n-Voice Acting for Gruber and Hans\n\nBrandon Tran\n-Voice Acting for Grobuskna and Soldier(s)\n\nMichael Tsarouhas\n-Voice Acting for Schienzel and Soldier(s)\n\nTesters:\n-Darkamo\n-JonnyBoy0719\n-MrJak\n-Greeneyedgirl927\n-Sgt. Penguin\n-Sergey Pogonin\n-zeldazelda.b360\n-Joshikumako\n\nSpecial Thanks:\n-Jason Snell\n-Darkamo\n-JonnyBoy0719\n\nDonators:\n-LiveMChief",
	};		

	for (int i = 0; i < _ARRAYSIZE(m_pLabelCredits); i++)
	{
		m_pLabelCredits[i] = vgui::SETUP_PANEL(new vgui::Label(this, "LabelCredits", ""));
		m_pLabelCredits[i]->SetZPos(30);
		m_pLabelCredits[i]->SetVisible(true);
		m_pLabelCredits[i]->SetPaintBorderEnabled(false);
		m_pLabelCredits[i]->SetPaintEnabled(true);
		m_pLabelCredits[i]->SetText(szText[i]);

		int w, h;
		m_pLabelCredits[i]->GetContentSize(w, h);
		m_iSizeH[i] = h;
		m_pLabelCredits[i]->SetSize(285, h);
		m_pLabelCredits[i]->SetPos(2, 315 + ((i > 0) ? m_iSizeH[i - 1] : 0));
	}

	InvalidateLayout();
}

CreditsListing::~CreditsListing()
{
}

void CreditsListing::DoAnimate()
{
	int sizeH = 0;
	for (int i = (_ARRAYSIZE(m_pLabelCredits) - 1); i >= 0; i--)
	{
		sizeH += m_iSizeH[i];

		int x, y;
		m_pLabelCredits[i]->GetPos(x, y);

		if (y > -sizeH)
			y -= 1;

		if (y <= -sizeH)
			y = 315 + ((i > 0) ? m_iSizeH[i - 1] : 0);

		m_pLabelCredits[i]->SetPos(x, y);
	}
}

void CreditsListing::DoReset()
{
	for (int i = 0; i < _ARRAYSIZE(m_pLabelCredits); i++)
		m_pLabelCredits[i]->SetPos(2, 315 + ((i > 0) ? m_iSizeH[i - 1] : 0));
}

void CreditsListing::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	for (int i = 0; i < _ARRAYSIZE(m_pLabelCredits); i++)
	{
		m_pLabelCredits[i]->SetFgColor(Color(90, 40, 40, 245));
		m_pLabelCredits[i]->SetFont(pScheme->GetFont("TFOCredits"));
	}
}