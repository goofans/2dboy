#include "WinImage.h"

#include <assert.h>
#include "ResourceLoader.h"

using namespace Boy;

#include "BoyLib/CrtDbgNew.h"

WinImage::WinImage(ResourceLoader *loader, const std::string &path) : Image(loader,path)
{
	mTexture = NULL;
	mWidth = -1;
	mHeight = -1;
}

WinImage::~WinImage()
{
	assert(mTexture == NULL);
}

int WinImage::getWidth()
{
	return mWidth;
}

int WinImage::getHeight()
{
	return mHeight;
}

bool WinImage::init(bool includeSounds)
{
	return mLoader->load(this);
}

void WinImage::destroy(bool includeSounds)
{
	if (mTexture!=NULL)
	{
		mTexture->Release();
		mTexture = NULL;
	}
}

void WinImage::setSize(int width, int height)
{
	mWidth = width;
	mHeight = height;
}

void WinImage::setTexture(IDirect3DTexture9 *tex, bool isScaled)
{
	mTexture = tex;
	mIsTextureScaled = isScaled;
}
