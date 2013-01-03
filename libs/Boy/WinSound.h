#pragma once

#include "Sound.h"
#include "irrKlang.h"

namespace Boy
{
	class WinSound : public Sound
	{
	public:

		WinSound(ResourceLoader *loader, const std::string &path);
		virtual ~WinSound();

		const std::string &getPath() { return mPath; }

		void setISoundSource(irrklang::ISoundSource *iss);
		irrklang::ISoundSource *getISoundSource();
		void setISound(irrklang::ISound *isound);
		irrklang::ISound *getISound();

		// fade stuff:
		void fade(float volume0, float volume1, float startTime, float duration);
		float getFadeStartTime() { return mFadeStartTime; }
		float getFadeDuration() { return mFadeDuration; }
		float getFadeVolume0() { return mFadeVolume0; }
		float getFadeVolume1() { return mFadeVolume1; }

		// overrides:
		virtual void reload();

	protected:

		virtual bool init(bool includeSounds);
		virtual void destroy(bool includeSounds);

	private:

		irrklang::ISoundSource *mISoundSource;
		irrklang::ISound *mISound;

		float mFadeStartTime;
		float mFadeDuration;
		float mFadeVolume0;
		float mFadeVolume1;
	};
};
