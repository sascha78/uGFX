/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#define GDISP_DRIVER_NAME				GDISP_WIN32
#define GDISP_DRIVER_PIXELFORMAT		GDISP_PIXELFORMAT_BGR888
//#define GDISP_DRIVER_EXACTCOLORS		GFXON
//#define GDISP_DRIVER_DYNAMIC			GFXON
//#define GDISP_DRIVER_DEINIT			GFXON
#define GDISP_DRIVER_FLUSH				GFXON
//#define GDISP_DRIVER_SETPOS			GFXON
//#define GDISP_DRIVER_READ				GFXON
//#define GDISP_DRIVER_MOVE				GFXON
//#define GDISP_DRIVER_IOCTL			GFXON

// Calling gdispGFlush() is optional for this driver but can be used by the
//	application to force a display update. eg after streaming.

#ifdef GDISP_DRIVER_EXTRA_API
	/*
	 * Additional API Calls specific to this driver
	 */
	
	// This function allows you to specify the parent window for any ugfx display windows created.
	// Passing a NULL will reset window creation to creating top level windows.
	// Note: In order to affect any static displays it must be called BEFORE gfxInit().
	// Note: Creating a window under a parent causes the Mouse to be disabled by default (rather than enabled as for a top window)
	void gfxWin32SetParentWindow(void *hwnd);
	
	#if GFX_USE_GINPUT && GINPUT_NEED_MOUSE
		// This function allows you to inject mouse events into the ugfx mouse driver
		void gfxWin32MouseInject(GDisplay *g, uint16_t buttons, gCoord x, gCoord y);
	
		// This function enables you to turn on/off normal mouse functions on a ugfx Win32 display window.
		void gfxWin32MouseEnable(GDisplay *g, bool_t enabled);
	
		// This function enables you to capture mouse events on a ugfx Win32 display window.
		// Passing NULL turns off the capture
		void gfxWin32MouseCapture(GDisplay *g, void (*capfn)(void * hWnd, GDisplay *g, uint16_t buttons, gCoord x, gCoord y));
	#endif
#endif