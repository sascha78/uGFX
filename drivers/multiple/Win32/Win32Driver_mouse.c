/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

//--------------------------------------------------------------------------
// Configuration options for this driver
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// Mouse Driver
//--------------------------------------------------------------------------

// Include mouse support code
#define GMOUSE_DRIVER_VMT		GMOUSEVMT_Win32
#include "../../../src/ginput/ginput_driver_mouse.h"

// Forward definitions
static bool_t Win32MouseInit(GMouse *m, unsigned driverinstance);
static bool_t Win32MouseRead(GMouse *m, GMouseReading *prd);

const GMouseVMT const GMOUSE_DRIVER_VMT[1] = {{
	{
		GDRIVER_TYPE_MOUSE,
		GMOUSE_VFLG_NOPOLL|GMOUSE_VFLG_DYNAMICONLY,
			// Extra flags for testing only
			//GMOUSE_VFLG_TOUCH|GMOUSE_VFLG_SELFROTATION|GMOUSE_VFLG_DEFAULTFINGER
			//GMOUSE_VFLG_CALIBRATE|GMOUSE_VFLG_CAL_EXTREMES|GMOUSE_VFLG_CAL_TEST|GMOUSE_VFLG_CAL_LOADFREE
			//GMOUSE_VFLG_ONLY_DOWN|GMOUSE_VFLG_POORUPDOWN
		sizeof(GMouse),
		_gmouseInitDriver, _gmousePostInitDriver, _gmouseDeInitDriver
	},
	1,				// z_max
	0,				// z_min
	1,				// z_touchon
	0,				// z_touchoff
	{				// pen_jitter
		0,				// calibrate
		0,				// click
		0				// move
	},
	{				// finger_jitter
		0,				// calibrate
		2,				// click
		2				// move
	},
	Win32MouseInit,	// init
	0,				// deinit
	Win32MouseRead,	// get
	0,				// calsave
	0				// calload
}};

void gfxWin32MouseInject(GDisplay *g, uint16_t buttons, gCoord x, gCoord y) {
	myg->dpa.mousebuttons = buttons;
	myg->dpa.mousex = x;
	myg->dpa.mousey = y;
	if ((gmvmt(myg->dpa.mouse)->d.flags & GMOUSE_VFLG_NOPOLL))		// For normal setup this is always GTrue
		_gmouseWakeup(myg->dpa.mouse);
}

void gfxWin32MouseEnable(GDisplay *g, bool_t enabled) {
	myg->dpa.mouseenabled = enabled;
}

void gfxWin32MouseCapture(GDisplay *g, void (*capfn)(void * hWnd, GDisplay *g, uint16_t buttons, gCoord x, gCoord y)) {
	myg->dpa.capfn = capfn;
}

static bool_t Win32MouseInit(GMouse *m, unsigned driverinstance) {
	(void)	m;
	(void)	driverinstance;
	return GTrue;
}

static bool_t Win32MouseRead(GMouse *m, GMouseReading *pt) {
	GDisplay *	g;

	g = m->display;

	pt->x = myg->dpa.mousex;
	pt->y = myg->dpa.mousey;
	pt->z = (myg->dpa.mousebuttons & GINPUT_MOUSE_BTN_LEFT) ? 1 : 0;
	pt->buttons = myg->dpa.mousebuttons;

	#if GDISP_NEED_CONTROL
		// If the self-rotation has been set in the VMT then do that here (TESTING ONLY)
		if ((gmvmt(m)->d.flags & GMOUSE_VFLG_SELFROTATION)) {		// For normal setup this is always False
			gCoord		t;

			switch(gdispGGetOrientation(m->display)) {
				case GDISP_ROTATE_0:
				default:
					break;
				case GDISP_ROTATE_90:
					t = pt->x;
					pt->x = g->g.Width - 1 - pt->y;
					pt->y = t;
					break;
				case GDISP_ROTATE_180:
					pt->x = g->g.Width - 1 - pt->x;
					pt->y = g->g.Height - 1 - pt->y;
					break;
				case GDISP_ROTATE_270:
					t = pt->y;
					pt->y = g->g.Height - 1 - pt->x;
					pt->x = t;
					break;
			}
		}
	#endif

	return GTrue;
}

static bool_t Win32MouseMsgHook(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	uint16_t	btns;

	switch(msg) {
	case WM_LBUTTONDOWN:
		g = (GDisplay *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		btns = myg->dpa.mousebuttons;
		btns |= GINPUT_MOUSE_BTN_LEFT;
		goto mousemove;

	case WM_LBUTTONUP:
		g = (GDisplay *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		btns = myg->dpa.mousebuttons;
		btns &= ~GINPUT_MOUSE_BTN_LEFT;
		goto mousemove;

	case WM_MBUTTONDOWN:
		g = (GDisplay *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		btns = myg->dpa.mousebuttons;
		btns |= GINPUT_MOUSE_BTN_MIDDLE;
		goto mousemove;

	case WM_MBUTTONUP:
		g = (GDisplay *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		btns = myg->dpa.mousebuttons;
		btns &= ~GINPUT_MOUSE_BTN_MIDDLE;
		goto mousemove;

	case WM_RBUTTONDOWN:
		g = (GDisplay *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		btns = myg->dpa.mousebuttons;
		btns |= GINPUT_MOUSE_BTN_RIGHT;
		goto mousemove;

	case WM_RBUTTONUP:
		g = (GDisplay *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		btns = myg->dpa.mousebuttons;
		btns &= ~GINPUT_MOUSE_BTN_RIGHT;
		goto mousemove;

	case WM_MOUSEMOVE:
		g = (GDisplay *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		btns = myg->dpa.mousebuttons;

	mousemove:
		if (myg->dpa.capfn)
			myg->dpa.capfn(hWnd, g, btns, (gCoord)LOWORD(lParam), (gCoord)HIWORD(lParam));
		if (myg->dpa.mouseenabled) {
			myg->dpa.mousebuttons = btns;
			myg->dpa.mousex = (gCoord)LOWORD(lParam);
			myg->dpa.mousey = (gCoord)HIWORD(lParam);
			if ((gmvmt(myg->dpa.mouse)->d.flags & GMOUSE_VFLG_NOPOLL))		// For normal setup this is always GTrue
				_gmouseWakeup(myg->dpa.mouse);
		}
		return GTrue;
	}
	return GFalse;
}
