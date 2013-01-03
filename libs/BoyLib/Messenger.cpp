#include "Messenger.h"

#include <algorithm>
#include <assert.h>
#include <stdio.h>
#include "MessageListener.h"

using namespace BoyLib;

Messenger *Messenger::gInstance = NULL;

#include "CrtDbgNew.h"

Messenger::Messenger()
{
}

Messenger::~Messenger()
{
	mGlobalListeners.clear();
}

void Messenger::sendMessage(std::string messageId, MessageSource *source, std::map<std::string,std::string> *params)
{
	// notify global listeners:
	for (int i=(int)mGlobalListeners.size()-1 ; i>=0 ; i--)
	{
		mGlobalListeners[i]->handleMessage(messageId,source,params);
	}
}

void Messenger::addListener(MessageListener *listener)
{
	mGlobalListeners.push_back(listener);
}

void Messenger::removeListener(MessageListener *listener)
{
	std::vector<MessageListener*>::iterator i = find(mGlobalListeners.begin(),mGlobalListeners.end(),listener);
	if (i!=mGlobalListeners.end())
	{
		mGlobalListeners.erase(i);
	}
}

Messenger *Messenger::instance()
{
	assert(gInstance!=NULL);
	return gInstance;
}

void Messenger::init()
{
	assert(gInstance==NULL);
	gInstance = new Messenger();
}

void Messenger::destroy()
{
	if (gInstance!=NULL)
	{
		delete gInstance;
		gInstance = NULL;
	}
}
