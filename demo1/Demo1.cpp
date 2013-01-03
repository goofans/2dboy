#include "Demo1.h"

#include <assert.h>
#include "Boy/Environment.h"
#include "Boy/GamePad.h"
#include "Boy/Graphics.h"
#include "Boy/Mouse.h"
#include <string>

Demo1 *Demo1::gInstance = NULL;

Demo1::Demo1()
{
}

Demo1::~Demo1()
{
}

Demo1 *Demo1::instance()
{
	if (gInstance==NULL)
	{
		gInstance = new Demo1();
	}

	return gInstance;
}

void Demo1::destroy()
{
	assert(gInstance!=NULL);
	delete gInstance;
	gInstance = NULL;
}

void Demo1::update(float dt)
{
	Boy::Graphics *g = Boy::Environment::instance()->getGraphics();
	float w = (float)g->getWidth();
	float h = (float)g->getHeight();

	Boy::GamePad *pad = Boy::Environment::instance()->getGamePad(0);
	mPos = pad->getAnalogR();
	mPos.x = w/2 + mPos.x*w/2;
	mPos.y = h/2 - mPos.y*h/2;
}

void Demo1::draw(Boy::Graphics *g)
{
	g->setColor(0xFFFFFFFF);
	g->fillRect((int)mPos.x-5,(int)mPos.y-5,10,10);
}

void Demo1::handleGamePadAdded(int gamePadId)
{
	Boy::Environment::instance()->getGamePad(gamePadId)->addListener(this);
}

void Demo1::handleGamePadRemoved(int gamePadId)
{
	Boy::Environment::instance()->getGamePad(gamePadId)->removeListener(this);
}

void Demo1::gamePadButtonDown(Boy::GamePad *pad, Boy::GamePad::Button button)
{
	Boy::Environment::instance()->stopMainLoop();
}

void Demo1::gamePadButtonUp(Boy::GamePad *pad, Boy::GamePad::Button button)
{
}
