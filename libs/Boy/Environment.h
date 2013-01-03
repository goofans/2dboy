#pragma once

#include "BoyLib/CrtDbgInc.h"

#include <string>
#include <stdio.h>
#include "BoyLib/UString.h"

#define MOUSE_COUNT_MAX 4
#define GAMEPAD_COUNT_MAX 4

namespace Boy
{
	class Game;
	class GamePad;
	class Graphics;
	class HttpResponseHandler;
	class Image;
	class Keyboard;
	class Mouse;
	class PersistenceLayer;
	class Renderer;
	class ResourceManager;
	class Sound;
	class SoundPlayer;
	class TriStrip;
	class Updater;
	class Wiimote;
	class Storage;

	class Environment
	{
	public:

		// static access method:
		static Environment			*instance();

		// init and destroy
		virtual void				init(Game *game, int screenWidth, int screenHeight, bool fullscreen, 
										 const char *windowTitle, const UString &persFile, unsigned char *persFileKey);
		virtual void				destroy();

		// main loop related:
		virtual void				startMainLoop() = 0;
		virtual void				stopMainLoop() = 0;
		virtual bool				isShuttingDown() = 0;

		// debug:
		virtual void				setDebugEnabled(bool enabled) = 0;
		virtual bool				isDebugEnabled() = 0;

		// timing related:
		virtual float				getTime() = 0;
		virtual void				pauseTime() = 0;
		virtual void				resumeTime() = 0;
		virtual int					getMaxFrameRate() = 0;
		virtual void				setMaxFrameRate(int framesPerSecond) = 0;
		virtual void				sleep(int milliseconds) = 0;

		// graphics:
		virtual Graphics			*getGraphics() = 0;
		virtual TriStrip			*createTriStrip(int numVerts) = 0;

		// controllers:
		virtual int					getMouseCount() = 0;
		virtual Mouse				*getMouse(int mouseId) = 0;
		virtual Mouse				*getFirstMouse() = 0;
		virtual int					getKeyboardCount() = 0;
		virtual Keyboard			*getKeyboard(int i) = 0;
		virtual int					getWiimoteCount() = 0;
		virtual Wiimote				*getWiimote(int i) = 0;
		virtual int					getGamePadCount() = 0;
		virtual GamePad				*getGamePad(int i) = 0;
		virtual void				showSystemMouse(bool show) = 0;
		void						fireMouseAdded(int mouseId);
		void						fireMouseRemoved(int mouseId);
		void						fireGamePadAdded(int gamePadId);
		void						fireGamePadRemoved(int gamePadId);

		// resource manager:
		virtual ResourceManager		*getResourceManager() = 0;

		// persistence layer:
		virtual PersistenceLayer	*getPersistenceLayer() = 0;

		// sound:
		virtual SoundPlayer			*getSoundPlayer() = 0;
		virtual void				setMute(bool mute) = 0;
		virtual bool				isMute() = 0;

		// misc:
		virtual void				showError(const std::string &message) = 0;
		virtual std::string			getHardwareId() = 0;
		virtual unsigned char		*getCryptoKey() = 0;
		virtual bool				isWindowResizable() = 0;

		// fullscreen:
		virtual bool				isFullScreen() = 0;
		virtual void				toggleFullScreen() = 0;
		virtual void				enableFullScreenToggle() = 0;
		virtual void				disableFullScreenToggle() = 0;

		// debug/misc:
		virtual void				debugLog(const char *fmt, ...) = 0;
		virtual void				screenshot(const char *filename) = 0;
		virtual void				dumpEnvironmentInfo(const char *filename) = 0;
		virtual void				setLogFile(FILE *logFile) = 0;

		// std lib wrappers
		virtual int					sprintf( char *pBuffer, int bufferLenChars, const char *pFormat, ... ) = 0;
		virtual int					stricmp( const char *pStr1, const char *pStr2 ) = 0;

		// file io
		virtual Storage				*getStorage() = 0;

		// shortcut methods:
		static Image				*getImage(const std::string &id);
		static Image				*getImage(const std::string &id, Image *defaultImg);
		static Image				*getImage(const char *id, Image *defaultImg);
		static void					playSound(const char *id, bool loop=false, float volume=1.0f);
		static void					playSound(Sound *sound, bool loop=false, float volume=1.0f);
		static void					stopSound(const char *id);
		static void					stopSound(Sound *sound);
		static int					screenWidth();
		static int					screenHeight();

		// config:
		virtual int					getSafeZoneInset() = 0;

	protected:

		Environment();
		virtual ~Environment();

		void						InitTinyXML();
		static void					*TiXmlFileOpen( const char *pFilePathUtf8 );
		static int					TiXmlFileRead( void *pContext, void *pBuffer, int readSizeBytes );
		static int					TiXmlFileSize( void *pContext );
		static void					TiXmlFileClose( void *pContext );

	private:

		static Environment			*gInstance;

	protected:

		// the game:
		Game						*mGame;

	};
}

// shorthand notation for debug logging:
#if defined(GOO_PLATFORM_WII)
	#if defined(GOO_CONFIG_FINAL)
		#define envDebugLog(format, args...) do {} while(0)
	#else
		#define envDebugLog(format, args...) Boy::Environment::instance()->debugLog( format , ## args )
	#endif
#elif defined(GOO_PLATFORM_WIN32)
	#define envDebugLog(format,...) Boy::Environment::instance()->debugLog( format , __VA_ARGS__ )
#elif defined(GOO_PLATFORM_OSX)
	#if defined(GOO_CONFIG_FINAL)
		#define envDebugLog(format, args...) do {} while(0)
	#else
		#define envDebugLog(format, args...) Boy::Environment::instance()->debugLog( format , ## args )
	#endif
#elif defined(GOO_PLATFORM_LINUX)
	#define envDebugLog(format, args...) Boy::Environment::instance()->debugLog( format , ## args )
#endif
