#pragma once

#include <string>

namespace Boy
{
	class ResourceLoader;

	class Resource
	{
	public:

		Resource(ResourceLoader *loader, const std::string &path);
		virtual ~Resource();

		virtual bool load();
		virtual void reload();
		virtual void release();

		bool isLoaded();

//	protected:

		virtual bool init(bool includeSounds) = 0;
		virtual void destroy(bool includeSounds) = 0;

	protected:

		ResourceLoader *mLoader;
		std::string mPath;

	private:

		int mRefCount;

	};
}
