//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Fades down and up two dividers to form a 'letterbox' (no dividers going vertically) then it fades in the chapter title. Use ChapterTitle usermsg to toggle this hud.
//
//=============================================================================//

#ifndef HUD_LETTERBOX_H
#define HUD_LETTERBOX_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h" 
#include "hud.h" 
#include "hud_macros.h" 
#include "c_baseplayer.h"
#include "c_basehlplayer.h"
#include "iclientmode.h" 
#include "vgui/ISurface.h"
#include "usermessages.h"
#include "hudelement.h"
#include "vgui_controls/AnimationController.h"
#include <vgui/ILocalize.h>

class CHudLetterbox : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudLetterbox, vgui::Panel);

public:
	CHudLetterbox(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);

	void MsgFunc_ChapterTitle(bf_read &msg);

	bool IsDrawing(void) { return (m_bFadeIn || m_bFadeOut); }

protected:
	virtual void Paint();
	virtual void PaintBackground();

private:

	wchar_t wcsChapterTitle[64];
	bool m_bFadeIn;
	bool m_bFadeOut;

	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "HUD_TFO_Health");
	CPanelAnimationVar(Color, m_TextColor, "TextColor", "255 255 255 160");

	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "0", "proportional_float");
};

#endif // HUD_LETTERBOX_H