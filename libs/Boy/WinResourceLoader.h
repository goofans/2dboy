#pragma once

#include "BoyLib/CrtDbgInc.h"

#include "irrKlang.h"
#include "ResourceLoader.h"

namespace Boy
{
	class WinD3DInterface;

	class WinResourceLoader : public ResourceLoader
	{
	public:

		WinResourceLoader(const std::string &language1, const std::string &language2, WinD3DInterface *sdld3dInterface);
		virtual ~WinResourceLoader();

		virtual bool load(Image *image);
		virtual bool load(Sound *sound);
		virtual Image *createImage(const std::string &filename);
		virtual Sound *createSound(const std::string &filename);

	private:

		irrklang::ISoundSource *tryLoad(const std::string &filename);

	private:

		WinD3DInterface *mInterface;

	};
};
