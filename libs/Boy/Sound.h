#pragma once

#include "BoyLib/CrtDbgInc.h"
#include "Resource.h"

namespace Boy
{
	class Sound : public Resource
	{
	public:
		Sound(ResourceLoader *loader, const std::string &path) : Resource(loader, path) {}
		virtual ~Sound() {}
	};
}

