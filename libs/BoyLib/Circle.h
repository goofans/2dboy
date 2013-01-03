#pragma once

#include "Boundary.h"
#include <map>

namespace BoyLib
{
	class Circle : public Boundary
	{
	public:

		Circle(float x, float y, float radius);
		virtual ~Circle();

		virtual bool contains(const Vector2 &point);

		virtual Boundary *clone();

		inline const Vector2 &getCenter() { return mCenter; }
		inline float getRadius() { return mRadius; }

	private:

		Vector2 mCenter;
		float mRadius;

	};
}