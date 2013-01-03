#pragma once

#include "d3d9.h"
#include "d3dx9.h"
#include "Environment.h"
#include "Graphics.h"
#include <string>

namespace Boy
{
	// define our flexible vertex format;
	#define BOYFVF D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1
	struct BoyVertex 
	{
		float x, y, z; // position
		DWORD color;
		float u, v; // texture coords
	};

	class Game;
	class WinImage;
	class WinTriStrip;

	class WinD3DInterface
	{
	public:

		WinD3DInterface(Game *game, int width, int height, const char *title, bool windowed, unsigned int refreshRate);
		virtual ~WinD3DInterface();

		// window methods:
		int getWidth();
		int getHeight();
		bool isWindowed();
//		void toggleFullScreen(bool toggle);

		// rendering methods:
		bool beginScene();
		void endScene();
		void drawImage(WinImage *image, DWORD color, float z);
		void drawImage(WinImage *image, DWORD color, float z, int x, int y, int w, int h);
		void drawRect(int x, int y, int w, int h, float z, DWORD color);
		void drawTriStrip(WinTriStrip *strip);
		void drawLine(int x0, int y0, int x1, int y1, Color color);

		// world transformation:
		void setTransform(D3DXMATRIX &xform);

		// texture loading:
		IDirect3DTexture9 *loadTexture(const char *filenameUtf8, D3DXIMAGE_INFO *imageInfo, bool *scaled, bool warn=true);

		// vertex buffer creation:
		IDirect3DVertexBuffer9 *createVertexBuffer(int numVerts);

		// clearing:
		inline void setClearZ(float z) { mClearZ = z; }
		inline void setClearColor(DWORD color) { mClearColor = color; }

		// d3d state:
		void setSamplerState(D3DSAMPLERSTATETYPE state, DWORD value);
		void setRenderState(D3DRENDERSTATETYPE state, DWORD value);
		DWORD getRenderState(D3DRENDERSTATETYPE state);

		// clipping:
		void setClipRect(int x, int y, int width, int height);

		// misc:
		void dumpInfo(std::ofstream &file);
		void handleLostDevice();
		void handleResetDevice();

	private:

		void initD3D();
		void assertSuccess(HRESULT hr);
		void drawImage(IDirect3DTexture9 *tex);
		void printDisplayModes(D3DFORMAT format, bool windowed);
		void handleError(HRESULT hr);

	private:

		std::string				mTitle;

		Boy::Game				*mGame;

		D3DPRESENT_PARAMETERS	mPresentationParameters;

		bool					mRendering;

		IDirect3DVertexBuffer9	*mVertexBuffer;

		float					mClearZ;
		DWORD					mClearColor;

		// hardware capabilities:
		UINT					mMaxTextureWidth;
		UINT					mMaxTextureHeight;
		bool					mPow2TextureSize;
	};
}

