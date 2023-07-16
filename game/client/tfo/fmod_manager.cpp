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

#define FMOD_FADE_TIME 0.5f

static System* g_pFMODSystem = NULL;
static FMOD_RESULT result = FMOD_OK;

static ConVar* pMusicVolume = NULL;
static ConVar* pMuteSoundFocus = NULL;

CFMODManager gFMODManager;
CFMODManager* FMODManager() { return &gFMODManager; }

// Returns the full path of a specified sound file in the /sounds folder
static const char* GetFullPathToSound(const char* pathToFileFromModFolder)
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

static bool IsGameInActive()
{
	return (pMuteSoundFocus && pMuteSoundFocus->GetBool() && !engine->IsActiveApp());
}

static float GetMusicVolume()
{
	return (pMusicVolume ? pMusicVolume->GetFloat() : 1.0f);
}

CFMODManager::CFMODManager()
{
	m_flVolume = m_flFadeTime = m_flFadeOutTime = 0.0f;
	m_bFadeIn = m_bFadeOut = m_bIsPlayingSound = false;

	szTransitSound[0] = 0;

	m_pSound = NULL;
	m_pChannel = NULL;
}

CFMODManager::~CFMODManager()
{
}

void CFMODManager::InitFMOD(void)
{
	result = System_Create(&g_pFMODSystem); // Create the main system object.

	if (result != FMOD_OK)
		Warning("FMOD ERROR: System creation failed!\n");
	else
		DevMsg("FMOD system successfully created.\n");

	result = g_pFMODSystem->init(10, FMOD_INIT_NORMAL, 0);   // Initialize FMOD system.

	if (result != FMOD_OK)
		Warning("FMOD ERROR: Failed to initialize properly!\n");
	else
		DevMsg("FMOD initialized successfully.\n");

	pMusicVolume = cvar->FindVar("snd_musicvolume");
	pMuteSoundFocus = cvar->FindVar("snd_mute_losefocus");
}

void CFMODManager::ExitFMOD(void)
{
	result = g_pFMODSystem->release();

	if (result != FMOD_OK)
		Warning("FMOD ERROR: System did not terminate properly!\n");
	else
		DevMsg("FMOD system terminated successfully.\n");
}

void CFMODManager::Think(void)
{
	m_flVolume = GetMusicVolume();

	if (m_pChannel == NULL)
		return;

	const float flTimeNow = engine->Time();

	bool bShouldMute = IsGameInActive();
	bool bIsMuted = false;

	m_pChannel->getMute(&bIsMuted);

	if (bIsMuted != bShouldMute)
		m_pChannel->setMute(bShouldMute);

	if (m_bFadeIn || m_bFadeOut)
	{
		float fraction = (flTimeNow - m_flFadeTime) / FMOD_FADE_TIME;
		fraction = clamp(fraction, 0.0f, 1.0f);

		m_pChannel->setVolume((m_bFadeIn ? fraction : (1.0f - fraction)) * m_flVolume);

		if (fraction >= 1.0f)
		{
			if (m_bFadeIn)
			{
				m_bFadeIn = false;
				m_bIsPlayingSound = true; // wait for 'final' countdown.
			}

			if (m_bFadeOut)
			{
				m_bFadeOut = false;

				// find the next sound, if we have a transit sound, prio that one.
				if (szTransitSound && szTransitSound[0])
					PlayAmbientSound(szTransitSound);
			}
		}
	}

	if (m_bIsPlayingSound)
	{
		m_pChannel->setVolume(m_flVolume);

		if (flTimeNow >= m_flFadeOutTime)
		{
			m_flFadeTime = flTimeNow;
			m_bFadeOut = true;
			m_bIsPlayingSound = false;
		}
	}
}

bool CFMODManager::PlayLoadingSound(const char* szSoundPath)
{
	Q_strncpy(szTransitSound, "", MAX_WEAPON_STRING); // clear transit sound.
	m_bFadeOut = false;
	m_bFadeIn = false;
	m_bIsPlayingSound = false;

	result = g_pFMODSystem->createStream(GetFullPathToSound(szSoundPath), FMOD_DEFAULT, 0, &m_pSound);

	if (result != FMOD_OK)
	{
		Warning("FMOD: Failed to create stream of sound '%s' ! (ERROR NUMBER: %i)\n", szSoundPath, result);
		return false;
	}

	result = g_pFMODSystem->playSound(FMOD_CHANNEL_REUSE, m_pSound, false, &m_pChannel);

	if (result != FMOD_OK)
	{
		Warning("FMOD: Failed to play sound '%s' ! (ERROR NUMBER: %i)\n", szSoundPath, result);
		return false;
	}

	m_pChannel->setVolume(m_flVolume);

	return true;
}

