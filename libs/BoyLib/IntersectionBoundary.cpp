#include "IntersectionBoundary.h"

#include <assert.h>

using namespace BoyLib;

#include "CrtDbgNew.h"

IntersectionBoundary::IntersectionBoundary(int maxBoundaries)
{
	assert(maxBoundaries>0);
	mMaxBoundaries = maxBoundaries;
	mBoundaries = new Boundary*[maxBoundaries];
	mNumBoundaries = 0;
}

IntersectionBoundary::~IntersectionBoundary()
{
	for (int i=0 ; i<mNumBoundaries ; i++)
	{
		delete mBoundaries[i];
	}

	delete[] mBoundaries;
}

bool IntersectionBoundary::contains(const Vector2 &point)
{
	assert(mNumBoundaries>0);

	for (int i=0 ; i<mNumBoundaries ; i++)
	{
		if (!mBoundaries[i]->contains(point))
		{
			return false;
		}
	}

	return true;
}

Boundary *IntersectionBoundary::clone()
{
	IntersectionBoundary *clone = new IntersectionBoundary(mMaxBoundaries);

	clone->mNumBoundaries = mNumBoundaries;

	for (int i=0 ; i<mNumBoundaries ; i++)
	{
		clone->mBoundaries[i] = mBoundaries[i]->clone();
	}

	return clone;
}


