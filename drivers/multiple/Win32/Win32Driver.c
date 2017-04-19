/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

//--------------------------------------------------------------------------
// Headers required for this driver
//--------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <wingdi.h>
#include <assert.h>

//--------------------------------------------------------------------------
// Include our board file (coded directly in here for Win32)
//--------------------------------------------------------------------------
//#include "gdisp_board_win32.h"

typedef struct GDISPDRIVERID(BoardPrivateArea) {
	HWND			hwnd;
	BITMAPV4HEADER	drawBMP;
	HDC				winDC;
	HBITMAP			winDIB;
	HBITMAP 		winDIBOrig;
	#if GFX_USE_GINPUT && GINPUT_NEED_MOUSE
		gCoord		mousex, mousey;
		uint16_t	mousebuttons;
		GMouse		*mouse;
		bool_t		mouseenabled;
		void (*capfn)(void * hWnd, GDisplay *g, uint16_t buttons, gCoord x, gCoord y);
	#endif
} GDISPDRIVERID(BoardPrivateArea);

//--------------------------------------------------------------------------
// Include our base driver template (framebuffer)
//--------------------------------------------------------------------------
#include "../../gdisp/gdisp_fb_rgb_unpacked.h"

//--------------------------------------------------------------------------
// Configuration options for this driver
//--------------------------------------------------------------------------

#ifndef GDISP_WIN32_WIDTH
	#define GDISP_WIN32_WIDTH		640
#endif
#ifndef GDISP_WIN32_HEIGHT
	#define GDISP_WIN32_HEIGHT		480
#endif
#ifndef GDISP_WIN32_DISPLAYS
	#define GDISP_WIN32_DISPLAYS	1
#endif
#ifndef GDISP_WIN32_SCALE
	#define GDISP_WIN32_SCALE		1
#endif

//--------------------------------------------------------------------------
// GDISP Driver
//--------------------------------------------------------------------------

// Extra defines specific for Win32
#define GDISP_WIN32_WINOFFSETX		50
#define GDISP_WIN32_WINOFFSETY		50
#define GDISP_WIN32_TITLE 			"uGFX"
#define GDISP_WIN32_FLG_READY		(GDISP_FLG_DRIVER<<0)
#define GDISP_WIN32_LINEBYTES		((sizeof(GDISP_DRIVER_COLOR_TYPE) * GDISP_WIN32_WIDTH + 3) & ~3)

#if GFX_USE_GINPUT && GINPUT_NEED_TOGGLE
	#include "Win32Driver_toggle.c"
#endif

#if GFX_USE_GINPUT && GINPUT_NEED_MOUSE
	#include "Win32Driver_mouse.c"
#endif

#if GFX_USE_GINPUT && GINPUT_NEED_KEYBOARD
	#include "Win32Driver_keyboard.c"
#endif

static HWND				GDISP_Win32hWndParent = 0;

/*===========================================================================*/
/* Driver local routines    .                                                */
/*===========================================================================*/

void gfxWin32SetParentWindow(void *hwnd) {
	GDISP_Win32hWndParent = (HWND)hwnd;
}

typedef struct gfxWin32CreateWinStruct {
	LPCTSTR			lpClassName;
	LPCTSTR			lpWindowName;
	DWORD			dwStyle;
	int				x, y;
	int				nWidth, nHeight;
	HWND			hWndParent;
	HMENU			hMenu;
	HINSTANCE		hInstance;
	LPVOID			lpParam;
	volatile bool_t	done;
	volatile HWND	hWnd;
} gfxWin32CreateWinStruct;

static DWORD WINAPI gfxWin32MessageLoop(void *param) {
	MSG msg;
	(void)param;

	// Establish ourselves as a message loop thread	
	PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE);
	
	// Let our parent thread know that we are ready
	*((bool_t *)param) = GTrue;
	
	while(GetMessage(&msg, 0, 0, 0) > 0) {
		// Is this our special thread message to create a new window?
		if (!msg.hwnd && msg.message == WM_USER) {
			gfxWin32CreateWinStruct *w;
			
			w = (gfxWin32CreateWinStruct *)msg.lParam;
			w->hWnd = CreateWindow(w->lpClassName, w->lpWindowName, w->dwStyle, w->x, w->y, w->nWidth, w->nHeight, w->hWndParent, w->hMenu, w->hInstance, w->lpParam);
			w->done = GTrue;

		// Or just a normal window message
		} else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	// Just kill the whole thing when we get the WM_QUIT message (or a fatal message loop error)
	ExitProcess(0);
	return 0;
}

