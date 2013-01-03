#include "ReusableTickableSet.h"

#include <assert.h>

using namespace BoyLib;

#include "CrtDbgNew.h"

ReusableTickableSet::ReusableTickableSet()
{
}

ReusableTickableSet::~ReusableTickableSet()
{
	// deallocate all remaining tickables:
	int numTickables = (int)mSet.size();
	for (int i=numTickables-1 ; i>=0 ; i--)
	{
		ReusableTickable *t = mSet[i];
		if (t->deleteWhenDone())
		{
			delete mSet[i];
		}
	}
	mSet.clear();
}

bool ReusableTickableSet::tick()
{
	bool allDone = true;
	int numTickables = (int)mSet.size();
	for (int i=0 ; i<numTickables ; i++)
	{
		// if the next tickable is done:
		if (mDone[i])
		{
			// skip it:
			continue;
		}

		// get the next tickable:
		ReusableTickable *t = mSet[i];

		// tick it:
		bool done =  t->tick();

		// if it finished:
		if (done)
		{
			mDone[i] = true;
		}
		else
		{
			allDone = false;
		}
	}

	return allDone;
}

void ReusableTickableSet::cancel()
{
	int numTickables = (int)mSet.size();
	for (int i=0 ; i<numTickables ; i++)
	{
		mSet[i]->cancel();
		mDone[i] = true;
	}
}

void ReusableTickableSet::restart()
{
	// we can only restart once we've reached the end:
	assert(isDone());

	// restart all tickables:
	for (int i=(int)mSet.size()-1 ; i>=0 ; i--)
	{
		mSet[i]->restart();
		mDone[i] = false;
	}
}

void ReusableTickableSet::add(ReusableTickable *tickable) 
{ 
	mSet.push_back(tickable); 
	mDone.push_back(false);
}

bool ReusableTickableSet::isDone()
{
	for (int i=(int)mSet.size()-1 ; i>=0 ; i--)
	{
		if (!mDone[i])
		{
			return false;
		}
	}

	return true;
}