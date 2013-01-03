#pragma once

#include "Function.h"

namespace BoyLib
{
	class SmoothTransitionFunction : public Function
	{
	public:

		SmoothTransitionFunction(float x0, float x1, float y0, float y1);
		virtual ~SmoothTransitionFunction();

		virtual float eval(float x);
		virtual bool canEval(float x);
		virtual float getEndValue() { return mY1; }

	private:

		float mX0;
		float mX1;
		float mY0;
		float mY1;
	};
}