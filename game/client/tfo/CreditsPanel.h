#include "cbase.h"
#include <vgui/VGUI.h>
#include "vgui_controls/Frame.h"
#include <vgui/ISurface.h>
#include <vgui/IInput.h>
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
#include <vgui_controls/Label.h>
#include "CreditsListing.h"

class CCreditsPanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CCreditsPanel, vgui::Frame);

public:
	CCreditsPanel( vgui::VPANEL parent );
	~CCreditsPanel();

	void PerformDefaultLayout();
	void OnShowPanel( bool bShow );

private:

	vgui::ImagePanel *m_pImgBackground;
	vgui::CreditsListing *m_pCreditsList;

protected:

	void OnTick();
	void PerformLayout();
	void PaintBackground();
	void OnKeyCodeTyped(vgui::KeyCode code);
	void OnFinishedClose();
};