#include "ReusableTickableQueue.h"

#include <assert.h>

using namespace BoyLib;

#include "CrtDbgNew.h"

ReusableTickableQueue::ReusableTickableQueue() : ReusableTickable(false)
{
	mCurrent = -1;
}

ReusableTickableQueue::~ReusableTickableQueue()
{
	mCurrent = -1;
	for (int i=0 ; i<(int)mQueue.size() ; i++)
	{
		ReusableTickable *t = mQueue[i];
		t->cancel();
		if (t->deleteWhenDone())
		{
			delete t;
		}
	}
	mQueue.clear();
}

void ReusableTickableQueue::restart()
{
	// we can only restart once we've reached the end:
	assert(mCurrent>=(int)mQueue.size());

	// reset current:
	mCurrent = -1;

	// restart all tickables:
	for (int i=(int)mQueue.size()-1 ; i>=0 ; i--)
	{
		mQueue[i]->restart();
	}
}

bool ReusableTickableQueue::tick()
{
	// if there are no tickables in the queue, we're done:
	if (mQueue.empty())
	{
		return true;
	}

	// if we're at the end of the queue:
	if (mCurrent >= (int)mQueue.size())
	{
		return true;
	}

	// if this is the first tick:
	if (mCurrent < 0)
	{
		// start at the beginning:
		mCurrent = 0;
	}

	// get the current tickable and tick it:
	ReusableTickable *t = mQueue[mCurrent];
	for (bool done=t->tick() ; done ; done=t->tick())
	{
		// if we're in here, it's because the tickable
		// is done, so let's move on to the next one:

		// increment the current tickable index:
		mCurrent++;

		// if we've reached the end:
		if (mCurrent >= (int)mQueue.size())
		{
			// show's over, return true:
			return true;
		}
	}

	return false;
}

void ReusableTickableQueue::cancel()
{
	mCurrent = -1;

	// cancel the tickables on this queue:
	for (int i=0 ; i<(int)mQueue.size() ; i++)
	{
		mQueue[i]->cancel();
	}
}

void ReusableTickableQueue::add(ReusableTickable *tickable) 
{ 
	mQueue.push_back(tickable); 
}
