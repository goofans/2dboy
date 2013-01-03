#pragma once

#include "CrtDbgInc.h"

#include <assert.h>
#include <istream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include "Vector2.h"

#define GPI (3.14159265358979323846f)

#ifndef NULL
#define NULL 0
#endif

inline float rad2deg(float rad) 
{
	return rad * 180.0f / GPI;
}

inline float deg2rad(float deg) 
{
	return deg * GPI / 180.0f;
}

inline int goo_roundf(float d) 
{ 
	return static_cast<int>( d + (d > 0.0 ? 0.5 : -0.5) );
}

inline int roundToNearestMultipleOf(float d, float n)
{
	float normalized = d / n;
	float normalizedRounded = (float)goo_roundf(normalized);
	return (int)(normalizedRounded * n);
}

inline float dist(float x0, float y0, float x1, float y1) 
{
	float dx = x1-x0;
	float dy = y1-y0;
	return boy_sqrtf(dx*dx+dy*dy);
}

inline float dist(const BoyLib::Vector2 &p0, const BoyLib::Vector2 &p1) 
{
	float dx = p0.x-p1.x;
	float dy = p0.y-p1.y;
	return boy_sqrtf(dx*dx+dy*dy);
}

inline float dist(float x, float y, const BoyLib::Vector2 &p) 
{
	float dx = p.x-x;
	float dy = p.y-y;
	return boy_sqrtf(dx*dx+dy*dy);
}

// shortest distance from point p to line through points l0 and l1
// taken from: http://mathworld.wolfram.com/Point-LineDistance2-Dimensional.html
inline float distPointToLine(const BoyLib::Vector2 &p, const BoyLib::Vector2 &l0, const BoyLib::Vector2 &l1) 
{
	float ldx = l1.x-l0.x; // dx for line
	float ldy = l1.y-l0.y; // dy for line
	return fabs( ldx * (l0.y - p.y) - (l0.x - p.x) * ldy ) / boy_sqrtf( ldx * ldx + ldy * ldy );
}

inline float distPointToLine(float px, float py, 
							 float l0x, float l0y,
							 float l1x, float l1y) 
{
	float ldx = l1x-l0x; // dx for line
	float ldy = l1y-l0y; // dy for line
	return fabs( ldx * (l0y - py) - (l0x - px) * ldy ) / boy_sqrtf( ldx * ldx + ldy * ldy );
}

inline float randf(float min, float max)
{
	assert(max>=min);
	return min + ((float)rand()/RAND_MAX) * (max-min);
}

inline int randi(int min, int max)
{
	assert(max>=min);
	return min + (rand()%(max-min+1));
}

// reflects a point p off a line l that passes through the
// two points l0 and l1.
inline BoyLib::Vector2 reflect(const BoyLib::Vector2 &p, const BoyLib::Vector2 &l0, const BoyLib::Vector2 &l1)
{
	// calculate the normal along the line:
	BoyLib::Vector2 n = (l1-l0);
	n /= n.magnitude();

	return l0*2 - p + n*2*(p-l0).dot(n);
}

inline float getAngleBetween(const BoyLib::Vector2 &v0, const BoyLib::Vector2& v1)
{
	return acos(v0.dot(v1) / (v0.magnitude()*v1.magnitude()));
}

// this function rotates a point p1 around the point p0.
// based on http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/BASICMAT/node4.html
inline BoyLib::Vector2 rotate(const BoyLib::Vector2 &p1, const BoyLib::Vector2 &p0, float angle)
{
	BoyLib::Vector2 d = p1-p0;

	return p0 + BoyLib::Vector2(
		d.x * cos(angle) - d.y * sin(angle), 
		d.x * sin(angle) + d.y * cos(angle));
}

// this function rotates a point p around the point origin.
// based on http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/BASICMAT/node4.html
inline BoyLib::Vector2 rotate(const BoyLib::Vector2 &p, float angle)
{
	return BoyLib::Vector2(
		p.x * cos(angle) - p.y * sin(angle), 
		p.x * sin(angle) + p.y * cos(angle));
}

