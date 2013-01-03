#include "TickableQueue.h"

#include <assert.h>

using namespace BoyLib;

#include "CrtDbgNew.h"

TickableQueue::TickableQueue(bool deleteWhenDone) : Tickable(deleteWhenDone)
{
	mCurrent = NULL;
}

TickableQueue::~TickableQueue()
{
	mCurrent = NULL;

	// deallocate all tickables on this queue:
	while (!mQueue.empty())
	{
		Tickable *t = mQueue.front();
		if (t->deleteWhenDone())
		{
			delete t;
		}
		mQueue.pop();
	}
}

bool TickableQueue::tick()
{
	// if there are no tickables in the queue, we're done:
	if (mQueue.empty())
	{
		return true;
	}

	// if we haven't selected the current tickable:
	if (mCurrent==NULL)
	{
		// get it from the queue:
		mCurrent = mQueue.front();
	}

	for (bool done=mCurrent->tick() ; done ; done=mCurrent->tick())
	{
		// if this tickable is to be deleted:
		if (mCurrent->deleteWhenDone())
		{
			delete mCurrent;
		}

		// the current tickable is done:
		mCurrent = NULL;

		// pop it off the queue:
		mQueue.pop();

		// if the queue is empty:
		if (mQueue.empty())
		{
			// we're done:
			return true;
		}

		// let set up the next tickable for ticking:
		mCurrent = mQueue.front();
	}

	return false;
}

void TickableQueue::cancel()
{
	mCurrent = NULL;

	// cancel all tickables on this queue:
	std::vector<Tickable*> deleteList;
	while (!mQueue.empty())
	{
		Tickable *t = mQueue.front();
		t->cancel();
		if (t->deleteWhenDone())
		{
			deleteList.push_back(t);
		}
		mQueue.pop();
	}

	// delete all tickables on this queue:
	for (int i=0 ; i<(int)deleteList.size() ; i++)
	{
		delete deleteList[i];
	}
	deleteList.clear();
}

void TickableQueue::add(Tickable *tickable) 
{ 
	assert(tickable!=NULL);
	mQueue.push(tickable); 
}

bool TickableQueue::isEmpty() 
{ 
	return mQueue.size()==0; 
}
