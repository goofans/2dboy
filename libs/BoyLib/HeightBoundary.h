#pragma once

#include "Boundary.h"

namespace BoyLib
{
	/*
	 * a 2D version of a height field.  this can represent the region above OR 
	 * below the "height boundary".  by default it represents the region below.
	 * if the constructor is given reversed=true, it will represent the region
	 * above
	 */
	class HeightBoundary : public Boundary
	{
	public:

		HeightBoundary(float top, float bottom, float minx, float maxx, int numSegments);
		virtual ~HeightBoundary();

		int getPointCount();
		float getHeight(int i);
		void setHeight(int i, float height);
		float getX(int i);

		inline float getMinX() { return mMinX; }
		inline float getMaxX() { return mMaxX; }
		inline float getBottom() { return mBottom; }

		virtual bool contains(const Vector2 &point);

		int getNearestPointIndex(float x);

		virtual Boundary *clone();

	private:

		float *mHeights;
		float *mXs;
		int mNumHeights;
		float mMinX;
		float mMaxX;
		float mBottom;

	};
}
