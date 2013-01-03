#pragma once

#include "BoyLib/UString.h"
#include "Environment.h"
#include "Keyboard.h"
#include <map>
#include "Mouse.h"
#include "SDL.h"
#include "WinGamePad.h"

namespace Boy
{
	class Game;
	class ResourceLoader;
	class WinGraphics;
	class WinD3DInterface;
	class WinImage;
	class WinStorage;

	class WinEnvironment : public Environment
	{
	public:

		WinEnvironment();
		virtual ~WinEnvironment();

		// implementation of Environment:
		virtual void				init(Game *game, int screenWidth, int screenHeight, bool fullscreen,
										 const char *windowTitle, const UString &persFile, unsigned char *persFileKey);
		virtual void				destroy();
		virtual Graphics			*getGraphics();
		virtual int					getMouseCount();
		virtual Mouse				*getFirstMouse();
		virtual Mouse				*getMouse(int mouseId);
		virtual int					getGamePadCount();
		virtual GamePad				*getGamePad(int i);
		virtual void				showSystemMouse(bool show);
		virtual int					getKeyboardCount();
		virtual Keyboard			*getKeyboard(int i);
		virtual int					getWiimoteCount();
		virtual Wiimote				*getWiimote(int i);
		virtual ResourceManager		*getResourceManager();
		virtual PersistenceLayer	*getPersistenceLayer();
		virtual SoundPlayer			*getSoundPlayer();
		virtual TriStrip			*createTriStrip(int numVerts);
		virtual void				startMainLoop();
		virtual void				stopMainLoop();
		virtual bool				isShuttingDown();
		virtual void				showError(const std::string &message);
		virtual float				getTime();
		virtual void				pauseTime();
		virtual void				resumeTime();
		virtual int					getMaxFrameRate();
		virtual void				setMaxFrameRate(int fps);
		virtual void				debugLog(const char *fmt, ...);
		virtual void				screenshot(const char *filename);
		virtual void				setMute(bool mute);
		virtual bool				isMute();
		virtual void				setDebugEnabled(bool enabled);
		virtual bool				isDebugEnabled();
		virtual bool				isFullScreen();
		virtual void				toggleFullScreen();
		virtual void				enableFullScreenToggle();
		virtual void				disableFullScreenToggle();
		virtual void				sleep(int milliseconds);
		virtual void				dumpEnvironmentInfo(const char *filename);
		virtual std::string			getHardwareId();
		virtual unsigned char		*getCryptoKey();
		virtual int					sprintf( char *pBuffer, int bufferLenChars, const char *pFormat, ... );
		virtual int					stricmp( const char *pStr1, const char *pStr2 );
		virtual Storage				*getStorage();
		virtual int					getSafeZoneInset();
		virtual bool				isWindowResizable() { return true; }

		bool						processVirtualMouseEvents(unsigned int key, bool down);
		void						getDesiredScreenSize(int *screenWidth, int *screenHeight);

	protected:

		void						update();
		void						draw();
		void						printTimingStats();
		void						setLogFile(FILE *f);
		void						updateVirtualMice();
		void						loadConfig();
		void						checkMouseInBounds();
		void						pollGamePads();

	protected:

		// subsystems:
		PersistenceLayer			*mPersistenceLayer;
		ResourceManager				*mResourceManager;
		ResourceLoader				*mResourceLoader;
		WinGraphics					*mGraphics;
		SoundPlayer					*mSoundPlayer;
		WinStorage					*mStorage;
		std::map<std::string,std::string> mConfig;

		// SDL interface:
		WinD3DInterface				*mPlatformInterface;

		// controllers:
		Mouse						*mMice[MOUSE_COUNT_MAX];
		GamePad						*mGamePads[GAMEPAD_COUNT_MAX];
		BoyLib::Vector2				mMouseVelocity[MOUSE_COUNT_MAX];
		Keyboard					*mKeyboard;
		bool						mShowSystemMouse;

		// sound related:
		float						mLastVolume;

		// shutdown flag:
		bool						mShutdownRequested;

		// runtime debug flag:
		bool						mIsDebugEnabled;

		// log file:
		FILE						*mLogFile;

		// graphics stuff:
		int							mFullScreenToggleDisableCount;
		BoyLib::Vector2				mLastKnownWindowSize;
		bool						mMouseInBounds;

		// timing related:
		Uint32						mT0;
		Uint32						mPauseTime;
		Uint32						mPauseDuration;
		int							mPauseCount;
		Uint32						mMaxFrameRate; // maximum allowed frame rate
		Uint32						mMinStepSize; // minimum delay between frames (in ms)
		Uint32						mUpdateCount;
		Uint32						mLastUpdate;

		Uint32						mIntervalFrameCount;
		Uint32						mIntervalStartTime;

		unsigned char				*mpCryptoKey;

		// temporary:
		bool						mIsLeftMouseButtonDown[MOUSE_COUNT_MAX];
	};
}
