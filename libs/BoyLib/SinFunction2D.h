#pragma once

#include "Function2D.h"

namespace BoyLib
{
	class SinFunction2D : public Function2D
	{
	public:
		SinFunction2D(float ampX, float freqX, float phaseshiftX,
					  float ampY, float freqY, float phaseshiftY);
		virtual ~SinFunction2D();

		virtual float evalX(float t);
		virtual float evalY(float t);

		virtual bool canEval(float t);

		virtual Function2D *clone();

	private:

		Vector2 mAmp;
		Vector2 mFreq;
		Vector2 mPhaseshift;
	};
}
