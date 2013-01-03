#pragma once

#include "CrtDbgInc.h"

#include <map>
#include <string>
#include <time.h>

namespace BoyLib
{
	struct PerfTimerEntry
	{
		std::string id;
		clock_t startTime;
		clock_t totalTime;
		clock_t minTime;
		clock_t maxTime;
		unsigned int calls;
	};

	class PerfTimer
	{
	public:

		static void setEnabled(bool enabled);
		static bool isEnabled();

		static void begin(const std::string &id);
		static void end(const std::string &id);

		static int getEntryCount();
		static const PerfTimerEntry *getEntry(int index);

	private:

		PerfTimer() {}
		virtual ~PerfTimer() {}

	private:

		static bool gIsEnabled;
		static std::map<std::string,PerfTimerEntry*> *gEntries;

	};
}
