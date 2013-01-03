#include "WinD3DInterface.h"

#include <assert.h>
#include "Boy/Mouse.h"
#include "BoyLib/BoyUtil.h"
#include "dxerr9.h"
#include "DXUT.h"
#include "Environment.h"
#include <fstream>
#include "Game.h"
#include <iostream>
#include "Keyboard.h"
#include "PersistenceLayer.h"
#include "ResourceManager.h"
#include "SDL.h"
#include "WinEnvironment.h"
#include "WinImage.h"
#include "WinTriStrip.h"

using namespace Boy;

#define PROJECTION_Z_NEAR 0
#define PROJECTION_Z_FAR 1

#define CAM_Z PROJECTION_Z_FAR

#define DEFAULT_D3DFORMAT D3DFMT_X8R8G8B8

#include "BoyLib/CrtDbgNew.h"

bool getPresentationParameters(D3DPRESENT_PARAMETERS &pp, int width, int height, bool windowed)
{
	pp.AutoDepthStencilFormat = D3DFMT_D16;
	pp.BackBufferCount = 3;
	pp.BackBufferFormat = (windowed ? D3DFMT_UNKNOWN : DEFAULT_D3DFORMAT);
	pp.BackBufferWidth = width;
	pp.BackBufferHeight = height;
	pp.EnableAutoDepthStencil = true;
	pp.Flags = 0;
	pp.FullScreen_RefreshRateInHz = 0;
	pp.hDeviceWindow = GetActiveWindow();
	pp.MultiSampleQuality = 0;
	pp.MultiSampleType = D3DMULTISAMPLE_NONE;
	pp.PresentationInterval = (windowed ? D3DPRESENT_INTERVAL_IMMEDIATE : D3DPRESENT_INTERVAL_DEFAULT);
	pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	pp.Windowed = windowed;

	// find the appropriate refresh rate for the desired resolution:
	if (!windowed)
	{
		D3DDISPLAYMODE mode;
		D3DFORMAT format = pp.BackBufferFormat;
		int modeCount = DXUTGetD3D9Object()->GetAdapterModeCount(D3DADAPTER_DEFAULT,format);
		for (int i=0 ; i<modeCount ; i++)
		{
			DXUTGetD3D9Object()->EnumAdapterModes(D3DADAPTER_DEFAULT,format,i,&mode);
			if (mode.Width==pp.BackBufferWidth &&
				mode.Height==pp.BackBufferHeight)
			{
				if (DXUTGetD3D9Object()->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, format, format, FALSE) == D3D_OK)
				{
					pp.FullScreen_RefreshRateInHz = mode.RefreshRate;
				}
			}
		}
		if (pp.FullScreen_RefreshRateInHz == 0)
		{
			return false;
		}
	}

	return true;
}

void CALLBACK askScreenSize(int *pWidth, int *pHeight, void* userContext)
{
	WinEnvironment *env = dynamic_cast<WinEnvironment*>(Environment::instance());
	env->getDesiredScreenSize(pWidth,pHeight);
}

static int gAltDown = false;
static int gShiftDown = false;
static int gControlDown = false;

Keyboard::Key getKey(UINT nChar)
{
	switch (nChar)
	{
	case VK_BACK:
		return Keyboard::KEY_BACKSPACE;
	case VK_TAB:
		return Keyboard::KEY_TAB;
	case VK_RETURN:
		return Keyboard::KEY_RETURN;
	case VK_SHIFT:
		return Keyboard::KEY_SHIFT;
	case VK_CONTROL:
		return Keyboard::KEY_CONTROL;
	case VK_PAUSE:
		return Keyboard::KEY_PAUSE;
	case VK_ESCAPE:
		return Keyboard::KEY_ESCAPE;
	case VK_END:
		return Keyboard::KEY_END;
	case VK_HOME:
		return Keyboard::KEY_HOME;
	case VK_LEFT:
		return Keyboard::KEY_LEFT;
	case VK_UP:
		return Keyboard::KEY_UP;
	case VK_RIGHT:
		return Keyboard::KEY_RIGHT;
	case VK_DOWN:
		return Keyboard::KEY_DOWN;
	case VK_INSERT:
		return Keyboard::KEY_INSERT;
	case VK_DELETE:
		return Keyboard::KEY_DELETE;
	case VK_F1:
		return Keyboard::KEY_F1;
	case VK_F2:
		return Keyboard::KEY_F2;
	case VK_F3:
		return Keyboard::KEY_F3;
	case VK_F4:
		return Keyboard::KEY_F4;
	case VK_F5:
		return Keyboard::KEY_F5;
	case VK_F6:
		return Keyboard::KEY_F6;
	case VK_F7:
		return Keyboard::KEY_F7;
	case VK_F8:
		return Keyboard::KEY_F8;
	case VK_F9:
		return Keyboard::KEY_F9;
	case VK_F10:
		return Keyboard::KEY_F10;
	case VK_F11:
		return Keyboard::KEY_F11;
	case VK_F12:
		return Keyboard::KEY_F12;
	}

	return Keyboard::KEY_UNKNOWN;
}

int getKeyMods()
{
	int mods = Keyboard::KEYMOD_NONE;
	if (gAltDown) mods |= Keyboard::KEYMOD_ALT;
	if (gShiftDown) mods |= Keyboard::KEYMOD_SHIFT;
	if (gControlDown) mods |= Keyboard::KEYMOD_CTRL;
	return mods;
}

UINT gLastKeyDown;

