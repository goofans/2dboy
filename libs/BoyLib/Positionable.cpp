#include "Positionable.h"

#include <assert.h>
#include "BoyUtil.h"

using namespace BoyLib;

#include "CrtDbgNew.h"

Positionable::Positionable() 
{ 
	mAnchor=NULL; 
	mAngle=0; 
	mIsRemapped = false;
	mInheritRotation = true;
	mPosRegisterDirty = true;
}

Positionable::Positionable(const Positionable &positionable) 
{ 
	mPos = positionable.mPos;
	mAngle = positionable.mAngle;
	mAnchor = positionable.mAnchor;
	mIsRemapped = false;
	mInheritRotation = positionable.mInheritRotation;
	mPosRegisterDirty = true;
}

Positionable::~Positionable() 
{
}

const Vector2 &Positionable::getPosition() 
{ 
	if (mAnchor==NULL)
	{
		return mPos;
	}
	else
	{
		if (mPosRegisterDirty)
		{
			mPosRegister = mAnchor->getPosition();

			if (mInheritRotation)
			{
				mPosRegister += rotate(mPos,mAnchor->getRotation());
			}
			else
			{
				mPosRegister += mPos;
			}

			mPosRegisterDirty = false;
		}

		return mPosRegister;
	}
}

float Positionable::getRotation() 
{ 
	if (mAnchor==NULL || !mInheritRotation)
	{
		return mAngle; 
	}
	else
	{
		return mAngle + mAnchor->getRotation();
	}
}

Cloneable *Positionable::clone(std::map<Cloneable*,Cloneable*> *cloneMap)
{
	Positionable *clone = new Positionable(*this);
	if (cloneMap!=NULL)
	{
		(*cloneMap)[this] = clone;
	}
	return clone;
}

void Positionable::remap(std::map<Cloneable*,Cloneable*> &cloneMap)
{
	assert(!mIsRemapped);
	mIsRemapped = true;

	if (mAnchor!=NULL)
	{
		Positionable *ac = dynamic_cast<Positionable*>(cloneMap[mAnchor]);
		assert(ac!=NULL);
		mAnchor = ac;
	}
}

void Positionable::setAnchor(Positionable *anchor, bool inheritRotation) 
{ 
	mAnchor = anchor; 
	mInheritRotation = inheritRotation;
}
	
void Positionable::setPosition(float x, float y) 
{ 
	mPos.x = x; 
	mPos.y = y; 
	mPosRegisterDirty = true;
}

void Positionable::setPosition(const Vector2 &pos) 
{ 
	mPos = pos; 
	mPosRegisterDirty = true;
}

void Positionable::addPosition(const Vector2 &pos) 
{ 
	mPos += pos; 
	mPosRegisterDirty = true;
}
