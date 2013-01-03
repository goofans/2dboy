#include "Environment.h"

#include <assert.h>
#include "Environment.h"
#include "Game.h"
#include "Graphics.h"
#include "ResourceManager.h"
#include "SoundPlayer.h"
#include "WinPersistenceLayer.h"
#include "Storage.h"

#define USE_POPCAP

#if defined(GOO_PLATFORM_WIN32)
	#include "WinEnvironment.h"
#elif defined(GOO_PLATFORM_WII)
	#include "WiiEnvironment.h"
#elif defined(GOO_PLATFORM_OSX)
	#include "OSXEnvironment.h"
#elif defined(GOO_PLATFORM_LINUX)
	#include "LinuxEnvironment.h"
#endif

using namespace Boy;

#include "BoyLib/CrtDbgNew.h"

#if defined(GOO_PLATFORM_WIN32)
	static WinEnvironment sEnvObj;
#elif defined(GOO_PLATFORM_WII)
	static WiiEnvironment sEnvObj;
#elif defined(GOO_PLATFORM_OSX)
	static OSXEnvironment sEnvObj;
#elif defined(GOO_PLATFORM_LINUX)
	static LinuxEnvironment sEnvObj;
#else
	#error "unknown platform"
#endif

Environment *Environment::gInstance = &sEnvObj;

Environment::Environment()
{
}

Environment::~Environment()
{
}

Environment *Environment::instance()
{
	assert(gInstance!=NULL);
	return gInstance;
}

void Environment::playSound(const char *id, bool loop, float volume)
{
	assert(id!=NULL);
	Sound *snd = instance()->getResourceManager()->getSound(id);
	playSound(snd,loop,volume);
}

void Environment::playSound(Sound *sound, bool loop, float volume)
{
	instance()->getSoundPlayer()->playSound(sound,volume,loop);
}

void Environment::stopSound(const char *id)
{
	assert(id!=NULL);
	Sound *snd = instance()->getResourceManager()->getSound(id);
	stopSound(snd);
}

void Environment::stopSound(Sound *sound)
{
	assert(sound!=NULL);
	instance()->getSoundPlayer()->stopSound(sound);
}

Image *Environment::getImage(const std::string &id)
{
	Image *img = Environment::instance()->getResourceManager()->getImage(id);
	assert(img!=NULL);
	return img;
}

Image *Environment::getImage(const char *id, Image *defaultImg)
{
	if (id==NULL)
	{
		return NULL;
	}

	return getImage(std::string(id), defaultImg);
}

Image *Environment::getImage(const std::string &id, Image *defaultImg)
{
	Image *img = instance()->getResourceManager()->getImage(id);
	return img==NULL ? defaultImg : img;
}

int Environment::screenWidth()
{
	assert(gInstance!=NULL);
	return gInstance->getGraphics()->getWidth();
}

int Environment::screenHeight()
{
	assert(gInstance!=NULL);
	return gInstance->getGraphics()->getHeight();
}

void Environment::InitTinyXML()
{
	TiXmlSetIOHooks( TiXmlFileOpen, TiXmlFileRead, TiXmlFileSize, TiXmlFileClose );
}

void *Environment::TiXmlFileOpen( const char *pFilePathUtf8 )
{
	Storage *pStorage = gInstance->getStorage();

	BoyFileHandle hFile;
	Storage::StorageResult openResult = pStorage->FileOpen( pFilePathUtf8, Storage::STORAGE_MODE_READ | Storage::STORAGE_MUST_EXIST, &hFile );
	void *retVal = NULL;
	if( openResult == Storage::STORAGE_OK )
	{
		retVal = (void*)(long long)hFile;
	}

	return retVal;
}

int Environment::TiXmlFileRead( void *pContext, void *pBuffer, int readSizeBytes )
{
	Storage *pStorage = gInstance->getStorage();

	int readOk = 0;
	BoyFileHandle hFile = (BoyFileHandle)(long long)pContext;
	Storage::StorageResult result = pStorage->FileRead( hFile, pBuffer, readSizeBytes );
	if( result == Storage::STORAGE_OK )
	{
		readOk = 1;
	}

	return readOk;
}

int Environment::TiXmlFileSize( void *pContext )
{
	Storage *pStorage = gInstance->getStorage();

	BoyFileHandle hFile = (BoyFileHandle)(long long)pContext;
	int size = pStorage->FileGetSize( hFile );
	return size;
}

void Environment::TiXmlFileClose( void *pContext )
{
	Storage *pStorage = gInstance->getStorage();

	BoyFileHandle hFile = (BoyFileHandle)(long long)pContext;
	pStorage->FileClose( hFile );
}

void Environment::fireMouseAdded(int mouseId)
{
	if (mGame!=NULL)
	{
		mGame->handleMouseAdded(mouseId);
	}
}

void Environment::fireMouseRemoved(int mouseId)
{
	if (mGame!=NULL)
	{
		mGame->handleMouseRemoved(mouseId);
	}
}

void Environment::fireGamePadAdded(int gamePadId)
{
	if (mGame!=NULL)
	{
		mGame->handleGamePadAdded(gamePadId);
	}
}

void Environment::fireGamePadRemoved(int gamePadId)
{
	if (mGame!=NULL)
	{
		mGame->handleGamePadRemoved(gamePadId);
	}
}

void Environment::init(Boy::Game *game, 
					   int screenWidth, 
					   int screenHeight, 
					   bool fullscreen, 
					   const char *windowTitle, 
					   const UString &persFile,
					   unsigned char *persFileKey)
{
	mGame = game;
}

void Environment::destroy()
{
	mGame = NULL;
}
