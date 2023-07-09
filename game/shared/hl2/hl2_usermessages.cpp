//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "usermessages.h"
#include "shake.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void RegisterUserMessages(void)
{
	usermessages->Register("Train", 1);
	usermessages->Register("HudText", -1);
	usermessages->Register("SayText", -1);
	usermessages->Register("SayText2", -1);
	usermessages->Register("TextMsg", -1);
	usermessages->Register("HudMsg", -1);
	usermessages->Register("ResetHUD", 1);		// called every respawn
	usermessages->Register("GameTitle", 0);
	usermessages->Register("ItemPickup", -1);
	usermessages->Register("ShowMenu", -1);
	usermessages->Register("Shake", 13);
	usermessages->Register("Fade", 10);
	usermessages->Register("Rumble", 3);	// Send a rumble to a controller
	usermessages->Register("Damage", 18);		// BUG: floats are sent for coords, no variable bitfields in hud & fixed size Msg
	usermessages->Register("CloseCaption", -1); // Show a caption (by string id number)(duration in 10th of a second)
	usermessages->Register("AmmoDenied", 2);
	usermessages->Register("BossData", -1);
	usermessages->Register("AchievementData", -1);
	usermessages->Register("ChapterTitle", -1);
}