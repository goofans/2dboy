#pragma once

#include "BoyLib/Rect.h"
#include "d3d9.h"
#include "d3dx9.h"
#include "Graphics.h"
#include "TriStrip.h"

#include <stack>

namespace Boy
{
	class GraphicsState;
	class WinD3DInterface;

	class WinGraphics : public Graphics
	{
	public:

		WinGraphics(WinD3DInterface *platformInterface);
		virtual ~WinGraphics();

		virtual void drawImage(Image *img);
		virtual void drawImage(Image *img, int subrectX, int subrectY, int subrectW, int subrectH);

		virtual void drawLine(int x0, int y0, int x1, int y1);
		virtual void fillRect(int x0, int y0, int w, int h);
		virtual void drawTriStrip(TriStrip *strip);

		virtual void scale(float x, float y);
		virtual void rotateDeg(float angle);
		virtual void rotateRad(float angle);
		virtual void translate(float x, float y);
//		virtual void setIdentity();
		virtual void preScale(float x, float y);
		virtual void preRotateDeg(float angle);
		virtual void preRotateRad(float angle);
		virtual void preTranslate(float x, float y);

		virtual void pushTransform();
		virtual void popTransform();
		virtual int getTransformStackSize();

		virtual void setAlpha(float alpha);
		virtual void setColor(Color color);
		virtual void setColorizationEnabled(bool enabled);

		virtual void setZTestEnabled(bool enabled);
		virtual bool isZTestEnabled();
		virtual void setZWriteEnabled(bool enabled);
		virtual bool isZWriteEnabled();
		virtual void setZFunction(CompareFunc func);
		virtual void setZ(float z);
		virtual float getZ();

		virtual void setAlphaTestEnabled(bool enabled);
		virtual bool isAlphaTestEnabled();
		virtual void setAlphaReferenceValue(int val);
		virtual void setAlphaFunction(CompareFunc func);

		virtual void setDrawMode(DrawMode mode);

		virtual void setClipRect(int x, int y, int width, int height);

		virtual int getWidth();
		virtual int getHeight();

		virtual void setClearZ(float z);
		virtual void setClearColor(Color color);

		void dumpInfo(std::ofstream &file);

	private:

		void updateTransform();

	private:

		DWORD mColor;
		bool mColorizationEnabled;

		float mZ;

		std::stack<D3DXMATRIX> mTransformStack;
		bool mTransformUpToDate;

		WinD3DInterface *mInterface;

	};
};

