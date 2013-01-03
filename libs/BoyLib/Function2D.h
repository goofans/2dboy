#pragma once

#include "Vector2.h"

namespace BoyLib
{
	class Function2D
	{
	public:

		Function2D() {}
		virtual ~Function2D() {}

		inline Vector2 eval(float t) { return Vector2(evalX(t),evalY(t)); }
		virtual float evalX(float t) = 0;
		virtual float evalY(float t) = 0;
		virtual bool canEval(float t) = 0;

		virtual Function2D *clone() = 0;

	};
}