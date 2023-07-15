//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Extended implementation of FMOD, allows proper fading in and out transitions between sounds.
// It also allows direct play using PlayLoadingSound to skip fading functions. Fading doesn't work when you're in the main menu and in game when not in a background map because the frametime and curtime is frozen (paused). 
// Unless there's any other way to interpolate fading, such as using the animation controller or a tick signal, I don't think it will be as smooth and at least the anim controller may freeze during pause as well.
// Notice: These fade functions work so much better in multiplayer because you can't pause the game in mp.
// 
//=============================================================================//

#ifndef FMOD_MANAGER_H
#define FMOD_MANAGER_H

#ifdef _WIN32
#pragma once
#endif

namespace FMOD
{
	class Sound;
	class Channel;
}

class CFMODManager
{
public:
	CFMODManager();
	~CFMODManager();

	void InitFMOD();
	void ExitFMOD();
	void Think();

	bool PlayAmbientSound(const char* szSoundPath);
	bool PlayLoadingSound(const char* szSoundPath);
	void StopAmbientSound(bool bForceOff = false);
	void TransitionAmbientSound(const char* szSoundPath);
	void UpdateVolume(void);

private:
	char szTransitSound[MAX_WEAPON_STRING];

	bool m_bFadeIn;
	bool m_bFadeOut;
	bool m_bIsPlayingSound;

	float m_flVolume;
	float m_flFadeTime;
	float m_flFadeOutTime;

	FMOD::Sound* m_pSound;
	FMOD::Channel* m_pChannel;
};

class CFMODAmbience
{
public:
	CFMODAmbience();
	~CFMODAmbience();

	void PlaySound(const char* pSoundPath);
	void StopSound(void);
	void SetVolume(float volume);
	void Think(void);
	void Destroy(void);

private:
	FMOD::Sound* m_pSound;
	FMOD::Channel* m_pChannel;
};

extern CFMODManager* FMODManager();

#endif //FMOD_MANAGER_H