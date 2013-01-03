#include "TickableSet.h"

using namespace BoyLib;

#include "CrtDbgNew.h"

TickableSet::TickableSet()
{
}

TickableSet::~TickableSet()
{
	// deallocate all remaining tickables:
	int numTickables = (int)mSet.size();
	for (int i=numTickables-1 ; i>=0 ; i--)
	{
		Tickable *t = mSet[i];
		if (t->deleteWhenDone())
		{
			delete mSet[i];
		}
	}
	mSet.clear();
}

bool TickableSet::tick()
{
	int numTickables = (int)mSet.size();
	for (int i=numTickables-1 ; i>=0 ; i--)
	{
		// get the next tickable:
		Tickable *t = mSet[i];

		// tick it:
		bool done = t->tick();

		// if it's done:
		if (done)
		{
			// remove it from the set:
			mSet.erase(mSet.begin()+i);

			// if this tickable is to be deleted:
			if (t->deleteWhenDone())
			{
				delete t;
			}
		}
	}

	return mSet.empty();
}

void TickableSet::cancel()
{
	int numTickables = (int)mSet.size();

	// first cancel them all:
	for (int i=0 ; i<numTickables ; i++)
	{
		Tickable *t = mSet[i];
		t->cancel();
	}

	// now delete them all:
	// (this two-phase approach works around some data integrity 
	// issues that could come up during cancellations -- see bug 151)
	for (int i=0 ; i<numTickables ; i++)
	{
		Tickable *t = mSet[i];
		if (t->deleteWhenDone())
		{
			delete t;
		}
	}
	mSet.clear();
}
