#include "Body.h"

#include "Boy/Environment.h"
#include "Boy/Graphics.h"
#include "Boy/ResourceManager.h"

#define BIG_ASTEROID_SPEED 40.0f
#define MEDIUM_ASTEROID_SPEED 100.0f
#define SMALL_ASTEROID_SPEED 150.0f
#define ASTEROID_ROTSPEED 50.0f
#define BULLET_DURATION 3.0f

Body::Body(BodyType type, float posx, float posy, float velx, float vely)
{
	mType = type;
	mPos.x = posx;
	mPos.y = posy;
	mVel.x = velx;
	mVel.y = vely;
	mRotVel = 0;
	mRot = 0;
	mTimeOfDeath = 0;

	int w = Boy::Environment::screenWidth();
	int h = Boy::Environment::screenHeight();

	switch (mType)
	{
	case BIG_ASTEROID:
		mImage = Boy::Environment::getImage(
			Boy::UString::format("IMAGE_ASTEROID_BIG_%d",randi(1,3)).toUtf8());
		mRadius = 100;
		do
		{
			mPos.x = (float)randi(0,w);
			mPos.y = (float)randi(0,h);
		}
		while (dist(mPos.x,mPos.y,w/2.0f,h/2.0f) < mRadius+100);
		mVel = rotate(BoyLib::Vector2(BIG_ASTEROID_SPEED, 0), randf(0,6.28f));
		mRotVel = randf(-ASTEROID_ROTSPEED,ASTEROID_ROTSPEED);
		mRot = randf(0,6.28f);
		break;
	case MEDIUM_ASTEROID:
		mImage = Boy::Environment::getImage(
			Boy::UString::format("IMAGE_ASTEROID_MEDIUM_%d",randi(1,3)).toUtf8());
		mRadius = 50;
		mVel = rotate(BoyLib::Vector2(MEDIUM_ASTEROID_SPEED, 0), randf(0,6.28f));
		mRotVel = randf(-ASTEROID_ROTSPEED,ASTEROID_ROTSPEED);
		mRot = randf(0,6.28f);
		break;
	case SMALL_ASTEROID:
		mImage = Boy::Environment::getImage(
			Boy::UString::format("IMAGE_ASTEROID_SMALL_%d",randi(1,3)).toUtf8());
		mRadius = 25;
		mVel = rotate(BoyLib::Vector2(SMALL_ASTEROID_SPEED, 0), randf(0,6.28f));
		mRotVel = randf(-ASTEROID_ROTSPEED,ASTEROID_ROTSPEED);
		mRot = randf(0,6.28f);
		break;
	case SHIP:
		mImage = Boy::Environment::getImage("IMAGE_SHIP");
		mRadius = 20;
		mPos.x = w/2.0f;
		mPos.y = h/2.0f;
		break;
	case BULLET:
		mImage = Boy::Environment::getImage("IMAGE_BULLET");
		mRadius = 1;
		mTimeOfDeath = Boy::Environment::instance()->getTime() + BULLET_DURATION;
		break;
	}
}

bool Body::collidesWith(Body *body)
{
	return dist(body->mPos.x,body->mPos.y,mPos.x,mPos.y) < body->mRadius+mRadius;
}

void Body::draw(Boy::Graphics *g)
{
	int w = Boy::Environment::screenWidth();
	int h = Boy::Environment::screenHeight();
	BoyLib::Vector2 pos = mPos;

	g->setDrawMode(Boy::Graphics::DRAWMODE_ADDITIVE);
	_draw(g,pos); // center
	pos.x += w;
	_draw(g,pos); // right
	pos.y += h; 
	_draw(g,pos); // bottom right
	pos.x -= w; 
	_draw(g,pos); // bottom
	pos.x -= w; 
	_draw(g,pos); // bottom left
	pos.y -= h; 
	_draw(g,pos); // left
	pos.y -= h; 
	_draw(g,pos); // top left
	pos.x += w; 
	_draw(g,pos); // top
	pos.x += w; 
	_draw(g,pos); // top right
	g->setDrawMode(Boy::Graphics::DRAWMODE_NORMAL);
}

void Body::_draw(Boy::Graphics *g, BoyLib::Vector2 &pos)
{
	if (mImage==NULL)
	{
		return;
	}
	g->pushTransform();
		g->rotateDeg(mRot);
		g->translate(pos.x,pos.y);
		g->drawImage(mImage);
	g->popTransform();
}

void Body::update(float dt)
{
	mRot += mRotVel*dt;
	mPos += mVel*dt;

	// make sure we're in bounds:
	int w = Boy::Environment::screenWidth();
	int h = Boy::Environment::screenHeight();
	bool killBullet = false;
	if (mPos.x > w)
	{
		killBullet = true;
		mPos.x -= w;
	}
	else if (mPos.x < 0)
	{
		killBullet = true;
		mPos.x += w;
	}
	if (mPos.y > h)
	{
		killBullet = true;
		mPos.y -= h;
	}
	else if (mPos.y < 0)
	{
		killBullet = true;
		mPos.y += h;
	}

	if (mType==BULLET && killBullet)
	{
		mTimeOfDeath = Boy::Environment::instance()->getTime() + 0.2f;
		mImage = NULL;
	}
}
