#pragma once

#include "ReusableTickable.h"
#include <vector>

namespace BoyLib
{
	class ReusableTickableSet : public ReusableTickable
	{
	public:
		ReusableTickableSet();
		virtual ~ReusableTickableSet();

		virtual void restart();
		virtual bool tick();
		virtual void cancel();

		void add(ReusableTickable *tickable);

	private:

		bool isDone();

	private:

		std::vector<ReusableTickable*> mSet;
		std::vector<bool> mDone;

	};
}