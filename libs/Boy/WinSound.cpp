#include "WinSound.h"

#include <assert.h>
#include "Environment.h"
#include "ResourceLoader.h"
#include "WinSoundPlayer.h"

using namespace Boy;

#include "BoyLib/CrtDbgNew.h"

WinSound::WinSound(ResourceLoader *loader, const std::string &path) : Sound(loader,path)
{
	mISoundSource = NULL;
	mISound = NULL;
	mFadeStartTime = -1;
	mFadeDuration = -1;
	mFadeVolume0 = -1;
	mFadeVolume1 = -1;
}

WinSound::~WinSound()
{
}

bool WinSound::init(bool includeSounds)
{
	if (includeSounds)
	{
		return mLoader->load(this);
	}
	else
	{
		return true;
	}
}

void WinSound::destroy(bool includeSounds)
{
	if (includeSounds)
	{
		SoundPlayer *sp = Environment::instance()->getSoundPlayer();
		if (mISound!=NULL)
		{
			sp->stopSound(this);			
		}
		if (mISoundSource!=NULL)
		{
			WinSoundPlayer *wsp = dynamic_cast<WinSoundPlayer*>(sp);
			irrklang::ISoundEngine *eng = wsp->getEngine();
			if (eng!=NULL)
			{
				eng->removeSoundSource(mISoundSource);
			}
		}
//		printf("WinSound::destroy() - setting isound to NULL (winsound=%p)\n",this);
		mISound = NULL;
		mISoundSource = NULL;
	}
}

void WinSound::reload()
{
	// do nothing, we don't want to reload sounds
}

void WinSound::setISoundSource(irrklang::ISoundSource *iss)
{
	assert(mISoundSource==NULL);
	mISoundSource = iss;
}

irrklang::ISoundSource *WinSound::getISoundSource()
{
	return mISoundSource;
}

void WinSound::setISound(irrklang::ISound *isound)
{
//	printf("WinSound::setISound() - winsound=%p isound=%p\n",this,isound);
	mISound = isound;
}

irrklang::ISound *WinSound::getISound()
{
	return mISound;
}

void WinSound::fade(float volume0, float volume1, float startTime, float duration)
{
	mFadeStartTime = startTime;
	mFadeDuration = duration;
	mFadeVolume0 = volume0;
	mFadeVolume1 = volume1;
}


