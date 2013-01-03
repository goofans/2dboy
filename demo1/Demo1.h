#pragma once

#include "Boy/Game.h"
#include "Boy/GamePadListener.h"
#include "BoyLib/Vector2.h"

class Demo1: public Boy::Game, public Boy::GamePadListener
{
public:

	Demo1();
	virtual ~Demo1();

	// singleton:
	static Demo1 *instance();
	static void destroy();

	// implementation of Game:
	virtual void update(float dt);
	virtual void draw(Boy::Graphics *g);
	virtual void handleGamePadAdded(int gamePadId);
	virtual void handleGamePadRemoved(int gamePadId);

	// implementation of GamePadListener:
	virtual void gamePadButtonDown(Boy::GamePad *pad, Boy::GamePad::Button button);
	virtual void gamePadButtonUp(Boy::GamePad *pad, Boy::GamePad::Button button);

private:

	static Demo1 *gInstance;

	BoyLib::Vector2 mPos;

};
