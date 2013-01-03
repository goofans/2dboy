#include "WinEnvironment.h"

#include <assert.h>
#include "BoyLib/md5.h"
#include "DXUT.h"
#include <fstream>
#include "Game.h"
#include "irrKlang.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ResourceManager.h"
#include "Util.h"
#include "WinPersistenceLayer.h"
#include "WinGraphics.h"
#include "WinD3DInterface.h"
#include "WinResourceLoader.h"
#include "WinSoundPlayer.h"
#include "WinTriStrip.h"
#include "WinStorage.h"

// the higher this number is, the lower the framerate will 
// drop before the game (simulation) starts to slow down:
#define MAX_UPDATES_PER_DRAW 15

// uncomment this to get timing info for every update/draw call on the console
//#define _VERBOSE_TIMING_STATS

using namespace Boy;

#include "BoyLib/CrtDbgNew.h"

// an environment is a static object now so ctor/dtor stuff should be done in init/destroy
WinEnvironment::WinEnvironment() {}
WinEnvironment::~WinEnvironment() {}

void WinEnvironment::init(Game *game, 
						  int screenWidth, 
						  int screenHeight, 
						  bool fullscreen, 
						  const char *windowTitle, 
						  const UString &persFile,
						  unsigned char *persFileKey)
{
	// call superclass:
	Environment::init(game,screenWidth,screenHeight,fullscreen,windowTitle,persFile,persFileKey);

	// mouse initially not in bounds:
	mMouseInBounds = false;

	// init libs
	InitTinyXML();

	// encryption
	mpCryptoKey = persFileKey;

	// storage
	mStorage = new WinStorage();

	// create persistence layer:
	mPersistenceLayer = new WinPersistenceLayer(persFile,mpCryptoKey);

	// create mice:
	for (int i=0 ; i<MOUSE_COUNT_MAX ; i++)
	{
		mMice[i] = new Mouse(i);
		mMice[i]->setConnected(false);
	}
	mShowSystemMouse = true;
	mMice[0]->setConnected(true); // only mouse 0 is connected by default on windows

	// create game pads:
	for (int i=0 ; i<GAMEPAD_COUNT_MAX ; i++)
	{
		mGamePads[i] = new WinGamePad(i);
	}

	// create keyboard:
	mKeyboard = new Keyboard();
	mKeyboard->setConnected(true);

	// load config:
	loadConfig();

	// sound:
	mSoundPlayer = new WinSoundPlayer();
	mLastVolume = -1;

	// get the desired screen size:
	getDesiredScreenSize(&screenWidth,&screenHeight);

	bool windowed = !fullscreen;

	// create a the platform interface:
	int refreshRate = 0;
	std::map<std::string,std::string>::iterator rrStr = mConfig.find("refreshrate");
	if (rrStr!=mConfig.end())
	{
		refreshRate = atoi(rrStr->second.c_str());
	}
	mPlatformInterface = new WinD3DInterface(game, screenWidth, screenHeight, windowTitle, windowed, refreshRate);
	mLastKnownWindowSize.x = screenWidth;
	mLastKnownWindowSize.y = screenHeight;

	// resource loader:
	std::vector<std::string> langs;
	tokenize(mConfig["language"],", ",langs);
	if (langs.size()==0)
	{
		langs.push_back("en");
	}
	mResourceLoader = new WinResourceLoader(langs[0], 
		langs.size()>1 ? langs[1] : "", 
		mPlatformInterface);

	// resource manager:
	mResourceManager = new ResourceManager(mResourceLoader, mpCryptoKey, langs[0],
		langs.size()>1 ? langs[1] : "");

	// graphics:
	mGraphics = new WinGraphics(mPlatformInterface);

	// we don't want to shut down right away:
	mShutdownRequested = false;

	// remember the game:
	mGame = game;

	// initialize timing stuff:
	mMinStepSize = 0;
	mUpdateCount = 0;
	mLastUpdate = SDL_GetTicks();
	mT0 = SDL_GetTicks();
	mIntervalFrameCount = 0;
	mPauseCount = 0;

	// debug:
#ifdef _DEBUG
	mIsDebugEnabled = true;
#else
	mIsDebugEnabled = false;
#endif

	// fullscreen:
	mFullScreenToggleDisableCount = 0;

	// log file:
	mLogFile = NULL;

	// virtual mouse button state:
	for (int i=0 ; i<MOUSE_COUNT_MAX ; i++)
	{
		mIsLeftMouseButtonDown[i] = false;
	}
}

