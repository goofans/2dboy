#include "SinFunction2D.h"

#include "BoyUtil.h"

using namespace BoyLib;

#include "CrtDbgNew.h"

SinFunction2D::SinFunction2D(float ampX, float freqX, float phaseshiftX,
							 float ampY, float freqY, float phaseshiftY)
{
	mAmp.x = ampX;
	mAmp.y = ampY;
	mFreq.x = freqX;
	mFreq.y = freqY;
	mPhaseshift.x = phaseshiftX*GPI*2;
	mPhaseshift.y = phaseshiftY*GPI*2;
}

SinFunction2D::~SinFunction2D()
{
}

float SinFunction2D::evalX(float t)
{
	return sin(mFreq.x*(t+mPhaseshift.x))*mAmp.x;
}

float SinFunction2D::evalY(float t)
{
	return sin(mFreq.y*(t+mPhaseshift.y))*mAmp.y;
}

bool SinFunction2D::canEval(float t)
{
	return true;
}

Function2D *SinFunction2D::clone()
{
	return new SinFunction2D(
		mAmp.x, mFreq.x, mPhaseshift.x,
		mAmp.y, mFreq.y, mPhaseshift.y);
}

