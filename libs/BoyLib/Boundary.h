#pragma once

#include "Vector2.h"
#include <map>

namespace BoyLib
{
	class Boundary
	{
	public:

		Boundary() {}
		virtual ~Boundary() {}

		virtual bool contains(const Vector2 &point) = 0;
		virtual Boundary *clone() = 0;

	};
}
