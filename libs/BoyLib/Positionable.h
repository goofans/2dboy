#pragma once

#include "Cloneable.h"
#include "Vector2.h"

namespace BoyLib
{
	class Positionable : public Cloneable
	{
	public:

		Positionable();
		Positionable(const Positionable &positionable);
		virtual ~Positionable();

		virtual const Vector2 &getPosition();
		virtual void setPosition(float x, float y);
		virtual void setPosition(const Vector2 &pos);
		virtual void addPosition(const Vector2 &pos);

		virtual float getRotation();
		virtual void setRotation(float angle) { mAngle = angle; }

		void setAnchor(Positionable *anchor, bool inheritRotation=true);
		inline Positionable *getAnchor() { return mAnchor; }

		// implementation of Cloneable:
		virtual Cloneable *clone(std::map<Cloneable*,Cloneable*> *cloneMap);
		virtual void remap(std::map<Cloneable*,Cloneable*> &cloneMap);

		void setPosRegisterDirty() { mPosRegisterDirty = true; } 

	protected:

		bool mPosRegisterDirty;

	private:

		// position / orientation:
		Vector2 mPos;
		float mAngle;

		// anchoring:
		Positionable *mAnchor;
		Vector2 mPosRegister;
		bool mInheritRotation;

		// TODO: remove, this is for debugging only:
		bool mIsRemapped;

	};
}
