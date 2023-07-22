//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Shows different crosshairs dynamically.
//
//=============================================================================//

#ifndef HUD_CROSSHAIRS_H
#define HUD_CROSSHAIRS_H

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

class CHudCrosshairs : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudCrosshairs, vgui::Panel);

public:

	CHudCrosshairs(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);
	void Hide(void);

protected:

	virtual void Paint();
	virtual void PaintBackground();
};

#endif // HUD_CROSSHAIRS_H