void WinEnvironment::destroy()
{
	Environment::destroy();

	delete mPersistenceLayer;
	for (int i=0 ; i<MOUSE_COUNT_MAX ; i++)
	{
		delete mMice[i];
		mMice[i] = NULL;
	}
	for (int i=0 ; i<GAMEPAD_COUNT_MAX ; i++)
	{
		delete mGamePads[i];
		mGamePads[i] = NULL;
	}
	delete mResourceManager;
	mResourceManager = NULL;
	delete mKeyboard;
	mKeyboard = NULL;
	delete mPlatformInterface;
	mPlatformInterface = NULL;
	delete mResourceLoader;
	mResourceLoader = NULL;
	delete mGraphics;
	mGraphics = NULL;
	delete mSoundPlayer;
	mSoundPlayer = NULL;
	delete mStorage;
	mStorage = NULL;

	mConfig.clear();
}

Graphics *WinEnvironment::getGraphics()
{
	return mGraphics;
}

ResourceManager *WinEnvironment::getResourceManager() 
{ 
	return mResourceManager;
}

PersistenceLayer *WinEnvironment::getPersistenceLayer() 
{ 
	return mPersistenceLayer; 
}

SoundPlayer *WinEnvironment::getSoundPlayer() 
{ 
	return mSoundPlayer;
}

Mouse *WinEnvironment::getFirstMouse()
{
	return mMice[0];
}

int WinEnvironment::getMouseCount()
{
	return MOUSE_COUNT_MAX;
}

Mouse *WinEnvironment::getMouse(int mouseId)
{
	assert(mouseId<MOUSE_COUNT_MAX);

	return mMice[mouseId];
}

int WinEnvironment::getGamePadCount()
{
	return GAMEPAD_COUNT_MAX;
}

GamePad *WinEnvironment::getGamePad(int i)
{
	assert(i<MOUSE_COUNT_MAX);

	return mGamePads[i];
}

void WinEnvironment::showSystemMouse(bool show)
{
	mShowSystemMouse = show;
	SDL_ShowCursor(show ? SDL_ENABLE : SDL_DISABLE);
}

int WinEnvironment::getKeyboardCount()
{
	return 1;
}

Keyboard *WinEnvironment::getKeyboard(int i)
{
	return mKeyboard;
}

int WinEnvironment::getWiimoteCount()
{
	return 0;
}

Wiimote *WinEnvironment::getWiimote(int i)
{
	assert(false);
	return NULL;
}

TriStrip *WinEnvironment::createTriStrip(int numVerts)
{
	WinTriStrip *strip = new WinTriStrip(numVerts);
	return strip;
}

SDL_semaphore *gLoadingSemaphore;

int loadingProc(void *data)
{
	Game *game = (Game*)data;

	// create a locked semaphore for synchronizing with loading thread:
	gLoadingSemaphore = SDL_CreateSemaphore(0);

	// give priority to loading:
//	::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

	// load the game:
	game->load();

	// unlock the loading semaphore:
	SDL_SemPost(gLoadingSemaphore);

	return 0;
}