// Fades in and sets all needed params for playing a sound through FMOD.
bool CFMODManager::PlayAmbientSound(const char* szSoundPath)
{
	// We don't want to play the same sound or any other before it is done!
	if (m_bFadeOut || m_bFadeIn || m_bIsPlayingSound)
		return false;

	result = g_pFMODSystem->createStream(GetFullPathToSound(szSoundPath), FMOD_DEFAULT, 0, &m_pSound);

	if (result != FMOD_OK)
	{
		Warning("FMOD: Failed to create stream of sound '%s' ! (ERROR NUMBER: %i)\n", szSoundPath, result);
		return false;
	}

	result = g_pFMODSystem->playSound(FMOD_CHANNEL_REUSE, m_pSound, false, &m_pChannel);

	if (result != FMOD_OK)
	{
		Warning("FMOD: Failed to play sound '%s' ! (ERROR NUMBER: %i)\n", szSoundPath, result);
		return false;
	}

	m_pChannel->setVolume(0.0); // we fade in no matter what, we will now go ahead and play a new file (.wav).

	// Get the length of the sound and set the timer to countdown.
	uint lengthOfSound;
	m_pSound->getLength(&lengthOfSound, FMOD_TIMEUNIT_MS);

	m_flFadeTime = engine->Time();
	m_flFadeOutTime = m_flFadeTime + ((float)lengthOfSound) - FMOD_FADE_TIME;
	Q_strncpy(szTransitSound, "", MAX_WEAPON_STRING); // clear transit sound.

	m_bFadeIn = true;
	return true;
}

// Abruptly stops playing current ambient sound.
void CFMODManager::StopAmbientSound(bool bForceOff)
{
	if (bForceOff)
		Q_strncpy(szTransitSound, "", MAX_WEAPON_STRING); // clear transit sound.	

	m_bIsPlayingSound = false;
	m_bFadeIn = false;
	m_bFadeOut = true;
	m_flFadeTime = engine->Time();
}

// We store a transit char which will be looked up right before the sound is fully faded out. (swapping) 
// This allow us to override a current playing song without interferring too much.
void CFMODManager::TransitionAmbientSound(const char* szSoundPath)
{
	// If the active sound is NULL, allow us to play right away.
	if (!PlayAmbientSound(szSoundPath))
	{
		Q_strncpy(szTransitSound, szSoundPath, MAX_WEAPON_STRING); // set transit sound.
		StopAmbientSound();
	}
}

// When we're in-game and the main menu is visible the game will be paused, which means that we'll not be allowed to fade in / out sounds, we update the sound volume differently then. Override it here:
void CFMODManager::UpdateVolume(void)
{
	if ((pMusicVolume == NULL) || !engine->IsInGame() || engine->IsLevelMainMenuBackground() || (m_pChannel == NULL))
		return;

	m_pChannel->setVolume(pMusicVolume->GetFloat());
}

// We use this one when we want to spontaniously change the map through a the 'map' command. Using a point_clientcommand or point_servercommand. 
CON_COMMAND_F(fmod_sound, "Play Loading Sound", FCVAR_HIDDEN)
{
	FMODManager()->PlayLoadingSound("music/loading_loop.mp3");
}

CFMODAmbience::CFMODAmbience()
{
	m_pSound = NULL;
	m_pChannel = NULL;
}

CFMODAmbience::~CFMODAmbience()
{
	Destroy();
}

void CFMODAmbience::PlaySound(const char* pSoundPath)
{
	result = g_pFMODSystem->createStream(GetFullPathToSound(pSoundPath), FMOD_LOOP_NORMAL | FMOD_2D | FMOD_HARDWARE, 0, &m_pSound);

	if (result != FMOD_OK)
	{
		Warning("FMOD: Failed to create stream for sound '%s' ! (ERROR NUMBER: %i)\n", pSoundPath, result);
		return;
	}

	result = g_pFMODSystem->playSound(FMOD_CHANNEL_REUSE, m_pSound, false, &m_pChannel);

	if (result != FMOD_OK)
	{
		Warning("FMOD: Failed to play sound '%s' ! (ERROR NUMBER: %i)\n", pSoundPath, result);
		return;
	}

	m_pChannel->setVolume(0.0);
}

void CFMODAmbience::StopSound(void)
{
	SetVolume(0.0f);
}

void CFMODAmbience::SetVolume(float volume)
{
	if (m_pChannel == NULL)
		return;

	m_pChannel->setVolume(volume);
}

void CFMODAmbience::Think(void)
{
	if (m_pChannel == NULL)
		return;

	bool bShouldMute = (engine->IsPaused() || IsGameInActive());
	bool bIsMuted = false;

	m_pChannel->getMute(&bIsMuted);

	if (bIsMuted != bShouldMute)
		m_pChannel->setMute(bShouldMute);
}

void CFMODAmbience::Destroy(void)
{
	SetVolume(0.0f);

	if (m_pSound != NULL)
		m_pSound->release();

	if (m_pChannel != NULL)
		m_pChannel->stop();

	m_pSound = NULL;
	m_pChannel = NULL;
}