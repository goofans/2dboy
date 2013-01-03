#pragma once

#include "BoyLib/CrtDbgInc.h"

#include <string>
#include <vector>

namespace Boy
{
	class ResourceGroup
	{
	public:

		ResourceGroup();
		virtual ~ResourceGroup();

		bool isEmpty() { return mResourcePaths.size()==0; }
		void addResourcePath(const std::string &path);

		// path iteration:
		const std::string *getFirstPath();
		const std::string *getNextPath();

	private:

		// maps resouce ids to resource paths:
		std::vector<std::string> mResourcePaths;

		// iterator:
		std::vector<std::string>::iterator mPathIterator;
	};
}
