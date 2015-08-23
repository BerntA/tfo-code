//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Loading Panel - Overrides the default loading panel and also hides it + reads its progress value.
//
//=============================================================================//

#include "cbase.h"
#include <vgui/VGUI.h>
#include "vgui_controls/Frame.h"
#include <vgui/ISurface.h>
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
#include <vgui_controls/Label.h>
#include "ImageProgressBar.h"

namespace vgui
{
	class Panel;
}

class CLoadingPanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CLoadingPanel, vgui::Frame);

public:
	CLoadingPanel(vgui::VPANEL parent);
	~CLoadingPanel();

	void SetRandomLoadingTip();
	void SetCustomLoadingImage(const char *szImage, bool bVisible);
	bool m_bIsLoading;
	bool m_bIsLoadingMainMenu;
	bool m_bIsMenuVisibleAndInGame;
	void SetIsLoadingMainMenu(bool bValue);

protected:
	virtual void OnThink();
	virtual void PaintBackground();
	virtual void PerformLayout();
	virtual void OnTick();
	virtual void OnScreenSizeChanged(int iOldWide, int iOldTall);
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	void SetupLayout(void);
	void SetLoadingAttributes(void);

	vgui::ImagePanel *m_pImgLoadingBackground;
	vgui::ImagePanel *m_pImgLoadingForeground;
	ImageProgressBar *m_pImgLoadingBar;
	vgui::Label *m_pTextLoadingTip;

	// Tips
	float m_flTipDisplayTime;
};