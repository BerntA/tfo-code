//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Extended implementation of FMOD, allows proper fading in and out transitions between sounds.
// It also allows direct play using PlayLoadingSound to skip fading functions. Fading doesn't work when you're in the main menu and in game when not in a background map because the frametime and curtime is frozen (paused). 
// Unless there's any other way to interpolate fading, such as using the animation controller or a tick signal, I don't think it will be as smooth and at least the anim controller may freeze during pause as well.
// Notice: These fade functions work so much better in multiplayer because you can't pause the game in mp.
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "fmod.hpp"
#include "fmod_manager.h"
#include "filesystem.h"

using namespace FMOD;

static System			*pSystem = NULL;
static Sound			*pSound = NULL;
static SoundGroup		*pSoundGroup = NULL;
static Channel			*pChannel = NULL;
static ChannelGroup		*pChannelGroup = NULL;
static ConVar			*pMusicVolume = NULL;
static FMOD_RESULT		result = FMOD_OK;

CFMODManager gFMODManager;
CFMODManager* FMODManager() { return &gFMODManager; }

CFMODManager::CFMODManager()
{
	m_flVolume = 0.0f;
	m_bFadeIn = false;
	m_bFadeOut = false;
	m_bIsPlayingSound = false;
}

CFMODManager::~CFMODManager()
{
}

void CFMODManager::InitFMOD(void)
{
	result = System_Create(&pSystem); // Create the main system object.

	if (result != FMOD_OK)
		Warning("FMOD ERROR: System creation failed!\n");
	else
		DevMsg("FMOD system successfully created.\n");

	result = pSystem->init(100, FMOD_INIT_NORMAL, 0);   // Initialize FMOD system.

	if (result != FMOD_OK)
		Warning("FMOD ERROR: Failed to initialize properly!\n");
	else
		DevMsg("FMOD initialized successfully.\n");

	pMusicVolume = cvar->FindVar("snd_musicvolume");
}

void CFMODManager::ExitFMOD(void)
{
	result = pSystem->release();

	if (result != FMOD_OK)
		Warning("FMOD ERROR: System did not terminate properly!\n");
	else
		DevMsg("FMOD system terminated successfully.\n");
}

// Returns the full path of a specified sound file in the /sounds folder
const char* CFMODManager::GetFullPathToSound(const char* pathToFileFromModFolder)
{
	static char pchFullPath[MAX_PATH];

	Q_snprintf(pchFullPath, MAX_PATH, "%s/sound/%s", engine->GetGameDirectory(), pathToFileFromModFolder);
	const int len = strlen(pchFullPath);

	// convert backwards slashes to forward slashes
	for (int i = 0; i < len; i++)
	{
		if (pchFullPath[i] == '\\')
			pchFullPath[i] = '/';
	}

	return pchFullPath;
}

// Returns the current sound playing.
const char *CFMODManager::GetCurrentSoundName(void)
{
	return szActiveSound;
}

// When we're in-game and the main menu is visible the game will be paused, which means that we'll not be allowed to fade in / out sounds, we update the sound volume differently then. Override it here:
void CFMODManager::UpdateVolume(void)
{
	if ((pMusicVolume == NULL) || !engine->IsInGame() || engine->IsLevelMainMenuBackground())
		return;

	pChannel->setVolume(pMusicVolume->GetFloat());
}

// Handles all fade-related sound stuffs
// Called every frame when the client is in-game
void CFMODManager::FadeThink(void)
{
	// Fading out uses this volume as 100%...
	m_flVolume = (pMusicVolume ? pMusicVolume->GetFloat() : 1.0f);

	if (m_bFadeIn || m_bFadeOut)
	{
		float flMilli = MAX(0.0f, 1000.0f - m_flLerp);
		float flSec = flMilli * 0.001f;
		if ((flSec >= 0.7))
		{
			if (m_bFadeIn)
			{
				m_bFadeIn = false;
				m_bIsPlayingSound = true; // wait for 'final' countdown.
			}

			if (m_bFadeOut)
			{
				m_bFadeOut = false;

				Q_strncpy(szActiveSound, "", MAX_WEAPON_STRING); // clear active sound.

				// find the next sound, if we have a transit sound, prio that one.
				if (szTransitSound && szTransitSound[0])
					PlayAmbientSound(szTransitSound);
			}
		}
		else
		{
			float flFrac = SimpleSpline(flSec / 0.7);
			pChannel->setVolume((m_bFadeIn ? flFrac : (1.0f - flFrac)) * m_flVolume);
		}
	}

	// Update our volume and count down to the point where we need to fade out.
	if (m_bIsPlayingSound)
	{
		pChannel->setVolume(m_flVolume);

		float flMilli = MAX(0.0f, m_flTimeConstant - m_flSoundLength);
		float flSec = flMilli * 0.001f;
		if ((flSec >= m_flFadeOutTime))
		{
			m_bFadeOut = true;
			m_bIsPlayingSound = false;
			m_flLerp = 1000.0f;
		}
	}

	// Update our timer which we use to interpolate the fade in and out *delay* from 0 to 100% volume.
	float frame_msec = 1000.0f * gpGlobals->frametime;

	if (m_flLerp > 0)
	{
		m_flLerp -= frame_msec;
		if (m_flLerp < 0)
			m_flLerp = 0;
	}

	if (m_flSoundLength > 0)
	{
		m_flSoundLength -= frame_msec;
		if (m_flSoundLength < 0)
			m_flSoundLength = 0;
	}
}