/* Create a Win32 window using the normal CreateWindow parameters.
 *
 * 	The reason we do this indirectly is because we want to create a seperate message loop thread to prevent
 * 	the uGFX having to deal with message loop.
 *	Unfortunately the thread that creates the window "owns" the message queue so we need to create the window
 * 	in the above message loop thread. We do this by posting a special thread message to the message loop who then
 * 	creates the window for us.
 */
static HWND gfxWin32CreateWindow(LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
	static DWORD			winThreadId = 0;
	gfxWin32CreateWinStruct	w;
	
	if (!winThreadId) {
		HANDLE			hth;
		volatile bool_t	ready;
		
		// Start the message loop thread
		ready = GFalse;
		if (!(hth = CreateThread(0, 0, gfxWin32MessageLoop, (void *)&ready, 0, &winThreadId)))
			return 0;
		SetThreadPriority(hth, THREAD_PRIORITY_ABOVE_NORMAL);
		CloseHandle(hth);
		
		// Wait until the thread says it is a message loop
		while(!ready)
			Sleep(1);
	}
	
	w.lpClassName = lpClassName;
	w.lpWindowName = lpWindowName;
	w.dwStyle = dwStyle;
	w.x = x;
	w.y = y;
	w.nWidth = nWidth;
	w.nHeight = nHeight;
	w.hWndParent = hWndParent;
	w.hMenu = hMenu;
	w.hInstance = hInstance;
	w.lpParam = lpParam;
	w.done = GFalse;
	w.hWnd = 0;
	PostThreadMessage(winThreadId, WM_USER, 0, (LPARAM)&w);
	
	// Wait for the result
	while (!w.done)
		Sleep(1);
		
	return w.hWnd;
}