inline bool getIntersectPoint(
			float x1, float y1,
			float x2, float y2,
			float x3, float y3,
			float x4, float y4,
			BoyLib::Vector2 &intersectPoint,
			bool allowOffSegment=false)
{
	// let's find the intersection of lines L12 and L34:
	float a1 = y2-y1;
	float b1 = x1-x2;
	float c1 = x2*y1 - x1*y2;
	float a2 = y4-y3;
	float b2 = x3-x4;
	float c2 = x4*y3 - x3*y4;
	float denom = a1*b2 - a2*b1;
	if (denom==0)
	{
		// these lines never intersect
		return false;
	}
	float x = (b1*c2 - b2*c1) / denom;
	float y = (a2*c1 - a1*c2) / denom;

	// check if this intersection in within this specific segment
	float d0 = dist(x3,y3,x4,y4);
	float d1 = dist(x,y,x3,y3);
	float d2 = dist(x,y,x4,y4);
	bool onSegment = (d0>=d1 && d0>=d2);

	if (onSegment || allowOffSegment)
	{
		intersectPoint.x = x;
		intersectPoint.y = y;
	}

	return onSegment;
}

inline BoyLib::Vector2 closestPointOnLine(
	const BoyLib::Vector2 &lp0,
	const BoyLib::Vector2 &lp1,
	const BoyLib::Vector2 &p)
{

	float dx = lp1.x - lp0.x;
	float dy = lp1.y - lp0.y;

	float perpdx = dy;
	float perpdy = -dx;

	BoyLib::Vector2 closestPoint;
	getIntersectPoint(
		lp0.x, lp0.y,
		lp1.x, lp1.y,
		p.x - 10*perpdx, p.y - 10*perpdy,
		p.x + 10*perpdx, p.y + 10*perpdy,
		closestPoint,
		false);

	return closestPoint;
}

inline float interpolatef(float v0, float v1, float t0, float t1, float t)
{
	assert(t1>t0);

	return v0 + (v1-v0) * (t-t0)/(t1-t0);
}

inline int interpolatei(int v0, int v1, float t0, float t1, float t)
{
	assert(t1>t0);
	assert(t>=t0);
	assert(t<=t1);

	return (int)(v0 + (v1-v0) * (t-t0)/(t1-t0));
}

inline int interpolateColor(int c0, int c1, float t0, float t1, float t)
{
	assert(t1>t0);
	assert(t>=t0);
	assert(t<=t1);

	int r0 = c0 & 0xff0000;
	int g0 = c0 & 0x00ff00;
	int b0 = c0 & 0x0000ff;

	int r1 = c1 & 0xff0000;
	int g1 = c1 & 0x00ff00;
	int b1 = c1 & 0x0000ff;

	return 
		(interpolatei(r0,r1,t0,t1,t) & 0xff0000) |
		(interpolatei(g0,g1,t0,t1,t) & 0x00ff00) |
		(interpolatei(b0,b1,t0,t1,t) & 0x0000ff);
}

/*
 * based on code from http://local.wasp.uwa.edu.au/~pbourke/geometry/2circle/
 */
