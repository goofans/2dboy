#include "HeightBoundary.h"

#include "BoyUtil.h"

using namespace BoyLib;

#include "CrtDbgNew.h"

HeightBoundary::HeightBoundary(float top, float bottom, float minx, float maxx, int numSegments)
{
//	printf("  >>> creating HeightBoundary %p\n",this);
	mNumHeights = numSegments + 1;

	float segLen = (maxx-minx) / numSegments;

	mHeights = new float[mNumHeights];
	mXs = new float[mNumHeights];
	for (int i=0 ; i<mNumHeights ; i++)
	{
		mHeights[i] = top;
		mXs[i] = minx + segLen * i;
	}

	mMinX = minx;
	mMaxX = maxx;

	mBottom = bottom;
}

HeightBoundary::~HeightBoundary()
{
//	printf("  >>> deleting HeightBoundary %p\n",this);
	delete[] mHeights;
	delete[] mXs;
}

int HeightBoundary::getPointCount()
{
	return mNumHeights;
}

float HeightBoundary::getX(int i)
{
	return mXs[i];
}

float HeightBoundary::getHeight(int i)
{
	assert(i>=0 && i<mNumHeights);
	return mHeights[i];
}

void HeightBoundary::setHeight(int i, float height)
{
	assert(i>=0 && i<mNumHeights);
	mHeights[i] = height;
}

bool HeightBoundary::contains(const Vector2 &point)
{
	// if the point is out of bounds horizontally:
	if (point.x<mMinX || point.x>mMaxX)
	{
		// this boundary doesn't contain the point:
		return false;
	}

	// figure out which segment this point lines up with:
	int segment = (int)((point.x-mMinX) / (mMaxX-mMinX) * (mNumHeights-1));
	float y0 = mHeights[segment];
	float y1 = mHeights[segment+1];

	// if it's below both points:
	if (point.y<=y0 && point.y<=y1)
	{
		// if it's above the bottom:
		if (point.y>mBottom)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	// if it's above both points:
	else if (point.y>=y0 && point.y>=y1)
	{
		return false;
	}
	else
	{
		return point.y < (y0+y1)/2;
	}
	/*
	else
	{
		// it's in the axis aligned rectangle defined by the two height points, 
		// so let's figure out whether it's below or above the line connecting 
		// the two height points.

		float segmentLen = (mMaxX-mMinX)/(mNumHeights-1);
		float x0 = segmentLen * segment;
		float x1 = segmentLen * (segment+1);

		float a1 = y1-y0;
		float b1 = x0-x1;
		float c1 = x1*y0 - x0*y1;
		float a2 = a1;
		float c2 = point.x*y0 - point.x*y1;
		float denom = -a1*b1;
		assert(denom!=0); // make sure these lines intersect

		// and the y value of the intersect is...
		float y = a1 * (c1 - c2) / denom;

		// if the point is below the line, it's contained in the height boundary:
		return (point.y<=y);
	}
	*/
}

int HeightBoundary::getNearestPointIndex(float x)
{
	int index = (int)((x-mMinX) / (mMaxX-mMinX) * (mNumHeights-1));
	if( index < 0 ) index = 0;
	if( index >= mNumHeights ) index = mNumHeights - 1;
	return index;}

Boundary *HeightBoundary::clone()
{
	HeightBoundary *clone = new HeightBoundary(1,0,0,1,1);

	// copy simple data:
	clone->mNumHeights = mNumHeights;
	clone->mMinX = mMinX;
	clone->mMaxX = mMaxX;
	clone->mBottom = mBottom;

	// reallocate arrays:
	delete[] clone->mHeights;
	delete[] clone->mXs;
	clone->mHeights = new float[mNumHeights];
	clone->mXs = new float[mNumHeights];

	// copy data:
	for (int i=0 ; i<mNumHeights ; i++)
	{
		clone->mHeights[i] = mHeights[i];
		clone->mXs[i] = mXs[i];
	}

	return clone;
}
