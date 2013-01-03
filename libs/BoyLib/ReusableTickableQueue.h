#pragma once

#include "ReusableTickable.h"
#include <queue>
#include <queue>

namespace BoyLib
{
	class ReusableTickableQueue : public ReusableTickable
	{
	public:

		ReusableTickableQueue();
		virtual ~ReusableTickableQueue();

		virtual void restart();
		virtual bool tick();
		virtual void cancel();

		void add(ReusableTickable *tickable);

	private:

		int mCurrent;
		std::vector<ReusableTickable*> mQueue;
	};
}