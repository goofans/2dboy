#include "WinSoundPlayer.h"

#include <algorithm>
#include <assert.h>
#include "Environment.h"
#include "irrKlang.h"
#include "ResourceManager.h"
#include "WinSound.h"

using namespace Boy;

#include "BoyLib/CrtDbgNew.h"

WinSoundPlayer::WinSoundPlayer()
{
	mSoundEngine = irrklang::createIrrKlangDevice();
//	assert(mSoundEngine!=NULL);
}

WinSoundPlayer::~WinSoundPlayer()
{
	if (mSoundEngine==NULL)
	{
		return;
	}
	mSoundEngine->drop();
}

void WinSoundPlayer::playSound(Sound *sound, float volume, bool loop)
{
	if (mSoundEngine==NULL)
	{
		return;
	}
	if (sound==NULL)
	{
		assert(false);
		return;
	}

//	envDebugLog("[WinSoundPlayer] playSound(%p) volume=%0.2d loop=%d\n",sound,volume,loop);
	WinSound *snd = dynamic_cast<WinSound*>(sound);
	irrklang::ISoundSource *src = snd->getISoundSource();

	if (src==NULL)
	{
		assert(false);
		return;
	}

	// play the sound:
	irrklang::ISound *iSound = mSoundEngine->play2D(
		src, // sound source
		loop, // loop sound
		true); // start paused
	assert(iSound!=NULL);

	irrklang::ISound *prev = snd->getISound();
	if (prev!=NULL && !prev->isFinished())
	{
		prev->stop();
	}
	snd->setISound(iSound);

	// set volume for this sound:
	iSound->setVolume(volume);

	// play it:
	iSound->setIsPaused(false);
}

void WinSoundPlayer::playSoundChain(std::vector<Sound*> &sounds, bool loopLastSound)
{
	// we're assuming there's more than one sound here:
	assert(sounds.size()>1);

	// create and remember the chain:
	SoundChain *chain = new SoundChain();
	chain->loopLastSound = loopLastSound;
	chain->sounds = sounds;
	mSoundChains.push_back(chain);

	// start playing the first sound:
	playSound(chain->sounds[0]);
}

void WinSoundPlayer::stopSound(Sound *sound)
{
	if (!isPlaying(sound))
	{
		return;
	}

	// stop playing it:
	WinSound *snd = dynamic_cast<WinSound*>(sound);
	irrklang::ISound *isound = snd->getISound();
	if (isound!=NULL)
	{
//		envDebugLog("##### stopping sound: winsound=%p isound=%p\n",snd,snd->getISound());
		isound->stop();
		isound->drop();
		snd->setISound(NULL);
	}
}

void WinSoundPlayer::stopAllSounds()
{
	if (mSoundEngine==NULL)
	{
		return;
	}
	mSoundEngine->stopAllSounds();
}

void WinSoundPlayer::setVolume(Sound *sound, float volume)
{
	WinSound *snd = dynamic_cast<WinSound*>(sound);
	if (snd!=NULL)
	{
		irrklang::ISound *isnd = snd->getISound();
		if (isnd!=NULL)
		{
			isnd->setVolume(volume);
		}
	}
}

void WinSoundPlayer::fadeIn(Sound *sound, float duration, bool loop)
{
	WinSound *snd = dynamic_cast<WinSound*>(sound);
	float now = Environment::instance()->getTime();
	snd->fade(0,1,now,duration);
	playSound(snd,0,loop);
	mFadingSounds.push_back(snd);
//	envDebugLog("##### fading sound IN: %x (%0.2f ms)\n",snd->getHandle(),duration*1000);
}

void WinSoundPlayer::fadeOut(Sound *sound, float duration)
{
	WinSound *snd = dynamic_cast<WinSound*>(sound);
//	envDebugLog("##### fading sound OUT: sound=%p isound=%p (%0.2f ms)\n",snd,snd->getISound(),duration*1000);
	irrklang::ISound *isnd = snd->getISound();
	if (isnd!=NULL && !isnd->isFinished())
	{
		float now = Environment::instance()->getTime();
		snd->fade(isnd->getVolume(),0,now,duration);
		mFadingSounds.push_back(snd);
	}
}

void WinSoundPlayer::tick()
{
	// update sound engine:
	if (mSoundEngine!=NULL)
	{
		mSoundEngine->update();
	}

	// iterate over all sound chains:
	int numChains = (int)mSoundChains.size();
	for (int i=numChains-1 ; i>=0 ; i--)
	{
		// get the chain:
		SoundChain *chain = mSoundChains[i];

		// if the first sound in the chain is done playing:
		if (!isPlaying(chain->sounds[0]))
		{
			// remove it:
			chain->sounds.erase(chain->sounds.begin());

			// if there are no more sounds in this chain:
			if (chain->sounds.size()==0)
			{
				// get rid of it:
				mSoundChains.erase(mSoundChains.begin()+i);
				delete chain;
			}
			else
			{
				// get the next sound:
				Sound *sound = chain->sounds[0];

				// if this is the last sound and we should loop it:
				if (chain->sounds.size()==1 && chain->loopLastSound)
				{
					// loop the sound:
					playSound(sound,1,true);

					// get rid of this sound chain:
					mSoundChains.erase(mSoundChains.begin()+i);
					delete chain;
				}
				else
				{
					// play the next sound:
					playSound(sound);
				}
			}
		}
	}

	// iterate over all fading sounds:
	int numFades = (int)mFadingSounds.size();
	for (int i=numFades-1 ; i>=0 ; i--)
	{
		WinSound *snd = mFadingSounds[i];

		float currentTime = Environment::instance()->getTime();
		float fadeStartTime = snd->getFadeStartTime();
		float fadeDuration = snd->getFadeDuration();
		float v0 = snd->getFadeVolume0();
		float v1 = snd->getFadeVolume1();
		float progress = (currentTime - fadeStartTime) / fadeDuration;
		// if we're still fading:
		if (progress<1)
		{
			// fade:
			float volume = v0 + (v1-v0) * progress;
			if (snd->getISound()!=NULL)
			{
				snd->getISound()->setVolume(volume);
			}
		}
		else
		{
			// we're done fading this sound:
			mFadingSounds.erase(mFadingSounds.begin()+i);

			// set final volume:
			if (snd->getISound()!=NULL)
			{
				snd->getISound()->setVolume(v1);
			}

			// if we're fading out:
			if (v1==0)
			{
				// stop the sound:
				stopSound(snd);
			}
		}
	}
}

bool WinSoundPlayer::isPlaying(Sound *sound)
{
	WinSound *snd = dynamic_cast<WinSound*>(sound);
	if (snd->getISound()==NULL)
	{
		return false;
	}
	return !snd->getISound()->isFinished();
}

void WinSoundPlayer::setMasterVolume(float volume)
{
	if (mSoundEngine==NULL)
	{
		return;
	}
	mSoundEngine->setSoundVolume(volume);
}

float WinSoundPlayer::getMasterVolume()
{
	if (mSoundEngine==NULL)
	{
		return 0.0f;
	}
	return mSoundEngine->getSoundVolume();
}
