#pragma once

#include "BoyLib/CrtDbgInc.h"

#include <string>

namespace Boy
{
	class Image;
	class Sound;

	class ResourceLoader
	{
	public:
		
		ResourceLoader(const std::string &language1, const std::string &language2) { mLanguage1 = language1; mLanguage2 = language2; }
		virtual ~ResourceLoader() {}

		virtual bool load(Image *image) = 0;
		virtual bool load(Sound *sound) = 0;
		virtual Image *createImage(const std::string &filename) = 0;
		virtual Sound *createSound(const std::string &filename) = 0;

		std::string &getLanguage1() { return mLanguage1; }
		std::string &getLanguage2() { return mLanguage2; }

	protected:

		std::string mLanguage1;
		std::string mLanguage2;

	};
}