static LRESULT GDISPDRIVERID(WindowProc)(HWND hWnd,	UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC				dc;
	PAINTSTRUCT		ps;
	GDisplay *		g;

	switch (Msg) {
	case WM_CREATE:
		// Get our GDisplay structure and attach it to the window
		g = (GDisplay *)((LPCREATESTRUCT)lParam)->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)g);

		// Fill in the private area
		myg->bpa.hwnd = hWnd;
		dc = GetDC(hWnd);
		myg->bpa.winDIB = CreateCompatibleBitmap(dc, GDISP_WIN32_WIDTH*GDISP_WIN32_SCALE, GDISP_WIN32_HEIGHT*GDISP_WIN32_SCALE);
		myg->bpa.winDC = CreateCompatibleDC(dc);
		ReleaseDC(hWnd, dc);
		myg->bpa.winDIBOrig = SelectObject(myg->bpa.winDC, myg->bpa.winDIB);

		// Mark the window as ready to go
		g->flags |= GDISP_WIN32_FLG_READY;
		break;

	case WM_ERASEBKGND:
		// Pretend we have erased the background.
		// We know we don't really need to do this as we
		// redraw the entire surface in the WM_PAINT handler.
		return GTrue;

	case WM_PAINT:
		// Get our GDisplay structure
		g = (GDisplay *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

		// Paint the main window area
		dc = BeginPaint(hWnd, &ps);
		BitBlt(dc, ps.rcPaint.left, ps.rcPaint.top,
			ps.rcPaint.right - ps.rcPaint.left,
			ps.rcPaint.bottom - ps.rcPaint.top,
			myg->bpa.winDC, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		// Get our GDisplay structure
		g = (GDisplay *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

		// Restore the window and free our bitmaps
		SelectObject(myg->bpa.winDC, myg->bpa.winDIBOrig);
		DeleteDC(myg->bpa.winDC);
		DeleteObject(myg->bpa.winDIB);

		// Quit the application
		PostQuitMessage(0);
		break;

	default:
		#if GFX_USE_GINPUT && GINPUT_NEED_KEYBOARD
			if (Win32KeyboardMsgHook(hWnd,	Msg, wParam, lParam)
				break;
		#endif
		#if GFX_USE_GINPUT && GINPUT_NEED_MOUSE
			if (Win32MouseMsgHook(hWnd,	Msg, wParam, lParam)
				break;
		#endif


		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	return 0;
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

static unsigned GDISPDRIVERID(count)(const struct GDISPVMT *vmt) {
	(void) vmt;
	return GDISP_WIN32_DISPLAYS;
}

static bool_t GDISPDRIVERID(init)(GDisplay *g) {
	static bool_t initDone = 0;
	RECT		rect;
	char		titlebuf[sizeof(GDISP_WIN32_TITLE) + 10];

	// Initialise the window class (if it hasn't been done already)
	if (!initDone) {
		WNDCLASS		wc;
		ATOM			winClass;

		// Create the window class
		wc.style           = CS_HREDRAW | CS_VREDRAW; // | CS_OWNDC;
		wc.lpfnWndProc     = (WNDPROC)GDISPDRIVERID(WindowProc);
		wc.cbClsExtra      = 0;
		wc.cbWndExtra      = 0;
		wc.hInstance       = GetModuleHandle(0);
		wc.hIcon           = LoadIcon(0, IDI_APPLICATION);
		wc.hCursor         = LoadCursor(0, IDC_ARROW);
		wc.hbrBackground   = GetStockObject(WHITE_BRUSH);
		wc.lpszMenuName    = 0;
		wc.lpszClassName   = GDISP_WIN32_TITLE;
		winClass = RegisterClass(&wc);
		assert(winClass != 0);
		initDone = GTrue;
	}

	// Initialise the GDISP structure
	g->g.Orientation = GDISP_ROTATE_0;
	g->g.Powermode = powerOn;
	g->g.Backlight = 100;
	g->g.Contrast = 50;
	g->g.Width = GDISP_WIN32_WIDTH;
	g->g.Height = GDISP_WIN32_HEIGHT;

	// Init the framebuffer: Allocate the frame buffer inline - we use the Win32 API here to avoid any uGFX heap
	GDISP_FRAMEBUFFER_INIT(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, GDISP_WIN32_LINEBYTES * GDISP_WIN32_HEIGHT), GDISP_WIN32_LINEBYTES);
	
	// Safety check!
	assert(myg->dpa.fb != 0);

	// Build the bitmap header	
	myg->bpa.drawBMP.bV4Size			= sizeof(myg->bpa.drawBMP);
	myg->bpa.drawBMP.bV4Width			= GDISP_WIN32_WIDTH;
	myg->bpa.drawBMP.bV4Height			= -GDISP_WIN32_HEIGHT;	// Negative makes it a top-down image
	myg->bpa.drawBMP.bV4Planes			= 1;
	myg->bpa.drawBMP.bV4BitCount		= GDISP_DRIVER_COLOR_TYPE_BITS;
	myg->bpa.drawBMP.bV4V4Compression	= BI_BITFIELDS;
	myg->bpa.drawBMP.bV4SizeImage		= GDISP_WIN32_HEIGHT * GDISP_WIN32_LINEBYTES;
	myg->bpa.drawBMP.bV4XPelsPerMeter	= 3078;
	myg->bpa.drawBMP.bV4YPelsPerMeter	= 3078;
	myg->bpa.drawBMP.bV4ClrUsed			= 0;
	myg->bpa.drawBMP.bV4ClrImportant	= 0;
	myg->bpa.drawBMP.bV4RedMask			= GDISP_DRIVER_RGB2COLOR(255,0,0);
	myg->bpa.drawBMP.bV4GreenMask		= GDISP_DRIVER_RGB2COLOR(0,255,0);
	myg->bpa.drawBMP.bV4BlueMask		= GDISP_DRIVER_RGB2COLOR(0,0,255);
	myg->bpa.drawBMP.bV4AlphaMask		= 0;
	myg->bpa.drawBMP.bV4CSType			= 0; //LCS_sRGB;

	// Allocate the frame buffer - we use the Win32 API here to avoid any uGFX heap
	myg->dpa.fb = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, myg->bpa.drawBMP.bV4SizeImage);

	// Set the window rectangle
	rect.top = 0; rect.bottom = GDISP_WIN32_HEIGHT*GDISP_WIN32_SCALE;
	rect.left = 0; rect.right = GDISP_WIN32_WIDTH*GDISP_WIN32_SCALE;
	AdjustWindowRect(&rect, WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU, 0);

	// Generate the window title
	sprintf(titlebuf, GDISP_WIN32_TITLE " - %u", g->systemdisplay+1);

	// Create the window
	myg->bpa.hwnd = gfxWin32CreateWindow(GDISP_WIN32_TITLE, titlebuf, WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_BORDER|WS_VISIBLE,
						g->controllerdisplay*GDISP_WIN32_WINOFFSETX, g->controllerdisplay*GDISP_WIN32_WINOFFSETY,
						rect.right-rect.left, rect.bottom-rect.top,
						GDISP_Win32hWndParent, 0,
						GetModuleHandle(0), g);
	assert(myg->bpa.hwnd != 0);

	// Wait for the window creation to complete (for safety)
	while(!(((volatile GDisplay *)g)->flags & GDISP_WIN32_FLG_READY))
		Sleep(1);

	// Create the associated mouse
	#if GFX_USE_GINPUT && GINPUT_NEED_MOUSE
		myg->bpa.mouseenabled = GDISP_Win32hWndParent ? GFalse : GTrue;
		myg->bpa.mouse = (GMouse *)gdriverRegister((const GDriverVMT const *)GMOUSE_DRIVER_VMT, g);
	#endif

	return GTrue;
}

#if GDISP_DRIVER_FLUSH
	static void GDISPDRIVERID(flush)(GDisplay *g) {
		#if GDISP_FRAMEBUFFER_REGIONFLUSH
			RECT	r;
			gCoord	w, h;
				
			// Do we really need to flush?
			GDISP_FRAMEBUFFER_FLUSHTEST();
			
			// Convert the flush area to windows format
			r.left = myg->dpa.ua.p1.x*GDISP_WIN32_SCALE;
			r.right = (myg->dpa.ua.p2.x+1)*GDISP_WIN32_SCALE;
			r.bottom = myg->dpa.ua.p1.y*GDISP_WIN32_SCALE;
			r.top = (myg->dpa.ua.p2.y+1)*GDISP_WIN32_SCALE;
			w = myg->dpa.ua.p2.x-myg->dpa.ua.p1.x+1;
			h = myg->dpa.ua.p2.y-myg->dpa.ua.p1.y+1;

			// Transfer the changed bits
			#if GDISP_WIN32_SCALE == 1
				SetDIBitsToDevice(myg->bpa.winDC, r.left, r.bottom, w*GDISP_WIN32_SCALE, h*GDISP_WIN32_SCALE,
								myg->dpa.ua.p1.x, (GDISP_WIN32_HEIGHT-1)-myg->dpa.ua.p2.y, 0, GDISP_WIN32_HEIGHT,
								myg->dpa.fb, (BITMAPINFO*)&myg->bpa.drawBMP, DIB_RGB_COLORS);
			#else
				StretchDIBits(myg->bpa.winDC, r.left, r.bottom, w*GDISP_WIN32_SCALE, h*GDISP_WIN32_SCALE,
								myg->dpa.ua.p1.x, (GDISP_WIN32_HEIGHT-1)-myg->dpa.ua.p2.y, w, h,
								myg->dpa.fb, (BITMAPINFO*)&myg->bpa.drawBMP, DIB_RGB_COLORS, SRCCOPY);
			#endif
			
			// Mark the flush area for update
			InvalidateRect(myg->bpa.hwnd, &r, FALSE);
			
		#else
			// Do we really need to flush?
			GDISP_FRAMEBUFFER_FLUSHTEST();
				
			// Transfer the changed bits
			#if GDISP_WIN32_SCALE == 1
				SetDIBits(myg->bpa.winDC, myg->bpa.winDIB, 0, GDISP_WIN32_HEIGHT, 
								myg->dpa.fb, (BITMAPINFO*)&myg->bpa.drawBMP, DIB_RGB_COLORS);
				
			#else
				StretchDIBits(myg->bpa.winDC, 0, 0, GDISP_WIN32_WIDTH*GDISP_WIN32_SCALE, GDISP_WIN32_HEIGHT*GDISP_WIN32_SCALE,
								0, 0, GDISP_WIN32_WIDTH, GDISP_WIN32_HEIGHT,
								myg->dpa.fb, (BITMAPINFO*)&myg->bpa.drawBMP, DIB_RGB_COLORS, SRCCOPY);
			#endif

			// Mark the flush area for update
			InvalidateRect(myg->bpa.hwnd, 0, FALSE);
		#endif
		
		// Ensure the flushed area is actually drawn 		
		UpdateWindow(myg->bpa.hwnd);
		
		// Reset the flush region ready for the next flush
		GDISP_FRAMEBUFFER_FLUSHRESET();
	}
#endif


/* ---- Optional Routines ---- */

#if GDISP_DRIVER_BITFILLS && GDISP_NEED_CONTROL
	static gPixel *rotateimg(GDisplay *g, const gPixel *buffer) {
		gPixel	*dstbuf;
		gPixel	*dst;
		const gPixel	*src;
		size_t	sz;
		gCoord	i, j;

		// Allocate the destination buffer
		sz = (size_t)g->p.cx * (size_t)g->p.cy;
		if (!(dstbuf = (gPixel *)malloc(sz * sizeof(gPixel))))
			return 0;

		// Copy the bits we need
		switch(g->g.Orientation) {
		case GDISP_ROTATE_0:
		default:
			return 0;					// not handled as it doesn't need to be.
		case GDISP_ROTATE_90:
			for(src = buffer+g->p.x1, j = 0; j < g->p.cy; j++, src += g->p.x2 - g->p.cx) {
				dst = dstbuf+sz-g->p.cy+j;
				for(i = 0; i < g->p.cx; i++, dst -= g->p.cy)
					*dst = *src++;
			}
			break;
		case GDISP_ROTATE_180:
			for(dst = dstbuf+sz, src = buffer+g->p.x1, j = 0; j < g->p.cy; j++, src += g->p.x2 - g->p.cx)
				for(i = 0; i < g->p.cx; i++)
					*--dst = *src++;
			break;
		case GDISP_ROTATE_270:
			for(src = buffer+g->p.x1, j = 0; j < g->p.cy; j++, src += g->p.x2 - g->p.cx) {
				dst = dstbuf+g->p.cy-j-1;
				for(i = 0; i < g->p.cx; i++, dst += g->p.cy)
					*dst = *src++;
			}
			break;
		}
		return dstbuf;
	}
#endif

#if GDISP_NEED_SCROLL && GDISP_DRIVER_SCROLL
	static void gdisp_lld_vertical_scroll(GDisplay *g) {
		RECT		rect;
		gCoord		lines;

		#if GDISP_NEED_CONTROL
			switch(g->g.Orientation) {
			case GDISP_ROTATE_0:
			default:
				rect.top = g->p.y;
				rect.bottom = rect.top+g->p.cy;
				rect.left = g->p.x;
				rect.right = rect.left+g->p.cx;
				lines = -g->p.y1;
				goto vertical_scroll;
			case GDISP_ROTATE_90:
				rect.bottom = g->g.Width - g->p.x;
				rect.top = rect.bottom-g->p.cx;
				rect.left = g->p.y;
				rect.right = rect.left+g->p.cy;
				lines = -g->p.y1;
				goto horizontal_scroll;
			case GDISP_ROTATE_180:
				rect.bottom = g->g.Height - g->p.y;
				rect.top = rect.bottom-g->p.cy;
				rect.right = g->g.Width - g->p.x;
				rect.left = rect.right-g->p.cx;
				lines = g->p.y1;
			vertical_scroll:
				if (lines > 0) {
					rect.bottom -= lines;
				} else {
					rect.top -= lines;
				}
				if (g->p.cy >= lines && g->p.cy >= -lines) {
					WaitForSingleObject(drawMutex, INFINITE);
					ScrollDC(myg->bpa.winDC, 0, lines, &rect, 0, 0, 0);
					#if !GDISP_FRAMEBUFFER_FORCEREDRAW
						ReleaseMutex(drawMutex);
						InvalidateRect(myg->bpa.hwnd, &rect, FALSE);
					#else
						{
							HDC		dc;
							dc = GetDC(myg->bpa.hwnd);
							ScrollDC(dc, 0, lines, &rect, 0, 0, 0);
							ReleaseDC(myg->bpa.hwnd, dc);
							ReleaseMutex(drawMutex);
						}
					#endif
				}
				break;
			case GDISP_ROTATE_270:
				rect.top = g->p.x;
				rect.bottom = rect.top+g->p.cx;
				rect.right = g->g.Height - g->p.y;
				rect.left = rect.right-g->p.cy;
				lines = g->p.y1;
			horizontal_scroll:
				if (lines > 0) {
					rect.right -= lines;
				} else {
					rect.left -= lines;
				}
				if (g->p.cy >= lines && g->p.cy >= -lines) {
					WaitForSingleObject(drawMutex, INFINITE);
					ScrollDC(myg->bpa.winDC, lines, 0, &rect, 0, 0, 0);
					#if !GDISP_FRAMEBUFFER_FORCEREDRAW
						ReleaseMutex(drawMutex);
						InvalidateRect(myg->bpa.hwnd, &rect, FALSE);
					#else
						{
							HDC		dc;
							dc = GetDC(myg->bpa.hwnd);
							ScrollDC(dc, lines, 0, &rect, 0, 0, 0);
							ReleaseDC(myg->bpa.hwnd, dc);
							ReleaseMutex(drawMutex);
						}
					#endif
				}
				break;
			}
		#else
			rect.top = g->p.y;
			rect.bottom = rect.top+g->p.cy;
			rect.left = g->p.x;
			rect.right = rect.left+g->p.cx;
			lines = -g->p.y1;
			if (lines > 0) {
				rect.bottom -= lines;
			} else {
				rect.top -= lines;
			}
			if (g->p.cy >= lines && g->p.cy >= -lines) {
				WaitForSingleObject(drawMutex, INFINITE);
				ScrollDC(myg->bpa.winDC, 0, lines, &rect, 0, 0, 0);
				#if !GDISP_FRAMEBUFFER_FORCEREDRAW
					ReleaseMutex(drawMutex);
					InvalidateRect(myg->bpa.hwnd, &rect, FALSE);
				#else
					{
						HDC		dc;
						dc = GetDC(myg->bpa.hwnd);
						ScrollDC(dc, 0, lines, &rect, 0, 0, 0);
						ReleaseDC(myg->bpa.hwnd, dc);
						ReleaseMutex(drawMutex);
					}
				#endif
			}
		#endif
	}
#endif

#if GDISP_NEED_CONTROL && GDISP_DRIVER_CONTROL
	static void gdisp_lld_control(GDisplay *g) {
		switch(g->p.x) {
		case GDISP_CONTROL_ORIENTATION:
			if (g->g.Orientation == (orientation_t)g->p.ptr)
				return;
			switch((orientation_t)g->p.ptr) {
				case GDISP_ROTATE_0:
				case GDISP_ROTATE_180:
					g->g.Width = GDISP_WIN32_WIDTH;
					g->g.Height = GDISP_WIN32_HEIGHT;
					break;
				case GDISP_ROTATE_90:
				case GDISP_ROTATE_270:
					g->g.Height = GDISP_WIN32_WIDTH;
					g->g.Width = GDISP_WIN32_HEIGHT;
					break;
				default:
					return;
			}
			g->g.Orientation = (orientation_t)g->p.ptr;
			return;
/*
		case GDISP_CONTROL_POWER:
		case GDISP_CONTROL_BACKLIGHT:
		case GDISP_CONTROL_CONTRAST:
*/
		}
	}
#endif

//--------------------------------------------------------------------------
// Cleanup our base driver template (framebuffer)
//--------------------------------------------------------------------------
#include "../../gdisp/gdisp_fb_rgb_unpacked.h"

// Cleanup all non-drivername tagged macros
#undef GDISP_WIN32_WINOFFSETX
#undef GDISP_WIN32_WINOFFSETY
#undef GDISP_WIN32_TITLE
#undef GDISP_WIN32_FLG_READY
#undef GDISP_WIN32_LINEBYTES