void WinEnvironment::startMainLoop()
{
	// bootstrap load
	mGame->preInitLoad();

	// draw the splash screen:
	mPlatformInterface->beginScene();
	mGame->preInitDraw(mGraphics);
	mPlatformInterface->endScene();

	// initialize the game:
	mGame->init();

	// semaphore for synchronizing with loading thread:
	gLoadingSemaphore = NULL;

	// start loading thread:
	SDL_CreateThread(loadingProc, mGame);

	// timing variables:
	mT0 = SDL_GetTicks();
	mIntervalStartTime = SDL_GetTicks();

	// main loop:
	while (!mShutdownRequested)
	{
		// keep track of when this iteration of the main loop started:
		Uint32 t0 = SDL_GetTicks();

		// if the loading thread is done:
		if (gLoadingSemaphore!=NULL && SDL_SemTryWait(gLoadingSemaphore)==0)
		{
			// get rid of the semaphore:
			SDL_DestroySemaphore(gLoadingSemaphore);
			gLoadingSemaphore = NULL;

			// handle loading completion:
			mGame->loadComplete();
		}

		// handle events (mouse movement, window stuff):
		MSG msg;
		msg.message = WM_NULL;

		while(PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE )!=0)
		{
			// Translate and dispatch the message
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		// if we're not paused, update:
		if (mPauseCount==0)
		{
			update();
		}

		// let's draw:
		draw();

		// print some timing stats:
		printTimingStats();

		// if a pause ended during this iteration:
		if (mPauseDuration>0)
		{
			// account for it:
			mT0 += mPauseDuration;
			// reset the pause duration now that it's accounted for:
			mPauseDuration = 0;
		}

		// calculate number of milliseconds this frame required:
		Uint32 t = SDL_GetTicks() - t0;

		// figure out if we need to sleep before the next frame:
		int sleepTime = (int)mMinStepSize - (int)t;
		if (sleepTime>0)
		{
			// sleep until it's time to calculate the next frame:
			sleep(sleepTime);
		}
	}

	mGame->preShutdown();
}

void WinEnvironment::update()
{
	// poll the game pads:
	pollGamePads();

	// tick the sound player:
	getSoundPlayer()->tick();

	// update:
	Uint32 t = SDL_GetTicks();
	mGame->update((t-mLastUpdate)/1000.0f);
	mLastUpdate = t;
	mUpdateCount++;
	mIntervalFrameCount++;
}

void WinEnvironment::draw()
{
	// begin the scene:
	Uint32 t0 = SDL_GetTicks();
	bool canDraw = mPlatformInterface->beginScene();
	Uint32 t1 = SDL_GetTicks();

	if (!canDraw)
	{
		return;
	}

	// draw:
	int s0 = mGraphics->getTransformStackSize();
	mGame->draw(mGraphics);
	int s1 = mGraphics->getTransformStackSize();
	assert(s0 == s1);

	// end the scene:
	mPlatformInterface->endScene();
}

void WinEnvironment::printTimingStats()
{
	// if it's time to calculate framerates:
	Uint32 dt = SDL_GetTicks() - mIntervalStartTime;
	if (dt > 1000)
	{
		// calculate fps/ups:
		float fps = (float)mIntervalFrameCount * 1000.0f / dt;

		// reset counters:
		mIntervalStartTime = SDL_GetTicks();
		mIntervalFrameCount = 0;

		envDebugLog("fps=%3.0f\n",fps);
	}
}

void WinEnvironment::stopMainLoop()
{
	mShutdownRequested = true;
}

bool WinEnvironment::isShuttingDown()
{
	return mShutdownRequested;
}

void WinEnvironment::showError(const std::string &message)
{
	MessageBoxA(0,message.c_str(),"Message from World of Goo Corporation",MB_OK | MB_ICONSTOP);
}

float WinEnvironment::getTime()
{
	return (SDL_GetTicks() - mT0) / 1000.0f;
}

void WinEnvironment::pauseTime()
{
	if (mPauseCount==0)
	{
		mPauseTime = SDL_GetTicks();
	}

	mPauseCount++;
}

void WinEnvironment::resumeTime()
{
	assert(mPauseCount>0);
	mPauseCount--;

	if (mPauseCount==0)
	{
		mPauseDuration += SDL_GetTicks() - mPauseTime;
	}
}

int WinEnvironment::getMaxFrameRate()
{
	return mMaxFrameRate;
}

void WinEnvironment::setMaxFrameRate(int fps)
{
	mMaxFrameRate = fps;
	mMinStepSize = (Uint32)(1000.0f / fps);
}

