#include "Mouse.h"

#include <algorithm>
#include "BoyLib/BoyUtil.h"
#include "Environment.h"
#include "Graphics.h"
#include "Image.h"
#include "MouseListener.h"

using namespace Boy;

#include "BoyLib/CrtDbgNew.h"

Mouse::Mouse(int id)
{
	mIsVisible = true;
	mId = id;
	mButtonsEnabled = true;
	mIsButtonDown[BUTTON_LEFT] = false;
	mIsButtonDown[BUTTON_RIGHT] = false;
	mIsButtonDown[BUTTON_MIDDLE] = false;
	mIsInBounds = false;
}

Mouse::~Mouse()
{
}

void Mouse::setVisible(bool visible)
{
	mIsVisible = visible;
}

bool Mouse::isVisible()
{
	return mIsVisible;
}

void Mouse::addListener(MouseListener *listener)
{
	assert(find(mListeners.begin(),mListeners.end(),listener) == mListeners.end());
	mListeners.push_back(listener);
}

void Mouse::removeListener(MouseListener *listener)
{
	assert(find(mListeners.begin(),mListeners.end(),listener) != mListeners.end());
	mListeners.erase(find(mListeners.begin(),mListeners.end(),listener));
}

void Mouse::addLowLevelListener(MouseListener *listener)
{
	assert(find(mLowLevelListeners.begin(),mLowLevelListeners.end(),listener) == mLowLevelListeners.end());
	mLowLevelListeners.push_back(listener);
}

void Mouse::removeLowLevelListener(MouseListener *listener)
{
	assert(find(mLowLevelListeners.begin(),mLowLevelListeners.end(),listener) != mLowLevelListeners.end());
	mLowLevelListeners.erase(find(mLowLevelListeners.begin(),mLowLevelListeners.end(),listener));
}

void Mouse::fireMoveEvent(float x, float y)
{
	// if this is not a valid move event:
	if (!isValidMouseCoord(x,y))
	{
		// do nothing:
		return;
	}

	mPosition.x = x;
	mPosition.y = y;
	mIsInBounds = true;
	if (mListeners.size()>0 && isEnabled())
	{
		int numListeners = (int)mListeners.size();
		for (int i=0 ; i<numListeners ; i++)
		{
			mListeners[i]->mouseMove(this);
		}
	}

	int numListeners = (int)mLowLevelListeners.size();
	for (int i=0 ; i<numListeners ; i++)
	{
		mLowLevelListeners[i]->mouseMove(this);
	}
}

void Mouse::fireDownEvent(Button button, int clickCount)
{
	mIsButtonDown[button] = true;
	if (mListeners.size()>0 && isEnabled() && mButtonsEnabled)
	{
		int numListeners = (int)mListeners.size();
		for (int i=0 ; i<numListeners ; i++)
		{
			mListeners[i]->mouseButtonDown(this,button,clickCount);
		}
	}

	int numListeners = (int)mLowLevelListeners.size();
	for (int i=0 ; i<numListeners ; i++)
	{
		mLowLevelListeners[i]->mouseButtonDown(this,button,clickCount);
	}
}

void Mouse::fireUpEvent(Button button)
{
	mIsButtonDown[button] = false;
	if (mListeners.size()>0 && isEnabled() && mButtonsEnabled)
	{
		int numListeners = (int)mListeners.size();
		for (int i=0 ; i<numListeners ; i++)
		{
			mListeners[i]->mouseButtonUp(this,button);
		}
	}

	int numListeners = (int)mLowLevelListeners.size();
	for (int i=0 ; i<numListeners ; i++)
	{
		mLowLevelListeners[i]->mouseButtonUp(this,button);
	}
}

void Mouse::fireWheelEvent(int delta)
{
	if (mListeners.size()>0 && isEnabled() && mButtonsEnabled)
	{
		int numListeners = (int)mListeners.size();
		for (int i=0 ; i<numListeners ; i++)
		{
			mListeners[i]->mouseWheel(this,delta);
		}
	}

	int numListeners = (int)mLowLevelListeners.size();
	for (int i=0 ; i<numListeners ; i++)
	{
		mLowLevelListeners[i]->mouseWheel(this,delta);
	}
}

void Mouse::fireEnterEvent()
{
	setVisible(true);

	mIsInBounds = true;

	if (mListeners.size()>0 && isEnabled())
	{
		int numListeners = (int)mListeners.size();
		for (int i=0 ; i<numListeners ; i++)
		{
			mListeners[i]->mouseEnter(this);
		}
	}

	int numListeners = (int)mLowLevelListeners.size();
	for (int i=0 ; i<numListeners ; i++)
	{
		mLowLevelListeners[i]->mouseEnter(this);
	}
}

void Mouse::fireLeaveEvent()
{
	setVisible(false);

	mIsInBounds = false;

	if (mListeners.size()>0 && isEnabled())
	{
		int numListeners = (int)mListeners.size();
		for (int i=0 ; i<numListeners ; i++)
		{
			mListeners[i]->mouseLeave(this);
		}
	}

	int numListeners = (int)mLowLevelListeners.size();
	for (int i=0 ; i<numListeners ; i++)
	{
		mLowLevelListeners[i]->mouseLeave(this);
	}
}

const BoyLib::Vector2 &Mouse::getPosition()
{
	return mPosition;
}

void Mouse::setPosition(const BoyLib::Vector2 &pos)
{
	mPosition = pos;
}

void Mouse::setConnected(bool connected) 
{ 
	// if there's no change, return:
	if (connected==isConnected())
	{
		return;
	}

	// connect the controller:
	Controller::setConnected(connected);

	// fire notification:
	if (connected)
	{
		Environment::instance()->fireMouseAdded(mId);
	}
	else
	{
		Environment::instance()->fireMouseRemoved(mId);
	}
}

void Mouse::setEnabled(bool enabled)
{
	Controller::setEnabled(enabled);
//	envDebugLog("-----------> mouse %d: enabled=%d\n",mId,enabled);
}

bool Mouse::getButtonsEnabled() 
{ 
	return mButtonsEnabled; 
}

void Mouse::setButtonsEnabled(bool enabled) 
{ 
//	envDebugLog("-----------> mouse %d: buttons=%d\n",mId,enabled);
	mButtonsEnabled = enabled; 
}

bool Mouse::isButtonDown(Button button)
{
	assert(button==BUTTON_LEFT || button==BUTTON_RIGHT || button==BUTTON_MIDDLE);
	return mIsButtonDown[button];
}

bool Mouse::isInBounds()
{
	return mIsInBounds;
}

bool Mouse::isValidMouseCoord(float x, float y)
{
	Boy::Graphics *g = Boy::Environment::instance()->getGraphics();
	int gw = g->getWidth();
	int gh = g->getHeight();
	if (x >= -gw/2 && x <= gw*3/2 &&
		y >= -gh/2 && y <= gh*3/2)
	{
		return true;
	}

	return false;
}
