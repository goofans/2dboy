#pragma once

#include "Image.h"
#include "d3d9.h"

namespace Boy
{
	class WinImage : public Image
	{
	public:

		WinImage(ResourceLoader *loader, const std::string &path);
		virtual ~WinImage();

		void setSize(int width, int height);

		const std::string &getPath() { return mPath; }

		void setTexture(IDirect3DTexture9 *tex, bool isScaled);

		inline IDirect3DTexture9 *getTexture() { return mTexture; }

		inline bool isTextureScaled() { return mIsTextureScaled; }

		// implementation of Image:
		virtual int getWidth();
		virtual int getHeight();

	private:

		// implementation of Resource:
		virtual bool init(bool includeSounds);
		virtual void destroy(bool includeSounds);

	private:

		IDirect3DTexture9 *mTexture;

		int mWidth;
		int mHeight;

		bool mIsTextureScaled;
	};
};