void WinEnvironment::debugLog(const char *fmt, ...)
{
#if defined (_DEBUG) || defined (GOO_PLATFORM_WIN32)
	// write to console:
	printf("[t=%0.2f] ",getTime());
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);

	if (mLogFile!=NULL)
	{
		fprintf(mLogFile, "[t=%0.2f] ",getTime());
		va_list ap2;
		va_start(ap2, fmt);
		vfprintf(mLogFile, fmt, ap2);
		va_end(ap2);
		fflush(mLogFile);
	}
#endif
}

void WinEnvironment::setLogFile(FILE *f)
{
	mLogFile = f;
}

void WinEnvironment::screenshot(const char *filename)
{
	// NYI
}

void WinEnvironment::setMute(bool mute)
{
	assert(mSoundPlayer!=NULL);

	if (mute)
	{
		mLastVolume = mSoundPlayer->getMasterVolume();
		mSoundPlayer->setMasterVolume(0);
	}
	else
	{
		if (mLastVolume<=0)
		{
			mLastVolume = 1;
		}
		assert(mLastVolume>0);
		mSoundPlayer->setMasterVolume(mLastVolume);
	}
}

bool WinEnvironment::isMute()
{
	return mSoundPlayer->getMasterVolume()==0;
}

void WinEnvironment::setDebugEnabled(bool enabled)
{
	mIsDebugEnabled = enabled;
}

bool WinEnvironment::isDebugEnabled()
{
	return mIsDebugEnabled;
}

void WinEnvironment::updateVirtualMice()
{
	for (int i=0 ; i<MOUSE_COUNT_MAX ; i++)
	{
		int x = mMouseVelocity[i].x;
		int y = mMouseVelocity[i].y;
		if (x!=0 || y!=0)
		{
			Mouse *m = mMice[i];
			m->fireMoveEvent(
				m->getPosition().x + x,
				m->getPosition().y + y);
		}
	}
}

#define VIRTUAL_MOUSE_SPEED 2
bool WinEnvironment::processVirtualMouseEvents(UINT key, bool down)
{
	if (!isDebugEnabled())
	{
		return false;
	}

	if (down)
	{
		switch(key)
		{
		case VK_UP:
			mMouseVelocity[1].y = -VIRTUAL_MOUSE_SPEED;
			return true;
		case VK_DOWN:
			mMouseVelocity[1].y = VIRTUAL_MOUSE_SPEED;
			return true;
		case VK_RIGHT:
			mMouseVelocity[1].x = VIRTUAL_MOUSE_SPEED;
			return true;
		case VK_LEFT:
			mMouseVelocity[1].x = -VIRTUAL_MOUSE_SPEED;
			return true;
		case VK_OEM_2: // this is / and ? for US keyboards
			if (!mIsLeftMouseButtonDown[1])
			{
				mMice[1]->fireDownEvent(Boy::Mouse::BUTTON_LEFT,1);
				mIsLeftMouseButtonDown[1] = true;
			}
			return true;
		}
	}
	else
	{
		switch(key)
		{
		case VK_UP:
		case VK_DOWN:
			mMouseVelocity[1].y = 0;
			return true;
		case VK_RIGHT:
		case VK_LEFT:
			mMouseVelocity[1].x = 0;
			return true;
		case VK_OEM_2: // this is / and ? for US keyboards
			if (mIsLeftMouseButtonDown[1])
			{
				getMouse(1)->fireUpEvent(Boy::Mouse::BUTTON_LEFT);
				mIsLeftMouseButtonDown[1] = false;
			}
			return true;
		}
	}

	return false;
}

bool WinEnvironment::isFullScreen()
{
	return !mPlatformInterface->isWindowed();
}

void WinEnvironment::toggleFullScreen()
{
	DXUTToggleFullScreen();
	mGame->fullscreenToggled(isFullScreen());
}

void WinEnvironment::enableFullScreenToggle()
{
	assert(mFullScreenToggleDisableCount>0);
	mFullScreenToggleDisableCount--;
}

