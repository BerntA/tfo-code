//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Server Mode for TFO. Special accessors. 
//
//=============================================================================//

#ifndef GAMEBASE_SERVER_H
#define GAMEBASE_SERVER_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "baseentity.h"

abstract_class IGameBase_Server
{
public:

	// Achievements
	virtual void SendAchievement(const char *szAchievement) = 0;
};

extern IGameBase_Server *GameBaseServer;

#endif // GAMEBASE_SERVER_H