void CALLBACK keyPressed(UINT nChar, bool keyDown, bool altDown, void* userContext)
{
	WinEnvironment *env = dynamic_cast<WinEnvironment*>(Environment::instance());
	env->processVirtualMouseEvents(nChar,keyDown);

	gAltDown = altDown;
	gShiftDown = DXUTIsKeyDown(VK_SHIFT);
	gControlDown = DXUTIsKeyDown(VK_CONTROL);

	Keyboard *kb = Environment::instance()->getKeyboard(0);
	if (kb!=NULL)
	{
		int mods = getKeyMods();
		Keyboard::Key key = getKey(nChar);
		if (key!=Keyboard::KEY_UNKNOWN)
		{
			nChar = 0;
		}

		if (keyDown)
		{
			// if this is a modified key or if it has no unicode equivalent:
			bool haveMods = mods!=Keyboard::KEYMOD_NONE;
			bool noUnicode = (nChar==0);
			if (haveMods || noUnicode)
			{
				kb->fireKeyDownEvent(nChar,key,(Keyboard::Modifiers)mods);
				gLastKeyDown = nChar;
//				envDebugLog("keyPressed: nChar=0x%x (%c)\n",nChar,nChar);
			}
		}
		else
		{
			kb->fireKeyUpEvent(nChar, key, (Keyboard::Modifiers)mods);
		}
	}

#ifdef _DEBUG
	if (gControlDown && gAltDown && nChar=='D' && keyDown)
	{
		Environment *env = Environment::instance();
		env->setDebugEnabled(!env->isDebugEnabled());
	}
#endif
}

void CALLBACK charPressed(wchar_t nChar, void* userContext)
{
	Keyboard *kb = Environment::instance()->getKeyboard(0);
	if (kb!=NULL)
	{
		int mods = getKeyMods();
		if (mods==Keyboard::KEYMOD_NONE && gLastKeyDown!=nChar)
		{
			kb->fireKeyDownEvent((wchar_t)nChar,Keyboard::KEY_UNKNOWN,(Keyboard::Modifiers)mods);
		}
	}
//	envDebugLog("charPressed: nChar=0x%x (%c)\n",nChar,nChar);
}

bool CALLBACK modifyDeviceSettings(DXUTDeviceSettings* deviceSettings, const D3DCAPS9* caps, void* userContext)
{
//	envDebugLog("modifyDeviceSettings\n");
    return true;
}

static bool gLastMouseLeftDown = false;
static bool gLastMouseRightDown = false;
static int gLastMouseX = -1;
static int gLastMouseY = -1;
void CALLBACK mouseEvent(bool leftDown, bool rightDown, bool middleDown, bool side1Down, bool side2Down, int wheelDelta, int x, int y, void* userContext)
{
	// if this is a mouse move event:
	if (x!=gLastMouseX || y!=gLastMouseY)
	{
		gLastMouseX = x;
		gLastMouseY = y;
		Environment::instance()->getMouse(0)->fireMoveEvent((float)x,(float)y);
	}
	else if (gLastMouseLeftDown!=leftDown)
	{
		gLastMouseLeftDown = leftDown;
		if (leftDown)
		{
			Environment::instance()->getMouse(0)->fireDownEvent(Mouse::BUTTON_LEFT,1);
		}
		else
		{
			Environment::instance()->getMouse(0)->fireUpEvent(Mouse::BUTTON_LEFT);
		}
	}
	else if (wheelDelta!=0)
	{
		Environment::instance()->getMouse(0)->fireWheelEvent(wheelDelta);
	}
}

LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext)
{
	switch (uMsg)
	{
	case WM_MOVE:
	case WM_MOVING:
	case WM_WINDOWPOSCHANGING:
	case WM_WINDOWPOSCHANGED:
	case WM_GETMINMAXINFO:
	case WM_NCPAINT:
	case WM_ERASEBKGND:
	case WM_PAINT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
	case WM_ACTIVATEAPP:
	case WM_NCACTIVATE:
	case WM_IME_SETCONTEXT:
	case WM_IME_NOTIFY:
	case WM_NCHITTEST:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:
		break;
	case WM_SETCURSOR:
		break;
	case WM_ACTIVATE:
		/*
		switch (wParam)
		{
		case WA_INACTIVE:
			envDebugLog("window deactivated\n");
			break;
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			envDebugLog("window activated\n");
			break;
		}
		*/
		break;
	case WM_SETFOCUS:
		if (pUserContext!=NULL)
		{
			((Game*)pUserContext)->focusGained();
		}
		break;
	case WM_KILLFOCUS:
		if (pUserContext!=NULL)
		{
			((Game*)pUserContext)->focusLost();
		}
		break;
	case WM_QUIT:
		break;
	case WM_CLOSE:
		Boy::Environment::instance()->stopMainLoop();
		break;
	case WM_SYSCOMMAND:
		/*
		if (wParam==HTLEFT ||
			wParam==HTRIGHT||
			wParam==HTTOP ||
			wParam==HTTOPLEFT||
			wParam==HTTOPRIGHT ||
			wParam==HTBOTTOM ||
			wParam==HTBOTTOMLEFT ||
			wParam==HTBOTTOMRIGHT)
		*/
		if (!Boy::Environment::instance()->isWindowResizable())
		{
			if (wParam==0xf001 || // left
				wParam==0xf002 || // right
				wParam==0xf003 || // top
				wParam==0xf004 || // top left
				wParam==0xf005 || // top right
				wParam==0xf006 || // bottom
				wParam==0xf007 || // bottom left
				wParam==0xf008)   // bottom right
			{
				*pbNoFurtherProcessing = TRUE;
			}
		}
//		envDebugLog("WM_SYSCOMMAND: uMsg=0x%x wParam=0x%x lParam=0x%x\n",uMsg,wParam,lParam);
		break;
	case WM_ENTERSIZEMOVE:
//		envDebugLog("WM_ENTERSIZEMOVE: uMsg=0x%x wParam=0x%x lParam=0x%x\n",uMsg,wParam,lParam);
		break;
	case WM_NCLBUTTONDOWN:
		if (wParam==HTZOOM)
		{
			*pbNoFurtherProcessing = TRUE;
		}
		break;
	case WM_NCLBUTTONDBLCLK:
		*pbNoFurtherProcessing = TRUE;
//		envDebugLog("WM_NCLBUTTONDBLCLK: uMsg=0x%x wParam=0x%x lParam=0x%x\n",uMsg,wParam,lParam);
		break;
	case WM_SIZING:
		*pbNoFurtherProcessing = TRUE;
//		envDebugLog("WM_SIZING: uMsg=0x%x wParam=0x%x lParam=0x%x\n",uMsg,wParam,lParam);
		break;
	case WM_SIZE:
//		envDebugLog("WM_SIZE: uMsg=0x%x wParam=0x%x lParam=0x%x\n",uMsg,wParam,lParam);
		break;
	case WM_NCCALCSIZE:
//		envDebugLog("WM_NCCALCSIZE: uMsg=0x%x wParam=0x%x lParam=0x%x\n",uMsg,wParam,lParam);
		break;
	default:
//		envDebugLog("MsgProc: uMsg=0x%x wParam=0x%x lParam=0x%x\n",uMsg,wParam,lParam);
		break;
	}
    return 0;
}

