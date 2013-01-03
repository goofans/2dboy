#include "WinResourceLoader.h"

#include <assert.h>
#include "Environment.h"
#include "WinImage.h"
#include "WinD3DInterface.h"
#include "WinSound.h"
#include "WinSoundPlayer.h"

using namespace Boy;

#include "BoyLib/CrtDbgNew.h"

WinResourceLoader::WinResourceLoader(const std::string &language1, 
									 const std::string &language2, 
									 WinD3DInterface *sdld3dInterface) 
									 : ResourceLoader(language1,language2)
{
	mInterface = sdld3dInterface;
}

WinResourceLoader::~WinResourceLoader()
{
}

Image *WinResourceLoader::createImage(const std::string &filename)
{
	return new WinImage(this,filename);
}

Sound *WinResourceLoader::createSound(const std::string &filename)
{
	return new WinSound(this,filename);
}

bool WinResourceLoader::load(Image *image)
{
	WinImage *img = dynamic_cast<WinImage*>(image);

	// try to find a localized version:
	D3DXIMAGE_INFO imageInfo;
	bool scaled;
	std::string fname = img->getPath()+"."+mLanguage1+".png";
	FILE *f = fopen(fname.c_str(),"rb");
	if (f==NULL)
	{
		if (mLanguage2.size()>0)
		{
			// try backup language:
			fname = img->getPath()+"."+mLanguage2+".png";
			f = fopen(fname.c_str(),"rb");
			if (f==NULL)
			{
				fname = img->getPath()+".png";
			}
			else
			{
				fclose(f);
			}
		}
		else
		{
			fname = img->getPath()+".png";
		}
	}
	else
	{
		fclose(f);
	}

	IDirect3DTexture9 *tex = mInterface->loadTexture(
		fname.c_str(),
		&imageInfo,
		&scaled);

	if (tex==NULL)
	{
		envDebugLog("could not load texture: %s.png\n",img->getPath().c_str());
	}

	// set the texture:
	if (tex!=NULL)
	{
		img->setTexture(tex, scaled);

		// remember the original image size:
		img->setSize(imageInfo.Width,imageInfo.Height);

		return true;
	}

	return false;
}

bool WinResourceLoader::load(Sound *sound)
{
	WinSound *snd = dynamic_cast<WinSound*>(sound);

	irrklang::ISoundSource *iss = tryLoad(snd->getPath()+"."+mLanguage1+".ogg"); // localized
	if (iss == NULL)
	{
		if (mLanguage2.size()>0)
		{
			iss = tryLoad(snd->getPath()+"."+mLanguage2+".ogg"); // localized
			if (iss == NULL)
			{
				iss = tryLoad(snd->getPath()+".ogg"); // default with ogg extension
			}
		}
		else
		{
			iss = tryLoad(snd->getPath()+".ogg"); // default with ogg extension
		}
	}

	if (iss == NULL)
	{
		envDebugLog("error loading sound '%s'\n",snd->getPath().c_str());
	}
//	assert(iss!=NULL);

	// store the sound source in the sound object:
	snd->setISoundSource(iss);

	if (iss!=NULL)
	{
		// make sure it's loaded:
		void *data = iss->getSampleData();
		if (data==NULL)
		{
			assert(false);
		}

		return true;
	}
	else
	{
		return false;
	}
}

irrklang::ISoundSource *WinResourceLoader::tryLoad(const std::string &filename)
{
	SoundPlayer *sp = Environment::instance()->getSoundPlayer();
	WinSoundPlayer *player = dynamic_cast<WinSoundPlayer*>(sp);
	irrklang::ISoundEngine *engine = player->getEngine();
	if (engine==NULL)
	{
		return NULL;
	}

	// if this file is already loaded:
	irrklang::ISoundSource *iss = engine->getSoundSource(filename.c_str(),false);
	if (iss!=NULL)
	{
		return iss;
	}

	// let's try to load it:
	iss = engine->addSoundSourceFromFile(
		filename.c_str(), // filename
		irrklang::ESM_NO_STREAMING, // no streaming
		false); // no preload
	iss->setForcedStreamingThreshold(-1);
	unsigned int len = iss->getPlayLength();
	if (len==0xffffffff)
	{
		engine->removeSoundSource(iss);
		return NULL;
	}

	return iss;
}