bool CFMODManager::PlayLoadingSound(const char *szSoundPath)
{
	Q_strncpy(szActiveSound, "", MAX_WEAPON_STRING); // clear active sound.
	Q_strncpy(szTransitSound, "", MAX_WEAPON_STRING); // clear transit sound.
	m_bFadeOut = false;
	m_bFadeIn = false;
	m_bIsPlayingSound = false;

	result = pSystem->createStream(GetFullPathToSound(szSoundPath), FMOD_DEFAULT, 0, &pSound);

	if (result != FMOD_OK)
	{
		Warning("FMOD: Failed to create stream of sound '%s' ! (ERROR NUMBER: %i)\n", szSoundPath, result);
		return false;
	}

	result = pSystem->playSound(FMOD_CHANNEL_REUSE, pSound, false, &pChannel);

	if (result != FMOD_OK)
	{
		Warning("FMOD: Failed to play sound '%s' ! (ERROR NUMBER: %i)\n", szSoundPath, result);
		return false;
	}

	pChannel->setVolume(m_flVolume);

	return true;
}

// Fades in and sets all needed params for playing a sound through FMOD.
bool CFMODManager::PlayAmbientSound(const char *szSoundPath)
{
	// We don't want to play the same sound or any other before it is done!
	if (m_bFadeOut || m_bFadeIn || m_bIsPlayingSound)
		return false;

	result = pSystem->createStream(GetFullPathToSound(szSoundPath), FMOD_DEFAULT, 0, &pSound);

	if (result != FMOD_OK)
	{
		Warning("FMOD: Failed to create stream of sound '%s' ! (ERROR NUMBER: %i)\n", szSoundPath, result);
		return false;
	}

	result = pSystem->playSound(FMOD_CHANNEL_REUSE, pSound, false, &pChannel);

	if (result != FMOD_OK)
	{
		Warning("FMOD: Failed to play sound '%s' ! (ERROR NUMBER: %i)\n", szSoundPath, result);
		return false;
	}

	pChannel->setVolume(0.0); // we fade in no matter what, we will now go ahead and play a new file (.wav).

	// Get the length of the sound and set the timer to countdown.
	uint lengthOfSound;
	pSound->getLength(&lengthOfSound, FMOD_TIMEUNIT_MS);

	m_flLerp = 1000.0f; // Fade Time.
	m_flSoundLength = ((float)lengthOfSound);
	m_flTimeConstant = m_flSoundLength;
	m_flFadeOutTime = ((m_flSoundLength - 1000.0f) / 1000); // We fade out 1 sec before the sound is over.

	Q_strncpy(szActiveSound, szSoundPath, MAX_WEAPON_STRING);
	Q_strncpy(szTransitSound, "", MAX_WEAPON_STRING); // clear transit sound.

	m_bFadeIn = true;

	return true;
}

// Abruptly stops playing current ambient sound.
void CFMODManager::StopAmbientSound(bool bForceOff)
{
	if (bForceOff)
	{
		Q_strncpy(szActiveSound, "", MAX_WEAPON_STRING);
		Q_strncpy(szTransitSound, "", MAX_WEAPON_STRING); // clear transit sound.
	}

	m_bIsPlayingSound = false;
	m_bFadeIn = false;
	m_bFadeOut = true;
	m_flLerp = 1000.0f;
}

// We store a transit char which will be looked up right before the sound is fully faded out. (swapping) 
// This allow us to override a current playing song without interferring too much.
bool CFMODManager::TransitionAmbientSound(const char *szSoundPath)
{
	// If the active sound is NULL, allow us to play right away.
	if (!PlayAmbientSound(szSoundPath))
	{
		Q_strncpy(szTransitSound, szSoundPath, MAX_WEAPON_STRING); // set transit sound.
		StopAmbientSound();
		return true;
	}

	return true;
}

// We use this one when we want to spontaniously change the map through a the 'map' command. Using a point_clientcommand or point_servercommand. 
CON_COMMAND_F(fmod_sound, "Play Loading Sound", FCVAR_HIDDEN)
{
	FMODManager()->PlayLoadingSound("musics/loading_loop.wav");
}