void WinEnvironment::disableFullScreenToggle()
{
	mFullScreenToggleDisableCount++;
}

void WinEnvironment::sleep(int milliseconds)
{
	SDL_Delay(milliseconds);
}

void WinEnvironment::dumpEnvironmentInfo(const char *filename)
{
	std::ofstream file;
	file.open(filename, std::ofstream::out | std::ofstream::app);

	mGraphics->dumpInfo(file);

	file.close();
}

std::string WinEnvironment::getHardwareId()
{
	char volumeName[MAX_PATH];
	DWORD serialNumber = 0; 

	::GetVolumeInformationA(
		NULL, // root path name (null uses current drive
		volumeName, // the name of the volume
		MAX_PATH, // length of above array
		&serialNumber, // volume serial number
		NULL,
		NULL,
		NULL,
		0);

	char serialString[128];
	this->sprintf(serialString,128,"%d",serialNumber);


	md5_state_t state;
	md5_byte_t hashedSerial[16];

	md5_init(&state);
	md5_append(&state, (const md5_byte_t*)serialString, (int)strlen(serialString));
	md5_finish(&state, hashedSerial);

	// convert to hex string:
	char hashedSerialString[33];
	for (int i=0 ; i<16 ; i++)
	{
		this->sprintf(hashedSerialString+(i*2  ), 2, "%x", (hashedSerial[i]>>4)&0xf); // high half byte
		this->sprintf(hashedSerialString+(i*2+1), 2, "%x", (hashedSerial[i]   )&0xf); // low half byte
	}
	hashedSerialString[32] = 0;

	return std::string((char*)hashedSerialString);
}

unsigned char *WinEnvironment::getCryptoKey()
{
	return mpCryptoKey;
}

struct WinHttpRequest
{
	HttpResponseHandler *responseHandler;
	std::string host;
	std::string resource;
};

int WinEnvironment::sprintf( char *pBuffer, int bufferLenChars, const char *pFormat, ... )
{
	va_list ap;
	va_start( ap, pFormat );
	int retVal = vsnprintf_s( pBuffer, bufferLenChars, _TRUNCATE, pFormat, ap );
	va_end( ap );

	return retVal;
}

int WinEnvironment::stricmp( const char *pStr1, const char *pStr2 )
{
	return _stricmp( pStr1, pStr2 );
}

Storage *WinEnvironment::getStorage()
{
	assert( mStorage );
	return mStorage;
}

void WinEnvironment::loadConfig()
{
	BoyFileHandle hFile;
	Storage *pStorage = Environment::instance()->getStorage();
	Storage::StorageResult result = pStorage->FileOpen( "config.txt", Storage::STORAGE_MODE_READ | Storage::STORAGE_MUST_EXIST, &hFile );
	if( result == Storage::STORAGE_OK )
	{
		int size = pStorage->FileGetSize( hFile );
		char *data = new char [size];
		result = pStorage->FileRead( hFile, data, size );
		assert( result == Storage::STORAGE_OK );
		pStorage->FileClose( hFile );

		// load the file:
		TiXmlDocument doc;
		doc.Parse(data);

		// deallocate buffer:
		delete[] data;

		// get the root element:
		TiXmlElement *root = doc.RootElement();
		assert(strcmp(root->Value(),"config")==0);

		// iterate over all values:
		for (TiXmlElement *e = root->FirstChildElement() ; e!=NULL ; e = e->NextSiblingElement())
		{
			const char *value = e->Attribute("value");
			mConfig[e->Attribute("name")] = value;
		}
	}
}

void WinEnvironment::getDesiredScreenSize(int *screenWidth, int *screenHeight)
{
	// see if we have overrides for screen size, use them:
	if (mConfig.find("screen_width")!=mConfig.end() &&
		mConfig.find("screen_height")!=mConfig.end())
	{
		int w = atoi(mConfig["screen_width"].c_str());
		int h = atoi(mConfig["screen_height"].c_str());
		*screenWidth = w;
		*screenHeight = h;
	}
}