HRESULT CALLBACK onCreateDevice(IDirect3DDevice9* d3dDevice, const D3DSURFACE_DESC* backBufferSurfaceDesc, void* userContext )
{
//	envDebugLog("onCreateDevice (windowed=%d w=%d h=%d)\n",DXUTIsWindowed(),backBufferSurfaceDesc->Width,backBufferSurfaceDesc->Height);
    return S_OK;
}

void CALLBACK onDestroyDevice(void* userContext )
{
//	envDebugLog("onDestroyDevice\n");
}

void CALLBACK onLostDevice(void* userContext)
{
//	envDebugLog("onLostDevice (windowed=%d)\n",DXUTIsWindowed());
	WinD3DInterface *inter = (WinD3DInterface*)userContext;
	inter->handleLostDevice();
//	DXUTGetD3D9Device()->Reset(&mPresentationParameters);
}

HRESULT CALLBACK onResetDevice(IDirect3DDevice9* d3dDevice, const D3DSURFACE_DESC* backBufferSurfaceDesc, void* userContext)
{
	assert(d3dDevice==DXUTGetD3D9Device());
//	envDebugLog("onResetDevice (windowed=%d w=%d h=%d)\n",DXUTIsWindowed(),backBufferSurfaceDesc->Width,backBufferSurfaceDesc->Height);
	WinD3DInterface *inter = (WinD3DInterface*)userContext;

	inter->handleResetDevice();
    return S_OK;
}

WinD3DInterface::WinD3DInterface(Game *game, int width, int height, const char *title, bool windowed, unsigned int refreshRate)
{
	// reset some members:
	mRendering = false;

	// remember the game:
	mGame = game;

	// register callbacks:
    DXUTSetCallbackD3D9DeviceCreated(onCreateDevice);
    DXUTSetCallbackD3D9DeviceReset(onResetDevice,this);
    DXUTSetCallbackD3D9DeviceLost(onLostDevice,this);
    DXUTSetCallbackD3D9DeviceDestroyed(onDestroyDevice);
    DXUTSetCallbackMsgProc(MsgProc,game);
	DXUTSetCallbackKeyboard(keyPressed);
	DXUTSetCallbackChar(charPressed);
	DXUTSetCallbackMouse(mouseEvent, true);
	DXUTSetCallbackAskScreenSize(askScreenSize,this);

    // initialize:
    DXUTInit();
	DXUTSetHotkeyHandling(true,false,false);

	// set up the cursor
    DXUTSetCursorSettings(true, false);

	// create window:
	DXUTDeviceSettings settings = DXUTGetDeviceSettings();
	settings.d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED;
	getPresentationParameters(settings.d3d9.pp,width,height,windowed);
	settings.d3d9.pp.FullScreen_RefreshRateInHz = refreshRate;
	if (windowed)
	{
		Boy::UString unicodeTitle(title);
		DXUTCreateWindow(unicodeTitle.wc_str());
	}
	DXUTCreateDeviceFromSettings(&settings);

	// initialize d3d:
	initD3D();

	// create the vertex buffer to be used 
	// for drawing subrects of images:
	mVertexBuffer = createVertexBuffer(4);

	// set the window title:
	mTitle = title;
	SDL_WM_SetCaption(mTitle.c_str(), NULL);

	// clearing params:
	mClearZ = PROJECTION_Z_FAR;
	mClearColor = 0x00000000;
}

WinD3DInterface::~WinD3DInterface()
{
	DXUTSetCallbackMsgProc(MsgProc,NULL); // game is NULL now
	DXUTShutdown();
}

int WinD3DInterface::getWidth()
{
	return DXUTGetWindowWidth();
}

int WinD3DInterface::getHeight()
{
	return DXUTGetWindowHeight();
}

bool WinD3DInterface::isWindowed()
{
	return DXUTIsWindowed();
}
/*
void WinD3DInterface::toggleFullScreen(bool toggle)
{
	// release the subrect vb:
	mVertexBuffer->Release();

	DXUTToggleFullScreen();

	// initialize D3D:
//	initD3D();

	// recreate the subrect vb:
	mVertexBuffer = createVertexBuffer(4);
}
*/
bool WinD3DInterface::beginScene()
{
	HRESULT hr = DXUTGetD3D9Device()->TestCooperativeLevel();
	if (hr!=S_OK)
	{
		handleError(hr);
		return false; // can't draw...
	}

	// set the rendering flag:
	mRendering = true;

	hr = DXUTGetD3D9Device()->Clear(
		0, // num rects to clear
		NULL, // rects
		D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
		mClearColor, // clear color
		mClearZ,  // z buffer clear value 
		0 );   // stencil clear value
	if(FAILED(hr)) return false;

	// let the rendering begin...
	hr = DXUTGetD3D9Device()->BeginScene();
	if(FAILED(hr)) return false;

	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);
	DXUTGetD3D9Device()->SetTransform(D3DTS_WORLD,&identity);

    D3DXMATRIX matView;    // the view transform matrix

	float w = (float)getWidth();
	float h = (float)getHeight();
	float w2 = w/2;
	float h2 = h/2;
	D3DXMatrixLookAtLH(&matView,
		&D3DXVECTOR3 (w2, h2, CAM_Z),	// the camera position
		&D3DXVECTOR3 (w2, h2, 0),		// the look-at position
		&D3DXVECTOR3 (0,  -1,  0));		// the up direction

    DXUTGetD3D9Device()->SetTransform(D3DTS_VIEW, &matView);    // set the view transform to matView

    D3DXMATRIX matProjection;     // the projection transform matrix

	D3DXMatrixOrthoLH(&matProjection,
		(FLOAT)w,
		(FLOAT)h,
		(FLOAT)PROJECTION_Z_NEAR,
		(FLOAT)PROJECTION_Z_FAR);

    DXUTGetD3D9Device()->SetTransform(D3DTS_PROJECTION, &matProjection);    // set the projection

	// set some default state stuff:
	DXUTGetD3D9Device()->SetRenderState(D3DRS_ALPHATESTENABLE, true);
	DXUTGetD3D9Device()->SetRenderState(D3DRS_ALPHAREF, 1);
	DXUTGetD3D9Device()->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	DXUTGetD3D9Device()->SetRenderState(D3DRS_COLORVERTEX, true);
	DXUTGetD3D9Device()->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);	
	DXUTGetD3D9Device()->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	DXUTGetD3D9Device()->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);

	return true;
}

