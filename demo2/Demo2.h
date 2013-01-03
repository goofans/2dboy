#pragma once

#include "Body.h"
#include "Boy/Environment.h"
#include "Boy/Font.h"
#include "Boy/Game.h"
#include "Boy/Graphics.h"
#include "Boy/KeyboardListener.h"
#include "BoyLib/BoyUtil.h"
#include "BoyLib/Vector2.h"
#include "Explosion.h"

class Demo2: public Boy::Game, public Boy::KeyboardListener
{
public:

	Demo2();
	virtual ~Demo2();

	// singleton:
	static Demo2 *instance();
	static void destroy();

	// implementation of Game:
	virtual void init();
	virtual void load();
	virtual void loadComplete();
	virtual void update(float dt);
	virtual void draw(Boy::Graphics *g);

	// implementation of KeyboardListener:
	virtual void keyUp(wchar_t unicode, Boy::Keyboard::Key key, Boy::Keyboard::Modifiers mods);
	virtual void keyDown(wchar_t unicode, Boy::Keyboard::Key key, Boy::Keyboard::Modifiers mods);

private:

	void death();
	void gameOver();
	void newGame();
	void nextLevel();

private:

	static Demo2 *gInstance;

	Body *mShip;
	std::vector<Body*> mAsteroids;
	std::vector<Body*> mBullets;
	std::vector<Explosion*> mExplosions;

	bool mThrust;
	bool mLeft;
	bool mRight;
	bool mGunArmed;

	int mLevel;
	int mLives;
	bool mLoadComplete;
	Boy::Image *mShipImage;
	Boy::Image *mThrustImage;
	Boy::Sound *mBoomSound;
	Boy::Sound *mFireSound;
	Boy::Sound *mThrustSound;
	Boy::Font *mFont;
	int mScore;
	bool mGameOver;
	float mLastRespawn;
	float mTimeOfDeath;
	int mDrawCount;
};
