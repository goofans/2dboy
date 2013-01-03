#include "SmoothTransitionFunction.h"

#include "BoyUtil.h"

using namespace BoyLib;

#include "CrtDbgNew.h"

SmoothTransitionFunction::SmoothTransitionFunction(float x0, float x1, float y0, float y1)
{
	mX0 = x0;
	mX1 = x1;
	mY0 = y0;
	mY1 = y1;
	assert(x0<=x1);
}

SmoothTransitionFunction::~SmoothTransitionFunction()
{
}

float SmoothTransitionFunction::eval(float x)
{
	if (mX0==mX1)
	{
		return mY1;
	}

	float normalizedX = (x-mX0) / (mX1-mX0);
	return mY0 + ((1 - cos(normalizedX * GPI)) / 2) * (mY1-mY0);
}

bool SmoothTransitionFunction::canEval(float x)
{
	return x>=mX0 && x<=mX1;
}