void WinD3DInterface::endScene()
{
	// reset the rendering flag:
	mRendering = false;

	// end of rendering:
	DXUTGetD3D9Device()->EndScene();

	//Show the results
	HRESULT hr = DXUTGetD3D9Device()->Present(NULL,NULL,NULL,NULL);
	handleError(hr);
}

void WinD3DInterface::handleError(HRESULT hr)
{
	switch (hr)
	{
	case D3D_OK: // same as S_OK
		// all clear!
		break;
	case D3DERR_DEVICELOST:
		SDL_Delay(100);
		break;
	case D3DERR_DEVICENOTRESET:
		// this is a hack, but it works:
		DXUTToggleFullScreen();
		DXUTToggleFullScreen();
		// end of hack
		SDL_Delay(100);
		break;
	default:
		assertSuccess(hr);
		break;
	}
}

void WinD3DInterface::drawImage(IDirect3DTexture9 *tex)
{
	// make sure beginScene was called:
	assert(mRendering);

	// bind to image's vertex buffer:
	HRESULT hr = DXUTGetD3D9Device()->SetStreamSource(0, mVertexBuffer, 0, sizeof(BoyVertex));
	if(FAILED(hr)) return;

	// set texture:
	hr = DXUTGetD3D9Device()->SetTexture(0,tex);
	if(FAILED(hr)) return;

	// render from vertex buffer:
	hr = DXUTGetD3D9Device()->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	if(FAILED(hr)) return;
}

void WinD3DInterface::drawImage(WinImage *image, DWORD color, float z)
{
	drawImage(image, color, z, 0, 0, image->getWidth(), image->getHeight());
}

void WinD3DInterface::drawImage(WinImage *image, DWORD color, float z, int x, int y, int w, int h)
{
	if (image->getTexture()==NULL)
	{
		envDebugLog("WARNING: trying to draw image with NULL texture (%s)\n",image->getPath().c_str());
		return;
	}

	// update the subrect vertex buffer values:
	D3DSURFACE_DESC textureInfo;
	HRESULT hr = image->getTexture()->GetLevelDesc(0,&textureInfo);
	assert(!FAILED(hr));

	float maxX = (float)w / 2.0f;
	float maxY = (float)h / 2.0f;
	float minX = -maxX;
	float minY = -maxY;

	float minU, minV, maxU, maxV;
	if (image->isTextureScaled())
	{
		float imgW = (float)image->getWidth();
		float imgH = (float)image->getHeight();
		minU = x / imgW;
		minV = y / imgH;
		maxU = minU + w / imgW;
		maxV = minV + h / imgH;
	}
	else
	{
		float texW = (float)textureInfo.Width;
		float texH = (float)textureInfo.Height;
		minU = x / texW;
		minV = y / texH;
		maxU = minU + w / texW;
		maxV = minV + h / texH;
	}
	int numVerts = 4;
	BoyVertex data[] = 
	{
		{minX, minY, z, color, minU, minV}, // top left
		{maxX, minY, z, color, maxU, minV}, // top right
		{minX, maxY, z, color, minU, maxV}, // bottom left
		{maxX, maxY, z, color, maxU, maxV}  // bottom right
	};

	// lock the vertex buffer:
	void *vbData = NULL;
	hr = mVertexBuffer->Lock(0, 0, &vbData, 0);
	assert(!FAILED(hr));

	// copy the data into the buffer:
	int byteCount = sizeof(BoyVertex) * numVerts;
	memcpy(vbData, data, byteCount);

	// unlock it:
	mVertexBuffer->Unlock();

	// draw the image:
	drawImage(image->getTexture());
}

void WinD3DInterface::drawRect(int x, int y, int w, int h, float z, DWORD color)
{
	float minX = (float)x;
	float minY = (float)y;
	float maxX = minX + w;
	float maxY = minY + h;

	int numVerts = 4;
	BoyVertex data[] = 
	{
		{minX, minY, z, color, 0, 0}, // top left
		{maxX, minY, z, color, 0, 0}, // top right
		{minX, maxY, z, color, 0, 0}, // bottom left
		{maxX, maxY, z, color, 0, 0}  // bottom right
	};

	// lock the vertex buffer:
	void *vbData = NULL;
	HRESULT hr = mVertexBuffer->Lock(0, 0, &vbData, 0);
	handleError(hr);

	// copy the data into the buffer:
	int byteCount = sizeof(BoyVertex) * numVerts;
	memcpy(vbData, data, byteCount);

	// unlock it:
	mVertexBuffer->Unlock();

	// draw the image:
	drawImage(NULL);
}

void WinD3DInterface::drawTriStrip(WinTriStrip *strip)
{
	// make sure beginScene was called:
	assert(mRendering);

	// create a vertex buffer:
	IDirect3DVertexBuffer9 *vb = createVertexBuffer(strip->mVertexCount);

	// lock the vertex buffer:
	void *vbData = NULL;
	HRESULT hr = vb->Lock(0, 0, &vbData, 0);
	handleError(hr);

	// copy the data into the buffer:
	int byteCount = sizeof(BoyVertex) * strip->mVertexCount;
	memcpy(vbData, strip->mVerts, byteCount);

	// unlock it:
	vb->Unlock();

	// bind to tristrip's vertex buffer:
	hr = DXUTGetD3D9Device()->SetStreamSource(0, vb, 0, sizeof(BoyVertex));
	if(FAILED(hr)) return;

	// set texture to NULL:
	hr = DXUTGetD3D9Device()->SetTexture(0,NULL);
	if(FAILED(hr)) return;

	// render from vertex buffer:
	hr = DXUTGetD3D9Device()->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, strip->mVertexCount-2);
	if(FAILED(hr)) return;

	// release the vertex buffer:
	vb->Release();
}

