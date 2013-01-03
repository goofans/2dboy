#pragma once

#include "BoyLib/CrtDbgInc.h"

#include "BoyLib/Rect.h"

namespace Boy
{
	typedef unsigned long Color;

	class Image;
	class TriStrip;

	class Graphics
	{
	public:

		enum CompareFunc
		{
			CMP_NEVER,
			CMP_LESS,
			CMP_EQUAL,
			CMP_LEQUAL,
			CMP_GREATER,
			CMP_NOTEQUAL,
			CMP_GEQUAL,
			CMP_ALWAYS
		};

		enum DrawMode
		{
			DRAWMODE_NORMAL,
			DRAWMODE_ADDITIVE
		};

		Graphics() {}
		virtual ~Graphics() {}

		/*
		 * image drawing methods:
		 */
		virtual void drawImage(Image *img) = 0;
		virtual void drawImage(Image *img, const BoyLib::Rect &subrect) { drawImage(img, (int)subrect.getX(), (int)subrect.getY(), (int)subrect.getWidth(), (int)subrect.getHeight()); }
		virtual void drawImage(Image *img, int subrectX, int subrectY, int subrectW, int subrectH) = 0;

		/*
		 * geometric primitive drawing methods
		 */
		virtual void drawLine(int x0, int y0, int x1, int y1) = 0;
		virtual void fillRect(int x0, int y0, int w, int h) = 0;
		virtual void drawTriStrip(TriStrip *strip) = 0;

		/*
		 * rendering transform modification methods:
		 */
		virtual void scale(float x, float y) = 0;
		virtual void rotateDeg(float angle) = 0;
		virtual void rotateRad(float angle) = 0;
		virtual void translate(float x, float y) = 0;
		virtual void preScale(float x, float y) = 0;
		virtual void preRotateDeg(float angle) = 0;
		virtual void preRotateRad(float angle) = 0;
		virtual void preTranslate(float x, float y) = 0;

		/*
		 * push/pop transforms onto/from the transform stack
		 */
		virtual void pushTransform() = 0;
		virtual void popTransform() = 0;
		virtual int getTransformStackSize() = 0;

		/*
		 * tells the graphics object which color to  
		 * use for subsequent drawing operations
		 */
		virtual void setColor(Color color) = 0;

		/*
		 * changes the alpha value of the current
		 * color without affecting the actual color
		 * (alpha is in the range [0,1])
		 */
		virtual void setAlpha(float alpha) = 0;

		/*
		 * determines whether or not the color specified by setColor() 
		 * will be used to colorize image drawing operations
		 */
		virtual void setColorizationEnabled(bool enabled) = 0;

		/*
		 * methods for determining z buffer behavior (whether to
		 * write to the z buffer, whether to make z checks before
		 * drawing a pixel, and the z value to use for rendered
		 * images)
		 */
		virtual void setZTestEnabled(bool enabled) = 0;
		virtual bool isZTestEnabled() = 0;
		virtual void setZWriteEnabled(bool enabled) = 0;
		virtual bool isZWriteEnabled() = 0;
		virtual void setZFunction(CompareFunc func) = 0;
		virtual void setZ(float z) = 0;
		virtual float getZ() = 0;

		/*
		 * methods that set buffer clearing parameters (which 
		 * value to clear the z buffer and frame buffer to)
		 */
		virtual void setClearZ(float z) = 0;
		virtual void setClearColor(Color color) = 0;

		/*
		 * methods for controlling alpha testing (whether to draw 
		 * a pixel to the frame buffer based on its alpha value)
		 */
		virtual void setAlphaTestEnabled(bool enabled) = 0;
		virtual bool isAlphaTestEnabled() = 0;
		virtual void setAlphaReferenceValue(int val) = 0;
		virtual void setAlphaFunction(CompareFunc func) = 0;

		/*
		 * sets the draw mode for subsequent rendering calls
		 */
		virtual void setDrawMode(DrawMode mode) = 0;

		/*
		 * sets the clip rectangle for rendering:
		 */ 
		virtual void setClipRect(int x, int y, int width, int height) = 0;

		/*
		 * access to the window/screen size
		 */
		virtual int getWidth() = 0;
		virtual int getHeight() = 0;
	};
}
