#include "PerfTimer.h"

#include <assert.h>

using namespace BoyLib;

bool PerfTimer::gIsEnabled = false;
std::map<std::string,PerfTimerEntry*> *PerfTimer::gEntries = NULL;

#include "CrtDbgNew.h"

void PerfTimer::setEnabled(bool enabled)
{
	gIsEnabled = enabled;
	if (enabled)
	{
		gEntries = new std::map<std::string,PerfTimerEntry*>;
		gEntries->clear();
	}
	else
	{
		delete gEntries;
	}
}

bool PerfTimer::isEnabled()
{
	return gIsEnabled;
}

void PerfTimer::begin(const std::string &id)
{
	if (!gIsEnabled)
	{
		return;
	}

	std::map<std::string,PerfTimerEntry*>::iterator iter = gEntries->find(id);

	PerfTimerEntry *entry;

	// if there is no entry for this id:
	if (iter==gEntries->end())
	{
		// create one:
		entry = new PerfTimerEntry;
		entry->id = id;
		entry->totalTime = 0;
		entry->calls = 0;
		entry->minTime = -1;
		entry->maxTime = -1;
		(*gEntries)[id] = entry;
	}
	else
	{
		entry = iter->second;
		assert(entry->startTime<0);
	}

	// remember start time:
	entry->startTime = clock();
	entry->calls++;
}

void PerfTimer::end(const std::string &id)
{
	if (!gIsEnabled)
	{
		return;
	}

	std::map<std::string,PerfTimerEntry*>::iterator iter = gEntries->find(id);

	// if there is an entry for this id:
	// (it's possible that the timer was activated between a begin/end pair
	// and so no entry exists for a particular id.  in this case we should
	// do nothing)
	if (iter!=gEntries->end())
	{
		// get the entry for this id:
		PerfTimerEntry *entry = iter->second;
		assert(entry->startTime>=0);

		// add time since begin call;
		clock_t dt = clock() - entry->startTime;
		entry->totalTime += dt;
		if (entry->minTime<0 || entry->minTime>dt)
		{
			entry->minTime = dt;
		}
		if (entry->maxTime<0 || entry->maxTime<dt)
		{
			entry->maxTime = dt;
		}

		// stop timing for this entry:
		entry->startTime = -1;
	}
}

int PerfTimer::getEntryCount()
{
	return (int)gEntries->size();
}

const PerfTimerEntry *PerfTimer::getEntry(int index)
{
	// TODO: implement this in a nicer way... this is fugly
	assert(index<(int)gEntries->size());
	std::map<std::string,PerfTimerEntry*>::iterator iter = gEntries->begin();
	for (int i=0 ; i<index ; i++)
	{
		iter++;
	}
	return iter->second;
}

