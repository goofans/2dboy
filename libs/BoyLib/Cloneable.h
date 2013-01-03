#pragma once

#include "CrtDbgInc.h"

#include <map>

namespace BoyLib
{
	/*
	 * an interface for cloneable objects
	 */
	class Cloneable
	{
	public:
		Cloneable() {}
		virtual ~Cloneable() {}

		// clone the object, adding a mapping from the original to the clone into 
		// cloneMap.  the cloning operation should add a mapping from the cloned
		// object to its clone into cloneMap.  cloneMap should not be used for 
		// anything else
		virtual Cloneable *clone(std::map<Cloneable*,Cloneable*> *cloneMap) = 0;

		// this method helps in the process of "deep cloning".  when a set of objects
		// is cloned and they contain pointers to one another, first, all the objects
		// should be cloned and then each cloned object should have its remap method
		// called so that it can map its pointers to point to the clones instead of
		// objects in the original set
		virtual void remap(std::map<Cloneable*,Cloneable*> &cloneMap) = 0;
	};
}
