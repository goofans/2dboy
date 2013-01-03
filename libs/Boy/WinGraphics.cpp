#include "WinGraphics.h"

#include <assert.h>
#include "BoyLib/BoyUtil.h"
#include "WinImage.h"
#include "WinD3DInterface.h"
#include "WinTriStrip.h"

using namespace Boy;

#include "BoyLib/CrtDbgNew.h"

WinGraphics::WinGraphics(WinD3DInterface *platformInterface)
{
	mInterface = platformInterface;
	mTransformUpToDate = false;
	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);
	mTransformStack.push(identity);
//	mUseBilinearFiltering.push(false);
	mColor = 0xffffffff;
	mColorizationEnabled = false;
	mZ = 0;
}

WinGraphics::~WinGraphics()
{
	while (mTransformStack.size()>0)
	{
		mTransformStack.pop();
	}
}


void WinGraphics::updateTransform()
{
	if (!mTransformUpToDate)
	{
		mInterface->setTransform(mTransformStack.top());
		mTransformUpToDate = true;
	}
}

void WinGraphics::drawImage(Image *img)
{
	updateTransform();
	mInterface->drawImage(
		dynamic_cast<WinImage*>(img), 
		mColorizationEnabled ? mColor : 0xffffffff,
		mZ);
}

void WinGraphics::drawImage(Image *img, int subrectX, int subrectY, int subrectW, int subrectH)
{
	updateTransform();
	mInterface->drawImage(
		dynamic_cast<WinImage*>(img), 
		mColorizationEnabled ? mColor : 0xffffffff,
		mZ,
		subrectX,
		subrectY,
		subrectW,
		subrectH);
}

void WinGraphics::drawLine(int x0, int y0, int x1, int y1)
{
	mInterface->drawLine(x0,y0,x1,y1,mColor);
}

void WinGraphics::fillRect(int x0, int y0, int w, int h)
{
	pushTransform();
		D3DXMATRIX identity;
		D3DXMatrixIdentity(&identity);
		mInterface->setTransform(identity);
		mInterface->drawRect(x0,y0,w,h,mZ,mColor);
	popTransform();
}

void WinGraphics::scale(float x, float y)
{
	D3DXMATRIX xform;
	D3DXMatrixScaling(&xform, x, y, 1);
	mTransformStack.top() *= xform;
//	mUseBilinearFiltering.top() = true;
	mTransformUpToDate = false;
}

void WinGraphics::rotateDeg(float angle)
{
	D3DXMATRIX xform;
	D3DXMatrixRotationZ(&xform, deg2rad(-angle));
	mTransformStack.top() *= xform;
//	mUseBilinearFiltering.top() = true;
	mTransformUpToDate = false;
}

void WinGraphics::rotateRad(float angle)
{
	D3DXMATRIX xform;
	D3DXMatrixRotationZ(&xform, -angle);
	mTransformStack.top() *= xform;
//	mUseBilinearFiltering.top() = true;
	mTransformUpToDate = false;
}

void WinGraphics::translate(float x, float y)
{
	D3DXMATRIX xform;
	D3DXMatrixTranslation(&xform, x, y, 0);
	mTransformStack.top() *= xform;
	mTransformUpToDate = false;
}

void WinGraphics::preScale(float x, float y)
{
	D3DXMATRIX xform;
	D3DXMatrixScaling(&xform, x, y, 1);
	mTransformStack.top() = xform * mTransformStack.top();
//	mUseBilinearFiltering.top() = true;
	mTransformUpToDate = false;
}

void WinGraphics::preRotateDeg(float angle)
{
	D3DXMATRIX xform;
	D3DXMatrixRotationZ(&xform, deg2rad(angle));
	mTransformStack.top() = xform * mTransformStack.top();
//	mUseBilinearFiltering.top() = true;
	mTransformUpToDate = false;
}

void WinGraphics::preRotateRad(float angle)
{
	D3DXMATRIX xform;
	D3DXMatrixRotationZ(&xform, angle);
	mTransformStack.top() = xform * mTransformStack.top();
//	mUseBilinearFiltering.top() = true;
	mTransformUpToDate = false;
}

void WinGraphics::preTranslate(float x, float y)
{
	D3DXMATRIX xform;
	D3DXMatrixTranslation(&xform, x, y, 0);
	mTransformStack.top() = xform * mTransformStack.top();
	mTransformUpToDate = false;
}

void WinGraphics::pushTransform()
{
	mTransformStack.push(D3DXMATRIX(mTransformStack.top()));
//	mUseBilinearFiltering.push(mUseBilinearFiltering.top());
}

void WinGraphics::popTransform()
{
	mTransformStack.pop();
//	mUseBilinearFiltering.pop();
}

int WinGraphics::getTransformStackSize()
{
	return (int)mTransformStack.size();
}

void WinGraphics::setColor(Color color)
{
	mColor = color;
}