void WinD3DInterface::drawLine(int x0, int y0, int x1, int y1, Color color)
{
	BoyVertex verts[2];
	verts[0].x = (float)x0;
	verts[0].y = (float)y0;
	verts[0].z = 0;
	verts[0].u = 0;
	verts[0].v = 0;
	verts[0].color = color;
	verts[1].x = (float)x1;
	verts[1].y = (float)y1;
	verts[1].z = 0;
	verts[1].u = 0;
	verts[1].v = 0;
	verts[1].color = color;

	// make sure beginScene was called:
	assert(mRendering);

	// create a vertex buffer:
	IDirect3DVertexBuffer9 *vb = createVertexBuffer(2);

	// lock the vertex buffer:
	void *vbData = NULL;
	HRESULT hr = vb->Lock(0, 0, &vbData, 0);
	handleError(hr);

	// copy the data into the buffer:
	int byteCount = sizeof(BoyVertex) * 2;
	memcpy(vbData, verts, byteCount);

	// unlock it:
	vb->Unlock();

	// bind to tristrip's vertex buffer:
	hr = DXUTGetD3D9Device()->SetStreamSource(0, vb, 0, sizeof(BoyVertex));
	if(FAILED(hr)) return;

	// set texture to NULL:
	hr = DXUTGetD3D9Device()->SetTexture(0,NULL);
	if(FAILED(hr)) return;

	// render from vertex buffer:
	hr = DXUTGetD3D9Device()->DrawPrimitive(D3DPT_LINELIST, 0, 1);
	if(FAILED(hr)) return;

	// release the vertex buffer:
	vb->Release();
}

#include <wchar.h>
void WinD3DInterface::assertSuccess(HRESULT hr)
{
//#ifdef _DEBUG
	if (hr<0)
	{
		const WCHAR *err = DXGetErrorString9(hr);
		const WCHAR *desc = DXGetErrorDescription9(hr);
		wprintf(err);
		printf("\n");
		wprintf(desc);
		printf("\n");
	}
//#endif
}

IDirect3DTexture9 *WinD3DInterface::loadTexture(const char *filenameUtf8, D3DXIMAGE_INFO *imageInfo, bool *scaled, bool warn)
{
	// declare some variables:
	IDirect3DTexture9 *tex = NULL;

	// get image info:
	UString filename(filenameUtf8);
	HRESULT result = D3DXGetImageInfoFromFile(filename.wc_str(), imageInfo);
	if (result != D3D_OK && warn)
	{
		printf("WARNING: D3DXGetImageInfoFromFile failed for '%s'\n",filename.toUtf8());
		assertSuccess(result);
		return NULL;
	}

	// calcualte texture width/height
	UINT width = D3DX_DEFAULT;
	UINT height = D3DX_DEFAULT;
	DWORD filter = D3DX_FILTER_NONE;
	if (imageInfo->Width > mMaxTextureWidth)
	{
		width = mMaxTextureWidth;
		if (mPow2TextureSize)
		{
			width = findHighestPowerOf2SmallerThan(width);
		}
		filter = D3DX_FILTER_LINEAR;
	}
	if (imageInfo->Height > mMaxTextureHeight)
	{
		height = mMaxTextureHeight;
		if (mPow2TextureSize)
		{
			width = findHighestPowerOf2SmallerThan(width);
		}
		filter = D3DX_FILTER_LINEAR;
	}

	// update the scaled flag:
	*scaled = (filter != D3DX_FILTER_NONE);

	// no color keying by default:
	D3DCOLOR colorKey = 0;

	// if this is a 32-bit ARGB image
	if (imageInfo->Format == D3DFMT_A8R8G8B8)
	{
		// map transparent white pixels to black:
		colorKey = 0x00ffffff;
		// (this fixes that white fuzz at the edges of images with alpha)
	}

	// load the texture:
	HRESULT hr = D3DXCreateTextureFromFileEx(
		DXUTGetD3D9Device(), // device
		filename.wc_str(), // file to load from
		width, // width
		height, // height
		D3DX_DEFAULT, // mipmap level count
		0, // usage
		D3DFMT_UNKNOWN, // pixel format
		D3DPOOL_MANAGED, // manged pool
		filter, // filtering function
		D3DX_DEFAULT, // default mipmap filter
		colorKey, // color key
		NULL, // optional structure to hold the source file info, but we get it above
		NULL, // no palette
		&tex); // the texture object to load the image into

	if (hr!=D3D_OK && warn)
	{
		printf("WARNING: D3DXCreateTextureFromFileEx failed for '%s'\n",filename.toUtf8());
		printf("w=%d h=%d filter=0x%x colorKey=0x%x\n",width,height,filter,colorKey);
		assertSuccess(hr);
	}

	if(FAILED(hr)) return NULL;

	// return it:
	return tex;
}

IDirect3DVertexBuffer9 *WinD3DInterface::createVertexBuffer(int numVerts)
{
	// create the vertex buffer:
	IDirect3DVertexBuffer9 *vb;
	int byteCount = sizeof(BoyVertex) * numVerts;
	HRESULT hr = DXUTGetD3D9Device()->CreateVertexBuffer(
		byteCount,
		D3DUSAGE_WRITEONLY,
		BOYFVF,
		D3DPOOL_MANAGED,
		&vb, 
		NULL);
	if(FAILED(hr)) return NULL;

	// return it:
	return vb;
}

