#pragma once

#include "Boundary.h"

namespace BoyLib
{
	class IntersectionBoundary : public Boundary
	{
	public:

		IntersectionBoundary(int maxBoundaries=4);
		virtual ~IntersectionBoundary();

		virtual bool contains(const Vector2 &point);

		virtual Boundary *clone();

	private:

		Boundary **mBoundaries;
		int mNumBoundaries;
		int mMaxBoundaries;
		
	};
}
