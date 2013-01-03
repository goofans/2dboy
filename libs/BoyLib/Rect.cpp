#include "Rect.h"

#include "BoyUtil.h"

using namespace BoyLib;

#include "CrtDbgNew.h"

Rect::Rect(float x, float y, float width, float height, bool isCentered)
{
	set(x,y,width,height,isCentered);
}

Rect::~Rect()
{
}

void Rect::set(float x, float y, float width, float height, bool isCentered)
{
	if (isCentered)
	{
		mMinX = x - width/2;
		mMaxX = x + width/2;
		mMinY = y - height/2;
		mMaxY = y + height/2;
	}
	else
	{
		mMinX = x;
		mMaxX = x + width;
		mMinY = y;
		mMaxY = y + height;
	}
}

bool Rect::contains(const Vector2 &point)
{
	return 
		point.x>=mMinX && point.x<=mMaxX &&
		point.y>=mMinY && point.y<=mMaxY;
}

Boundary *Rect::clone()
{
	Rect *clone = new Rect(0,0,0,0,false);
	
	clone->mMinX = mMinX;
	clone->mMinY = mMinY;
	clone->mMaxX = mMaxX;
	clone->mMaxY = mMaxY;

	return clone;
}

Rect &Rect::operator = (const Rect &rect)
{
	mMinX = rect.mMinX;
	mMaxX = rect.mMaxX;
	mMinY = rect.mMinY;
	mMaxY = rect.mMaxY;

	return *this;
}

Rect Rect::intersect(const Rect &rect)
{
	Rect intersection;

	intersection.mMinX = std::max(mMinX, rect.mMinX);
	intersection.mMinY = std::max(mMinY, rect.mMinY);
	intersection.mMaxX = std::min(mMaxX, rect.mMaxX);
	intersection.mMaxY = std::min(mMaxY, rect.mMaxY);

	// if the rects don't intersect:
	if (intersection.mMinX > intersection.mMaxX || 
		intersection.mMinY > intersection.mMaxY)
	{
		// reset the rect to a null rect:
		intersection.mMinX = 0;
		intersection.mMaxX = 0;
		intersection.mMinY = 0;
		intersection.mMaxY = 0;
	}

	return intersection;
}

Rect Rect::Union(const Rect &rect)
{
	Rect Union;

	Union.mMinX = std::min(mMinX, rect.mMinX);
	Union.mMinY = std::min(mMinY, rect.mMinY);
	Union.mMaxX = std::max(mMaxX, rect.mMaxX);
	Union.mMaxY = std::max(mMaxY, rect.mMaxY);

	return Union;
}

bool Rect::intersects(const Rect &rect)
{
	float minx = std::max(mMinX, rect.mMinX);
	float miny = std::max(mMinY, rect.mMinY);
	float maxx = std::min(mMaxX, rect.mMaxX);
	float maxy = std::min(mMaxY, rect.mMaxY);

	return minx<maxx && miny<maxy;
}