void WinD3DInterface::initD3D()
{
	// get some device capabilities:
	D3DCAPS9 caps;
	DXUTGetD3D9Device()->GetDeviceCaps(&caps);
	mMaxTextureWidth = caps.MaxTextureWidth;
	mMaxTextureHeight = caps.MaxTextureHeight;
	mPow2TextureSize = (caps.TextureCaps & D3DPTEXTURECAPS_POW2) != 0;

	// set up an orthogonal projection matrix:
	D3DXMATRIX projection;
	D3DXMatrixOrthoRH(
		&projection, 
		(FLOAT)mPresentationParameters.BackBufferWidth,
		(FLOAT)mPresentationParameters.BackBufferHeight,
		(FLOAT)-1000,
		(FLOAT)+1000);
	HRESULT hr = DXUTGetD3D9Device()->SetTransform(D3DTS_PROJECTION,&projection);
	assert(!FAILED(hr));

	// set world / view matrices to identity:
	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);
	DXUTGetD3D9Device()->SetTransform(D3DTS_WORLD, &identity);
	DXUTGetD3D9Device()->SetTransform(D3DTS_VIEW, &identity);

	// turn off lighting:
    DXUTGetD3D9Device()->SetRenderState(D3DRS_LIGHTING, false);

	// set the flexible vertex format:
	hr = DXUTGetD3D9Device()->SetFVF(BOYFVF);
	if(FAILED(hr)) return;

	// set up texture blending parameters:
	hr = DXUTGetD3D9Device()->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
	if(FAILED(hr)) return;
	hr = DXUTGetD3D9Device()->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
	if(FAILED(hr)) return;
	hr = DXUTGetD3D9Device()->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
	if(FAILED(hr)) return;
	hr = DXUTGetD3D9Device()->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
	if(FAILED(hr)) return;
	hr = DXUTGetD3D9Device()->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_DIFFUSE);
	if(FAILED(hr)) return;
	hr = DXUTGetD3D9Device()->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_TEXTURE);
	if(FAILED(hr)) return;
	hr = DXUTGetD3D9Device()->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
	if(FAILED(hr)) return;
	hr = DXUTGetD3D9Device()->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
	if(FAILED(hr)) return;
	hr = DXUTGetD3D9Device()->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP);
	if(FAILED(hr)) return;
	hr = DXUTGetD3D9Device()->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP);
	if(FAILED(hr)) return;
}

void WinD3DInterface::setTransform(D3DXMATRIX &xform)
{
	// set the new transform:
	DXUTGetD3D9Device()->SetTransform(D3DTS_WORLD, &xform);
}

void WinD3DInterface::setSamplerState(D3DSAMPLERSTATETYPE state, DWORD value)
{
	DXUTGetD3D9Device()->SetSamplerState(0,state,value);
}

void WinD3DInterface::setRenderState(D3DRENDERSTATETYPE state, DWORD value)
{
	HRESULT hr = DXUTGetD3D9Device()->SetRenderState(state,value);
	assertSuccess(hr);
}

DWORD WinD3DInterface::getRenderState(D3DRENDERSTATETYPE state)
{
	DWORD value;
	DXUTGetD3D9Device()->GetRenderState(state,&value);
	return value;
}

void WinD3DInterface::setClipRect(int x, int y, int width, int height)
{
	RECT rect;
	rect.left = x;
	rect.right = x+width;
	rect.top = y;
	rect.bottom = y+height;
	DXUTGetD3D9Device()->SetScissorRect(&rect);
}