inline bool intersectCircles(float x0, float y0, float r0,
					  float x1, float y1, float r1,
                      float *xi0, float *yi0,
                      float *xi1, float *yi1)
{
	float a, dx, dy, d, h, rx, ry;
	float x2, y2;

	// dx and dy are the vertical and horizontal distances between the circle centers:
	dx = x1 - x0;
	dy = y1 - y0;

	// Determine the straight-line distance between the centers:
	d = boy_sqrtf(dx*dx+dy*dy);

	// Check for solvability
	if (d > (r0 + r1))
	{
		// no solution, circles do not intersect:
		return false;
	}

	if (d < fabs(r0 - r1))
	{
		// no solution, one circle is contained in the other:
		return 0;
	}

	// 'point 2' is the point where the line through the circle intersection 
	// points crosses the line between the circle centers.

	// Determine the distance from point 0 to point 2:
	a = ((r0*r0) - (r1*r1) + (d*d)) / (2.0f * d) ;

	// Determine the coordinates of point 2:
	x2 = x0 + (dx * a/d);
	y2 = y0 + (dy * a/d);

	// Determine the distance from point 2 to either of the intersection points:
	h = boy_sqrtf((r0*r0) - (a*a));

	// Now determine the offsets of the intersection points from point 2:
	rx = -dy * (h/d);
	ry = dx * (h/d);

	// Determine the absolute intersection points:
	*xi0 = x2 + rx;
	*xi1 = x2 - rx;
	*yi0 = y2 + ry;
	*yi1 = y2 - ry;

	return true;
}


inline std::string Trim(const std::string& theString)
{
	unsigned long aStartPos = 0;
	while ( aStartPos < (unsigned long)theString.length() && isspace(theString[aStartPos]) )
		aStartPos++;

	unsigned long anEndPos = (unsigned long)theString.length() - 1;
	while ( (anEndPos >= 0) && (anEndPos < (unsigned long)theString.length()) && isspace(theString[anEndPos]) )
		anEndPos--;

	return theString.substr(aStartPos, anEndPos - aStartPos + 1);
}

inline char skipUntil(std::istream &is, const std::string &delimChars)
{
	bool done = false;

	std::istream::traits_type::int_type c = 0;
	while (true)
	{
		// get the next char
		c = is.get();

		// if c is a delimiter:
		if (delimChars.find( std::istream::traits_type::to_char_type(c) ) != std::string::npos)
		{
			break;
		}

		// if we've reached the end of the stream:
		if (is.eof())
		{
			c = 0;
			break;
		}
	}

	return std::istream::traits_type::to_char_type(c);
}

inline void skipUntil(std::istream &is, int delimChar)
{
	bool done = false;

	int c = 0;
	while (true)
	{
		// get the next char
		c = is.get();

		// if we found it:
		if (c==delimChar)
		{
			break;
		}

		// if we've reached the end of the stream:
		if (is.eof())
		{
			c = 0;
			break;
		}
	}
}

inline void skip(std::istream &is, int skipChar)
{
	bool done = false;

	int c = 0;
	while (true)
	{
		// get the next char
		c = is.get();

		// if we've reached the end of the stream:
		if (is.eof())
		{
			c = 0;
			break;
		}

		// if we're not supposed to skip this char:
		if (c!=skipChar)
		{
			break;
		}
	}
}

inline char readUntil(std::istream &is, std::string &buffer, const char *delimChars, bool respectEscapeChar=false)
{
	buffer.clear();
	bool done = false;
	std::istream::traits_type::int_type c = 0;
	bool escaping = false;

	while (true)
	{
		// get next char:
		c = is.get();

		// if c is a delimiter:
		if (!escaping && strchr(delimChars,c) != NULL)
		{
			break;
		}

		// if we've reached the end of the stream:
		if (is.eof())
		{
			break;
		}

		// if c is the escape char and we respect it and we're not in an escape sequence:
		if (c=='\\' && respectEscapeChar && !escaping)
		{
			escaping = true;
		}
		else
		{
			buffer += std::istream::traits_type::to_char_type(c);
			escaping = false;			
		}
	}

	return std::istream::traits_type::to_char_type(c);
}

inline unsigned int findNextHigherPowerOf2(unsigned int num)
{
	unsigned int i = 1;

	while (i<num)
	{
		i *= 2;
	}
	
	return i;
}

inline unsigned int findHighestPowerOf2SmallerThan(unsigned int num)
{
	unsigned int i = 1;

	while (i*2<num)
	{
		i *= 2;
	}
	
	return i;
}

