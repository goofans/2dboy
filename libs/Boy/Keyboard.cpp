#include "Keyboard.h"

#include <algorithm>
#include <assert.h>
#include "Environment.h"
#include "KeyboardListener.h"

using namespace Boy;

#include "BoyLib/CrtDbgNew.h"

Keyboard::Keyboard()
{
}

Keyboard::~Keyboard()
{
}

void Keyboard::addListener(KeyboardListener *listener)
{
	assert(find(mListeners.begin(),mListeners.end(),listener) == mListeners.end());
	mListeners.push_back(listener);
}

void Keyboard::removeListener(KeyboardListener *listener)
{
	assert(find(mListeners.begin(),mListeners.end(),listener) != mListeners.end());
	mListeners.erase(find(mListeners.begin(),mListeners.end(),listener));
}

void Keyboard::fireKeyDownEvent(wchar_t unicode, Key key, Modifiers mods)
{
//	envDebugLog("Keyboard::fireKeyDownEvent(): unicode=%c key=0x%02x\n",unicode,key);
	if (mListeners.size()>0 && isEnabled())
	{
		int numListeners = (int)mListeners.size();
		for (int i=0 ; i<numListeners ; i++)
		{
			mListeners[i]->keyDown(unicode, key, mods);
		}
	}
}

void Keyboard::fireKeyUpEvent(wchar_t unicode, Key key, Modifiers mods)
{
//	envDebugLog("Keyboard::fireKeyUpEvent(): unicode=%c key=0x%02x\n",unicode,key);
	if (mListeners.size()>0 && isEnabled())
	{
		int numListeners = (int)mListeners.size();
		for (int i=0 ; i<numListeners ; i++)
		{
			mListeners[i]->keyUp(unicode, key, mods);
		}
	}
}