void WinD3DInterface::dumpInfo(std::ofstream &file)
{
	D3DCAPS9 caps;
	DXUTGetD3D9Device()->GetDeviceCaps(&caps);

	file << "type: ";
	switch (caps.DeviceType)
	{
	case D3DDEVTYPE_HAL:
		file << "D3DDEVTYPE_HAL";
		break;
	case D3DDEVTYPE_REF:
		file << "D3DDEVTYPE_REF";
		break;
	case D3DDEVTYPE_SW:
		file << "D3DDEVTYPE_SW";
		break;
	}
	file << "\n";

	file << "\nCaps3:\n";
	file << "\t D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD = " << ((caps.Caps3 & D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD) ? "true\n" : "false\n");

	file << "\nPresentationIntervals:\n";
	file << "\t D3DPRESENT_INTERVAL_DEFAULT = " << ((caps.PresentationIntervals & D3DPRESENT_INTERVAL_DEFAULT) ? "true\n" : "false\n");
	file << "\t D3DPRESENT_INTERVAL_ONE = " << ((caps.PresentationIntervals & D3DPRESENT_INTERVAL_ONE) ? "true\n" : "false\n");
	file << "\t D3DPRESENT_INTERVAL_TWO = " << ((caps.PresentationIntervals & D3DPRESENT_INTERVAL_TWO) ? "true\n" : "false\n");
	file << "\t D3DPRESENT_INTERVAL_THREE = " << ((caps.PresentationIntervals & D3DPRESENT_INTERVAL_THREE) ? "true\n" : "false\n");
	file << "\t D3DPRESENT_INTERVAL_FOUR = " << ((caps.PresentationIntervals & D3DPRESENT_INTERVAL_FOUR) ? "true\n" : "false\n");
	file << "\t D3DPRESENT_INTERVAL_IMMEDIATE = " << ((caps.PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE) ? "true\n" : "false\n");

	file << "\nDevCaps (D3D):\n";
	file << "\t D3DDEVCAPS_CANRENDERAFTERFLIP = "<<((caps.DevCaps & D3DDEVCAPS_CANRENDERAFTERFLIP)?"true\n":"false\n");
	file << "\t D3DDEVCAPS_DRAWPRIMTLVERTEX = "<<((caps.DevCaps & D3DDEVCAPS_DRAWPRIMTLVERTEX)?"true\n":"false\n");
	file << "\t D3DDEVCAPS_HWRASTERIZATION = "<<((caps.DevCaps & D3DDEVCAPS_HWRASTERIZATION)?"true\n":"false\n");
	file << "\t D3DDEVCAPS_PUREDEVICE = "<<((caps.DevCaps & D3DDEVCAPS_PUREDEVICE)?"true\n":"false\n");
	file << "\t D3DDEVCAPS_SEPARATETEXTUREMEMORIES = "<<((caps.DevCaps & D3DDEVCAPS_SEPARATETEXTUREMEMORIES)?"true\n":"false\n");
	file << "\t D3DDEVCAPS_TEXTURENONLOCALVIDMEM = "<<((caps.DevCaps & D3DDEVCAPS_TEXTURENONLOCALVIDMEM)?"true\n":"false\n");
	file << "\t D3DDEVCAPS_TEXTUREVIDEOMEMORY = "<<((caps.DevCaps & D3DDEVCAPS_TEXTUREVIDEOMEMORY)?"true\n":"false\n");
	file << "\t D3DDEVCAPS_TLVERTEXSYSTEMMEMORY = "<<((caps.DevCaps & D3DDEVCAPS_TLVERTEXSYSTEMMEMORY)?"true\n":"false\n");
	file << "\t D3DDEVCAPS_TLVERTEXVIDEOMEMORY = "<<((caps.DevCaps & D3DDEVCAPS_TLVERTEXVIDEOMEMORY)?"true\n":"false\n");

	file << "\nPrimitiveMiscCaps:\n";
	file << "\t D3DPMISCCAPS_MASKZ = "<<((caps.PrimitiveMiscCaps & D3DPMISCCAPS_MASKZ)?"true\n":"false\n");
	file << "\t D3DPMISCCAPS_BLENDOP = "<<((caps.PrimitiveMiscCaps & D3DPMISCCAPS_BLENDOP)?"true\n":"false\n");
	file << "\t D3DPMISCCAPS_SEPARATEALPHABLEND = "<<((caps.PrimitiveMiscCaps & D3DPMISCCAPS_SEPARATEALPHABLEND)?"true\n":"false\n");

	file << "\nRasterCaps:\n";
	file << "\t D3DPRASTERCAPS_SCISSORTEST = "<<((caps.RasterCaps & D3DPRASTERCAPS_SCISSORTEST)?"true\n":"false\n");
	file << "\t D3DPRASTERCAPS_ZBUFFERLESSHSR = "<<((caps.RasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR)?"true\n":"false\n");
	file << "\t D3DPRASTERCAPS_ZTEST = "<<((caps.RasterCaps & D3DPRASTERCAPS_ZTEST)?"true\n":"false\n");

	file << "\nZCmpCaps:\n";
	file << "\t D3DPCMPCAPS_GREATER = "<<((caps.ZCmpCaps & D3DPCMPCAPS_GREATER)?"true\n":"false\n");
	file << "\t D3DPCMPCAPS_GREATEREQUAL = "<<((caps.ZCmpCaps & D3DPCMPCAPS_GREATEREQUAL)?"true\n":"false\n");
	file << "\t D3DPCMPCAPS_LESS = "<<((caps.ZCmpCaps & D3DPCMPCAPS_LESS)?"true\n":"false\n");
	file << "\t D3DPCMPCAPS_LESSEQUAL = "<<((caps.ZCmpCaps & D3DPCMPCAPS_LESSEQUAL)?"true\n":"false\n");

	file << "\nSrcBlendCaps:\n";
	file << "\t D3DPBLENDCAPS_BOTHINVSRCALPHA = "<<((caps.SrcBlendCaps & D3DPBLENDCAPS_BOTHINVSRCALPHA)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_BOTHSRCALPHA = "<<((caps.SrcBlendCaps & D3DPBLENDCAPS_BOTHSRCALPHA)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_DESTALPHA = "<<((caps.SrcBlendCaps & D3DPBLENDCAPS_DESTALPHA)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_DESTCOLOR = "<<((caps.SrcBlendCaps & D3DPBLENDCAPS_DESTCOLOR)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_INVDESTALPHA = "<<((caps.SrcBlendCaps & D3DPBLENDCAPS_INVDESTALPHA)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_INVDESTCOLOR = "<<((caps.SrcBlendCaps & D3DPBLENDCAPS_INVDESTCOLOR)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_INVSRCALPHA = "<<((caps.SrcBlendCaps & D3DPBLENDCAPS_INVSRCALPHA)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_INVSRCCOLOR = "<<((caps.SrcBlendCaps & D3DPBLENDCAPS_INVSRCCOLOR)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_ONE = "<<((caps.SrcBlendCaps & D3DPBLENDCAPS_ONE)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_SRCALPHA = "<<((caps.SrcBlendCaps & D3DPBLENDCAPS_SRCALPHA)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_SRCALPHASAT = "<<((caps.SrcBlendCaps & D3DPBLENDCAPS_SRCALPHASAT)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_SRCCOLOR = "<<((caps.SrcBlendCaps & D3DPBLENDCAPS_SRCCOLOR)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_ZERO = "<<((caps.SrcBlendCaps & D3DPBLENDCAPS_ZERO)?"true\n":"false\n");

	file << "\nDestBlendCaps:\n";
	file << "\t D3DPBLENDCAPS_BOTHINVSRCALPHA = "<<((caps.DestBlendCaps & D3DPBLENDCAPS_BOTHINVSRCALPHA)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_BOTHSRCALPHA = "<<((caps.DestBlendCaps & D3DPBLENDCAPS_BOTHSRCALPHA)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_DESTALPHA = "<<((caps.DestBlendCaps & D3DPBLENDCAPS_DESTALPHA)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_DESTCOLOR = "<<((caps.DestBlendCaps & D3DPBLENDCAPS_DESTCOLOR)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_INVDESTALPHA = "<<((caps.DestBlendCaps & D3DPBLENDCAPS_INVDESTALPHA)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_INVDESTCOLOR = "<<((caps.DestBlendCaps & D3DPBLENDCAPS_INVDESTCOLOR)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_INVSRCALPHA = "<<((caps.DestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_INVSRCCOLOR = "<<((caps.DestBlendCaps & D3DPBLENDCAPS_INVSRCCOLOR)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_ONE = "<<((caps.DestBlendCaps & D3DPBLENDCAPS_ONE)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_SRCALPHA = "<<((caps.DestBlendCaps & D3DPBLENDCAPS_SRCALPHA)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_SRCALPHASAT = "<<((caps.DestBlendCaps & D3DPBLENDCAPS_SRCALPHASAT)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_SRCCOLOR = "<<((caps.DestBlendCaps & D3DPBLENDCAPS_SRCCOLOR)?"true\n":"false\n");
	file << "\t D3DPBLENDCAPS_ZERO = "<<((caps.DestBlendCaps & D3DPBLENDCAPS_ZERO)?"true\n":"false\n");

	file << "\nAlphaCmpCaps:\n";
	file << "\t D3DPCMPCAPS_GREATER = "<<((caps.AlphaCmpCaps & D3DPCMPCAPS_GREATER)?"true\n":"false\n");
	file << "\t D3DPCMPCAPS_GREATEREQUAL = "<<((caps.AlphaCmpCaps & D3DPCMPCAPS_GREATEREQUAL)?"true\n":"false\n");
	file << "\t D3DPCMPCAPS_LESS = "<<((caps.AlphaCmpCaps & D3DPCMPCAPS_LESS)?"true\n":"false\n");
	file << "\t D3DPCMPCAPS_LESSEQUAL = "<<((caps.AlphaCmpCaps & D3DPCMPCAPS_LESSEQUAL)?"true\n":"false\n");

	file << "\nTextureCaps:\n";
	file << "\t D3DPTEXTURECAPS_ALPHA = "<<((caps.TextureCaps & D3DPTEXTURECAPS_ALPHA)?"true\n":"false\n");
	file << "\t D3DPTEXTURECAPS_MIPMAP = "<<((caps.TextureCaps & D3DPTEXTURECAPS_MIPMAP)?"true\n":"false\n");
	file << "\t D3DPTEXTURECAPS_NONPOW2CONDITIONAL = "<<((caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)?"true\n":"false\n");
	file << "\t D3DPTEXTURECAPS_POW2 = "<<((caps.TextureCaps & D3DPTEXTURECAPS_POW2)?"true\n":"false\n");
	file << "\t D3DPTEXTURECAPS_SQUAREONLY = "<<((caps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY)?"true\n":"false\n");

	file << "\nTextureFilterCaps:\n";
	file << "\t D3DPTFILTERCAPS_MAGFLINEAR = "<<((caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR)?"true\n":"false\n");
	file << "\t D3DPTFILTERCAPS_MINFLINEAR = "<<((caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR)?"true\n":"false\n");
	file << "\t D3DPTFILTERCAPS_MIPFLINEAR = "<<((caps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR)?"true\n":"false\n");

	file << "\nTextureAddressCaps:\n";
	file << "\t D3DPTADDRESSCAPS_BORDER = "<<((caps.TextureAddressCaps & D3DPTADDRESSCAPS_BORDER)?"true\n":"false\n");
	file << "\t D3DPTADDRESSCAPS_CLAMP = "<<((caps.TextureAddressCaps & D3DPTADDRESSCAPS_CLAMP)?"true\n":"false\n");
	file << "\t D3DPTADDRESSCAPS_INDEPENDENTUV = "<<((caps.TextureAddressCaps & D3DPTADDRESSCAPS_INDEPENDENTUV)?"true\n":"false\n");
	file << "\t D3DPTADDRESSCAPS_MIRROR = "<<((caps.TextureAddressCaps & D3DPTADDRESSCAPS_MIRROR)?"true\n":"false\n");
	file << "\t D3DPTADDRESSCAPS_MIRRORONCE = "<<((caps.TextureAddressCaps & D3DPTADDRESSCAPS_MIRRORONCE)?"true\n":"false\n");
	file << "\t D3DPTADDRESSCAPS_WRAP = "<<((caps.TextureAddressCaps & D3DPTADDRESSCAPS_WRAP)?"true\n":"false\n");
	
	file << "\nLineCaps:\n";
	file << "\t D3DLINECAPS_ALPHACMP = "<<((caps.LineCaps & D3DLINECAPS_ALPHACMP)?"true\n":"false\n");
	file << "\t D3DLINECAPS_ANTIALIAS = "<<((caps.LineCaps & D3DLINECAPS_ANTIALIAS)?"true\n":"false\n");
	file << "\t D3DLINECAPS_BLEND = "<<((caps.LineCaps & D3DLINECAPS_BLEND)?"true\n":"false\n");
	file << "\t D3DLINECAPS_ZTEST = "<<((caps.LineCaps & D3DLINECAPS_ZTEST)?"true\n":"false\n");

	file << "\nMaxTextureWidth = " << caps.MaxTextureWidth << "\n";
	file << "\nMaxTextureHeight = " << caps.MaxTextureHeight << "\n";
	file << "\nMaxTextureAspectRatio = " << caps.MaxTextureAspectRatio << "\n";

	file << "\nTextureOpCaps:\n";
	file << "\t D3DTEXOPCAPS_ADD = "<<((caps.TextureOpCaps & D3DTEXOPCAPS_ADD)?"true\n":"false\n");
	file << "\t D3DTEXOPCAPS_ADDSIGNED = "<<((caps.TextureOpCaps & D3DTEXOPCAPS_ADDSIGNED)?"true\n":"false\n");
	file << "\t D3DTEXOPCAPS_MODULATE = "<<((caps.TextureOpCaps & D3DTEXOPCAPS_MODULATE)?"true\n":"false\n");
	file << "\t D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR = "<<((caps.TextureOpCaps & D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR)?"true\n":"false\n");
	file << "\t D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA = "<<((caps.TextureOpCaps & D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA)?"true\n":"false\n");
	file << "\t D3DTEXOPCAPS_SELECTARG1 = "<<((caps.TextureOpCaps & D3DTEXOPCAPS_SELECTARG1)?"true\n":"false\n");
	file << "\t D3DTEXOPCAPS_SELECTARG2 = "<<((caps.TextureOpCaps & D3DTEXOPCAPS_SELECTARG2)?"true\n":"false\n");
	file << "\t D3DTEXOPCAPS_SUBTRACT = "<<((caps.TextureOpCaps & D3DTEXOPCAPS_SUBTRACT)?"true\n":"false\n");
}

void WinD3DInterface::handleLostDevice()
{
	mVertexBuffer->Release();
	ResourceManager *rm = Environment::instance()->getResourceManager();
	if (rm!=NULL)
	{
		printf("handleDeviceLost(): calling rm->destroyResources()... ");
		rm->destroyResources(false);
		printf("done\n");
	}
}

void WinD3DInterface::handleResetDevice()
{
	// persist the fullscreen setting:
	Boy::Environment *env = Boy::Environment::instance();
	Boy::PersistenceLayer *pl = env->getPersistenceLayer();
	bool fullscreen = env->isFullScreen();
	pl->putString("fullscreen",fullscreen?"true":"false",true);

	mVertexBuffer = createVertexBuffer(4);
	ResourceManager *rm = Environment::instance()->getResourceManager();
	if (rm!=NULL)
	{
		printf("handleResetDevice(): calling rm->initResources()... ");
		rm->initResources(false);
		printf("done\n");
	}
	initD3D();

	// notify the game of the device reset:
	mGame->windowResized(0,0,DXUTGetWindowWidth(),DXUTGetWindowHeight());
}