void WinEnvironment::checkMouseInBounds()
{
	POINT p;
	WINDOWINFO info;
	if (GetCursorPos(&p) && GetWindowInfo(DXUTGetHWND(),&info))
	{
		bool mouseInBounds = 
			p.x>=info.rcClient.left && 
			p.x<=info.rcClient.right &&
			p.y>=info.rcClient.top &&
			p.y<=info.rcClient.bottom;

		if (mouseInBounds!=mMouseInBounds)
		{
			if (mouseInBounds)
			{
				mMice[0]->fireEnterEvent();			
				ShowCursor(false);
			}
			else
			{
				mMice[0]->fireLeaveEvent();			
				ShowCursor(true);
			}
			mMouseInBounds = mouseInBounds;
		}
	}
}

int WinEnvironment::getSafeZoneInset()
{
	return atoi(mConfig["ui_inset"].c_str());
}

void WinEnvironment::pollGamePads()
{
	XINPUT_STATE state;

	for (int i=0 ; i<GAMEPAD_COUNT_MAX ; i++)
	{
		// get the game pad:
		GamePad *gp = mGamePads[i];

		// get the state:
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		DWORD result = XInputGetState(i, &state);

		// update connection state:
		if (result==ERROR_SUCCESS)
		{
//			envDebugLog("wButtons[%d] = 0x%0x\n",i,state.Gamepad.wButtons);
			gp->setConnected(true);
		}
		else
		{
			gp->setConnected(false);
		}

		// update buttons:
		gp->setButtonDown(GamePad::BUTTON_DPAD_UP,		(state.Gamepad.wButtons & 0x1)!=0);
		gp->setButtonDown(GamePad::BUTTON_DPAD_DOWN,	(state.Gamepad.wButtons & 0x2)!=0);
		gp->setButtonDown(GamePad::BUTTON_DPAD_LEFT,	(state.Gamepad.wButtons & 0x4)!=0);
		gp->setButtonDown(GamePad::BUTTON_DPAD_RIGHT,	(state.Gamepad.wButtons & 0x8)!=0);
		gp->setButtonDown(GamePad::BUTTON_START,		(state.Gamepad.wButtons & 0x10)!=0);
		gp->setButtonDown(GamePad::BUTTON_AUX,			(state.Gamepad.wButtons & 0x20)!=0);
		gp->setButtonDown(GamePad::BUTTON_L_STICK,		(state.Gamepad.wButtons & 0x40)!=0);
		gp->setButtonDown(GamePad::BUTTON_R_STICK,		(state.Gamepad.wButtons & 0x80)!=0);
		gp->setButtonDown(GamePad::BUTTON_L_SHOULDER,	(state.Gamepad.wButtons & 0x100)!=0);
		gp->setButtonDown(GamePad::BUTTON_R_SHOULDER,	(state.Gamepad.wButtons & 0x200)!=0);
		gp->setButtonDown(GamePad::BUTTON_0,			(state.Gamepad.wButtons & 0x1000)!=0);
		gp->setButtonDown(GamePad::BUTTON_1,			(state.Gamepad.wButtons & 0x2000)!=0);
		gp->setButtonDown(GamePad::BUTTON_2,			(state.Gamepad.wButtons & 0x4000)!=0);
		gp->setButtonDown(GamePad::BUTTON_3,			(state.Gamepad.wButtons & 0x8000)!=0);

		// update triggers:
		gp->setTriggers((float)state.Gamepad.bLeftTrigger/(float)MAXBYTE,
						(float)state.Gamepad.bRightTrigger/(float)MAXBYTE);

		// update the analog sticks:
		gp->setAnalogL(
			(float)state.Gamepad.sThumbLX / (float)MAXSHORT,
			(float)state.Gamepad.sThumbLY / (float)MAXSHORT);
		gp->setAnalogR(
			(float)state.Gamepad.sThumbRX / (float)MAXSHORT,
			(float)state.Gamepad.sThumbRY / (float)MAXSHORT);
	}
}
