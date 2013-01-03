#pragma once

#include "BoyLib/CrtDbgInc.h"

namespace Boy
{
	class Graphics;

	/**
	 * game interface used by the framework to drive the main loop
	 */
	class Game
	{
	public:
		Game() {}
		virtual ~Game() {}

		/*
		 * called by the framework before the game's init() method
		 * is called.  this gives the game an opportunity to load the
		 * resources for preInitDraw
		 */
		virtual void preInitLoad() {}

		/*
		 * called by the framework before the game's init() method
		 * is called.  this gives the game an opportunity to draw
		 * something to the screen that will be displayed until the
		 * first call to draw(), which will take place after the
		 * init() call.
		 */
		virtual void preInitDraw(Graphics *g) {}

		/*
		 * called by the framework after subsystems are initialized 
		 * but before the initial game loading commences.  this is
		 * where the bare-bones initialization tasks (e.g. creating
		 * the loading screen) should be performed. 
		 */
		virtual void init() {}

		/*
		 * called after init().  this is where the game should do 
		 * the more beefy initialization work (e.g. heavy resource 
		 * loading, heavy graphics setup).  this call runs in a 
		 * separate thread, so be careful what you do here.  		 
		 */
		virtual void load() {}

		/*
		 * called after load() finishes running.  this call runs in
		 * the main thread.
		 */
		virtual void loadComplete() {}

		/*
		 * this method is called by the framework once for every
		 * iteration of the game loop, immediately before rendering
		 * takes place.  dt is the amount of time that's passed 
		 * since the last update call.
		 */
		virtual void update(float dt) = 0;

		/*
		 * called by the framework once for every iteration of the
		 * main loop.  implementations should draw themselves using 
		 * the Graphics object on every call to this method
		 */
		virtual void draw(Graphics *g) = 0;

		/*
		 * called by the framework after window size has changed
		 */
		virtual void windowResized(int oldWidth, int oldHeight, int newWidth, int newHeight) {}

		/*
		 * called by the framework after a fullscreen toggle takes place
		 */
		virtual void fullscreenToggled(bool isFullscreen) {}

		/*
		 * the game can override this method to perform any cleanup 
		 * operations it needs to at shutdown.  it will be called
		 * after the main loop exits.
		 */
		virtual void preShutdown() {}

		/*
		 * called by the framework when a new mouse object becomes available
		 */
		virtual void handleMouseAdded(int mouseId) {}

		/*
		 * called by the framework when an existing mouse object becomes unavailable
		 */
		virtual void handleMouseRemoved(int mouseId) {}

		/*
		 * called by the framework when a new game pad object becomes available
		 */
		virtual void handleGamePadAdded(int gamePadId) {}

		/*
		 * called by the framework when an existing game pad object becomes unavailable
		 */
		virtual void handleGamePadRemoved(int gamePadId) {}

		/*
		 * called when the window loses focus
		 */
		virtual void focusLost() {}

		/*
		 * called when the window gains focus
		 */
		virtual void focusGained() {}

	};
}
