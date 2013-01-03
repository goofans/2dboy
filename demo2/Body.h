#pragma once

#include "Boy/Environment.h"
#include "Boy/Game.h"
#include "Boy/Graphics.h"
#include "Boy/KeyboardListener.h"
#include "BoyLib/BoyUtil.h"
#include "BoyLib/Vector2.h"

enum BodyType
{
	SMALL_ASTEROID = 0,
	BIG_ASTEROID = 1,
	MEDIUM_ASTEROID = 2,
	SHIP = 3,
	BULLET = 4
};


class Body 
{
public:

	Body(BodyType type, float posx=0, float posy=0, float velx=0, float vely=0);

	bool collidesWith(Body *body);
	void draw(Boy::Graphics *g);
	void update(float dt);

private:
	void _draw(Boy::Graphics *g, BoyLib::Vector2 &pos);

public:
	Boy::Image *mImage;
	BoyLib::Vector2 mPos;
	BoyLib::Vector2 mVel;
	float mRot;
	float mRotVel;
	float mRadius;
	float mTimeOfDeath;

	BodyType mType;
};

