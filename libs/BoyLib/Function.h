#pragma once

#include "CrtDbgInc.h"

namespace BoyLib
{
	class Function
	{
	public:
		Function() {}
		virtual ~Function() {}

		virtual float eval(float x) = 0;
		virtual bool canEval(float x) = 0;
		virtual float getEndValue() = 0; // returns the value for the highest 'x' the function can evaluate 

	};
}