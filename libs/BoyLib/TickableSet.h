#pragma once

#include "Tickable.h"
#include <vector>

namespace BoyLib
{
	class TickableSet : public Tickable
	{
	public:
		TickableSet();
		virtual ~TickableSet();

		virtual bool tick();
		virtual void cancel();

		inline void add(Tickable *tickable) { mSet.push_back(tickable); }

	private:

		std::vector<Tickable*> mSet;

	};
}