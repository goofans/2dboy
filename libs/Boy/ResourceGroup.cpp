#include "ResourceGroup.h"

#include <algorithm>
#include <assert.h>

using namespace Boy;

#include "BoyLib/CrtDbgNew.h"

ResourceGroup::ResourceGroup()
{
	mPathIterator = mResourcePaths.end();
}

ResourceGroup::~ResourceGroup()
{
	mResourcePaths.clear();
}

void ResourceGroup::addResourcePath(const std::string &path)
{
	// if this path is NOT already in the group:
	if (find(mResourcePaths.begin(),mResourcePaths.end(),path) == mResourcePaths.end())
	{
		// add it:
		mResourcePaths.push_back(path);
	}
}

const std::string *ResourceGroup::getFirstPath()
{
	if (mResourcePaths.size()==0)
	{
		return NULL;
	}

	mPathIterator = mResourcePaths.begin();
	std::string *path = &(*mPathIterator);

	return path;
}

const std::string *ResourceGroup::getNextPath()
{
	mPathIterator++;
	if (mPathIterator == mResourcePaths.end())
	{
		return NULL;
	}
	return &(*mPathIterator);
}

