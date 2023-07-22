//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Dialogue Scene Handler. Sends its params to the client for reading. (event)
//
//=============================================================================//

#ifndef TFO_DIALOGUE_ENTITY_H
#define TFO_DIALOGUE_ENTITY_H

#ifdef _WIN32
#pragma once
#endif

#include "baseentity.h"
#include "gamerules.h"

class CDialogueManager : public CLogicalEntity
{
	DECLARE_CLASS(CDialogueManager, CLogicalEntity);
	DECLARE_DATADESC();

public:

	CDialogueManager();
	void Spawn();
	bool m_bOptionAvailable[3];

private:

	string_t szDialogueScene;
	void InputStartDialogue(inputdata_t &inputData);
	COutputEvent OnStartDialogue;
};

#endif // TFO_DIALOGUE_ENTITY_H