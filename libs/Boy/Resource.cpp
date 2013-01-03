#include "Resource.h"

#include <assert.h>

using namespace Boy;

#include "BoyLib/CrtDbgNew.h"

Resource::Resource(ResourceLoader *loader, const std::string &path)
{
	mLoader = loader;
	mPath = path;
	mRefCount = 0;
}

Resource::~Resource()
{
}

bool Resource::load()
{
	bool success = true;
	if (mRefCount==0)
	{
		success = init(true);
	}

	mRefCount++;

	return success;
}

void Resource::reload()
{
	if (mRefCount!=0)
	{
		destroy(true);
		init(true);
	}
}

void Resource::release()
{
	if (mRefCount==0)
	{
		return;
	}

	assert(mRefCount>0);

	mRefCount--;

	if (mRefCount==0)
	{
		destroy(true);
	}
}

bool Resource::isLoaded()
{
	return mRefCount>0;
}


