#pragma once

#include "BoyLib/CrtDbgInc.h"

#include <string>
#include <vector>

namespace Boy
{
	class Sound;

	class SoundPlayer
	{
	public:
		SoundPlayer() {}
		virtual ~SoundPlayer() {}

		// play/stop methods:
		virtual void playSound(Sound *sound, float volume=1, bool loop=false) = 0;
		virtual void playSoundChain(std::vector<Sound*> &soundss, bool loopLastSound=false) = 0;
		virtual void stopSound(Sound *sound) = 0;
		virtual void stopAllSounds() = 0;

		// volume control:
		virtual void setVolume(Sound *sound, float volume) = 0;
		virtual void setMasterVolume(float volume) = 0;
		virtual float getMasterVolume() = 0;

		// fading:
		virtual void fadeIn(Sound *sound, float duration, bool loop) = 0;
		virtual void fadeOut(Sound *sound, float duration) = 0;

		// ticking:
		virtual void tick() = 0;

		// misc:
		virtual bool isPlaying(Sound *sound) = 0;
	};
}

