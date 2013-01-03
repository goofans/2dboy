#pragma once

#include "CrtDbgInc.h"

namespace BoyLib
{
	class ReusableTickable
	{
	public:

		ReusableTickable(bool deleteWhenDone=true) { mDeleteWhenDone = deleteWhenDone; }
		virtual ~ReusableTickable() {}

		virtual bool tick() = 0;
		virtual void cancel() = 0;
		virtual void restart() = 0;

		inline bool deleteWhenDone() { return mDeleteWhenDone; }

	protected:

		bool mDeleteWhenDone;
	};
}