void WinGraphics::setAlpha(float alpha)
{
	mColor &= 0x00ffffff;
	mColor |= ((int)(255.0f * alpha)) << 24;
}

void WinGraphics::setColorizationEnabled(bool enabled)
{
	mColorizationEnabled = enabled;
}

void WinGraphics::setZTestEnabled(bool enabled)
{
	mInterface->setRenderState(D3DRS_ZENABLE, enabled ? D3DZB_TRUE : D3DZB_FALSE); 
}

bool WinGraphics::isZTestEnabled()
{
	return mInterface->getRenderState(D3DRS_ZENABLE) != 0;
}

void WinGraphics::setZWriteEnabled(bool enabled)
{
	mInterface->setRenderState(D3DRS_ZWRITEENABLE, enabled); 
}

bool WinGraphics::isZWriteEnabled()
{
	return mInterface->getRenderState(D3DRS_ZWRITEENABLE) != 0; 
}

void WinGraphics::setZ(float z)
{
	assert(z>=0 && z<=1);
	mZ = z;
}

float WinGraphics::getZ()
{
	return mZ;
}

void WinGraphics::setZFunction(CompareFunc func)
{
	D3DCMPFUNC zfunc;
	// TODO: this should be optimized... use a lookup array!
	switch(func)
	{
	case CMP_NEVER:
		zfunc = D3DCMP_NEVER;
		break;
	case CMP_LESS:
		zfunc = D3DCMP_LESS;
		break;
	case CMP_EQUAL:
		zfunc = D3DCMP_EQUAL;
		break;
	case CMP_LEQUAL:
		zfunc = D3DCMP_LESSEQUAL;
		break;
	case CMP_GREATER:
		zfunc = D3DCMP_GREATER;
		break;
	case CMP_NOTEQUAL:
		zfunc = D3DCMP_NOTEQUAL;
		break;
	case CMP_GEQUAL:
		zfunc = D3DCMP_GREATEREQUAL;
		break;
	case CMP_ALWAYS:
		zfunc = D3DCMP_ALWAYS;
		break;
	default:
		assert(false);
	}

	mInterface->setRenderState(D3DRS_ZFUNC, zfunc);
}

void WinGraphics::setDrawMode(DrawMode mode)
{
	switch (mode)
	{
	case DRAWMODE_NORMAL:
		mInterface->setRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
		mInterface->setRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
		break;
	case DRAWMODE_ADDITIVE:
		mInterface->setRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
		mInterface->setRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE);
		break;
	}
}

void WinGraphics::setAlphaTestEnabled(bool enabled)
{
	mInterface->setRenderState(D3DRS_ALPHATESTENABLE, enabled);
}

bool WinGraphics::isAlphaTestEnabled()
{
	return mInterface->getRenderState(D3DRS_ALPHATESTENABLE) != 0; 
}

void WinGraphics::setAlphaReferenceValue(int val)
{
	mInterface->setRenderState(D3DRS_ALPHAREF, val);
}

void WinGraphics::setAlphaFunction(CompareFunc func)
{
	D3DCMPFUNC afunc;
	// TODO: this should be optimized... use a lookup array!
	switch(func)
	{
	case CMP_NEVER:
		afunc = D3DCMP_NEVER;
		break;
	case CMP_LESS:
		afunc = D3DCMP_LESS;
		break;
	case CMP_EQUAL:
		afunc = D3DCMP_EQUAL;
		break;
	case CMP_LEQUAL:
		afunc = D3DCMP_LESSEQUAL;
		break;
	case CMP_GREATER:
		afunc = D3DCMP_GREATER;
		break;
	case CMP_NOTEQUAL:
		afunc = D3DCMP_NOTEQUAL;
		break;
	case CMP_GEQUAL:
		afunc = D3DCMP_GREATEREQUAL;
		break;
	case CMP_ALWAYS:
		afunc = D3DCMP_ALWAYS;
		break;
	default:
		assert(false);
	}

	mInterface->setRenderState(D3DRS_ALPHAFUNC, afunc);
}

void WinGraphics::setClipRect(int x, int y, int width, int height)
{
	mInterface->setClipRect(x,y,width,height);
}

void WinGraphics::drawTriStrip(TriStrip *strip)
{
	WinTriStrip *s = dynamic_cast<WinTriStrip*>(strip);
	pushTransform();
		D3DXMATRIX identity;
		D3DXMatrixIdentity(&identity);
		mInterface->setTransform(identity);
		mInterface->drawTriStrip(s);
	popTransform();
}

int WinGraphics::getWidth()
{
	return mInterface->getWidth();
}

int WinGraphics::getHeight()
{
	return mInterface->getHeight();
}

void WinGraphics::setClearZ(float z)
{
	mInterface->setClearZ(z);
}

void WinGraphics::setClearColor(Color color)
{
	mInterface->setClearColor(color);
}

void WinGraphics::dumpInfo(std::ofstream &file)
{
	mInterface->dumpInfo(file);
}
