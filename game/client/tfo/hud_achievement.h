//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Achievement HUD. This HUD will display whenever you receive an achievement. Check c_achievement_manager.cpp for the call to this HUD element.
//
//=============================================================================//

#ifndef HUD_ACHIEVEMENT_H
#define HUD_ACHIEVEMENT_H
#ifdef _WIN32
#pragma once
#endif

#include "hud.h" 
#include "hud_macros.h" 
#include "c_baseplayer.h"
#include "iclientmode.h" 
#include "vgui/ISurface.h"
#include "hudelement.h"
#include "vgui_controls/AnimationController.h"

class CHudAchievement : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudAchievement, vgui::Panel);

public:
	CHudAchievement(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);
	void ShowAchievement(void);

protected:
	virtual void Paint();
	virtual void PaintBackground();

	int m_nTexture_FG;

private:

	// Achievement Bools
	bool ShouldShow;
	float flShowDuration;
};

#endif // HUD_ACHIEVEMENT_H