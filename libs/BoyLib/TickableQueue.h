#pragma once

#include "Tickable.h"
#include <queue>
#include <queue>

namespace BoyLib
{
	class TickableQueue : public Tickable
	{
	public:
		TickableQueue(bool deleteWhenDone = true);
		virtual ~TickableQueue();

		virtual bool tick();
		virtual void cancel();

		void add(Tickable *tickable);
		bool isEmpty();

	private:

		Tickable *mCurrent;
		std::queue<Tickable*> mQueue;
	};
}