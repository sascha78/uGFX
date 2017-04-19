/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef GFX_IN_IMPLEMENTATION
	#define GFX_IN_IMPLEMENTATION	GFXON
	#include "../../gfx.h"
#endif

#if GFX_USE_GDISP

#include "../../drivers/gdisp/gdisp_drivers_private.h"

// Include the "Single File Make" compatible parts of uGFX

/* The very first thing that has to be compiled here is mf_font.c so that
 * inclusion of the font header files does not stop the inclusion of the
 * implementation specific parts of the font files.
 */
#include "mcufont/mf_font.c"
#include "mcufont/mf_rlefont.c"
#include "mcufont/mf_bwfont.c"
#include "mcufont/mf_scaledfont.c"
#include "mcufont/mf_encoding.c"
#include "mcufont/mf_justify.c"
#include "mcufont/mf_kerning.c"
#include "mcufont/mf_wordwrap.c"

#include "gdisp_fonts.c"
#include "gdisp_image.c"

/* Include the low level driver information */
//#include "gdisp_driver.h"

// Number of milliseconds for the startup logo - 0 means disabled.
#if GDISP_NEED_STARTUP_LOGO
	#define GDISP_STARTUP_LOGO_TIMEOUT		1000
	#define GDISP_STARTUP_LOGO_COLOR		GFXWHITE
#else
	#define GDISP_STARTUP_LOGO_TIMEOUT		0
#endif

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

#if GDISP_NEED_TIMERFLUSH
	static GTimer	FlushTimer;
#endif

GDisplay	*GDISP;

#if GDISP_NEED_MULTITHREAD
	#define MUTEX_INIT(g)		gfxMutexInit(&(g)->mutex)
	#define MUTEX_ENTER(g)		gfxMutexEnter(&(g)->mutex)
	#define MUTEX_EXIT(g)		gfxMutexExit(&(g)->mutex)
	#define MUTEX_DEINIT(g)		gfxMutexDestroy(&(g)->mutex)
#else
	#define MUTEX_INIT(g)
	#define MUTEX_ENTER(g)
	#define MUTEX_EXIT(g)
	#define MUTEX_DEINIT(g)
#endif

#if GDISP_NEED_VALIDATION || GDISP_NEED_CLIP
	#define TEST_CLIP_AREA(g)																				\
			if ((g)->p.pos.x < (g)->clip.p1.x) { (g)->p.cx -= (g)->clip.p1.x - (g)->p.pos.x; (g)->p.pos.x = (g)->clip.p1.x; }	\
			if ((g)->p.pos.y < (g)->clip.p1.y) { (g)->p.cy -= (g)->clip.p1.y - (g)->p.pos.y; (g)->p.pos.y = (g)->clip.p1.y; }	\
			if ((g)->p.pos.x + (g)->p.cx > (g)->clip.p2.x)	(g)->p.cx = (g)->clip.p2.x - (g)->p.pos.x;						\
			if ((g)->p.pos.y + (g)->p.cy > (g)->clip.p2.y)	(g)->p.cy = (g)->clip.p2.y - (g)->p.pos.y;						\
			if ((g)->p.cx > 0 && (g)->p.cy > 0)
#else
	#define TEST_CLIP_AREA(g)
#endif

#define SETWIN(g, x0, y0, x1, y1)	{ g->win.r.p1.x = (x0); g->win.r.p1.y = (y0); g->win.r.p2.x = (x1); g->win.r.p2.y = (y1); }
#define CHKWIN(g, x0, y0, x1, y1)	(g->win.r.p1.x > (x0) || g->win.r.p2.x < (x1) || g->win.r.p1.y > (y0) || g->win.r.p2.y < (y1))
#define SETPOS(g, x0, y0)			{ g->win.p.x = (x0); g->win.p.y = (y0); }
#define CHKPOS(g, x0, y0)			((x0) != g->win.p.x || (y0) != g->win.p.y)

#if GDISP_DRIVER_VMT_SETPOS == GFXON
	#define SETBLK(g, x0, y0)															\
		{																				\
			gdisp_lld_start(g);															\
			if (CHKPOS(g, (x0), (y0))) {												\
				SETPOS(g, (x0), (y0));													\
				gdisp_lld_setpos(g);													\
			}																			\
		}
	
	#define SETWINPOS(g, x0, y0, x1, y1, optx)											\
		{																				\
			if (CHKWIN(g, (x0), (y0), (x1), (y1))) {									\
				SETWIN(g, 0, 0, g->g.Width-1, g->g.Height-1);							\
				gdisp_lld_start(g);														\
			}																			\
			if (CHKPOS(g, (x0), (y0))) {												\
				SETPOS(g, (x0), (y0));													\
				gdisp_lld_setpos(g);													\
			}																			\
		}
	
#elif GDISP_DRIVER_VMT_SETPOS == GFXOFF
	#define SETBLK(g, x0, y0)		gdisp_lld_start(g);

	#define SETWINPOS(g, x0, y0, x1, y1, optx)											\
		{																				\
			if (CHKPOS(g, (x0), (y0)) || CHKWIN(g, (x0), (y0), (x1), (y1))) {			\
				SETWIN(g, (x0), (y0), (optx), g->g.Height-1);							\
				gdisp_lld_start(g);														\
			}																			\
		}
	
#else
	#define SETBLK(g, x0, y0)															\
		{																				\
			gdisp_lld_start(g);															\
			if (gvmt(g)->setpos && CHKPOS(g, (x0), (y0))) {								\
				SETPOS(g, (x0), (y0));													\
				gdisp_lld_setpos(g);													\
			}																			\
		}

	#define SETWINPOS(g, x0, y0, x1, y1, optx)											\
		{																				\
			if (gvmt(g)->setpos) {														\
				if (CHKWIN(g, (x0), (y0), (x1), (y1))) {								\
					SETWIN(g, 0, 0, g->g.Width-1, g->g.Height-1);						\
					gdisp_lld_start(g);													\
				}																		\
				if (CHKPOS(g, (x0), (y0))) {											\
					SETPOS(g, (x0), (y0));												\
					gdisp_lld_setpos(g);												\
				}																		\
			} else if (CHKPOS(g, (x0), (y0)) || CHKWIN(g, (x0), (y0), (x1), (y1))) {	\
				SETWIN(g, (x0), (y0), (optx), g->g.Height-1);							\
				gdisp_lld_start(g);														\
			}																			\
		}
#endif

/*==========================================================================*/
/* Internal functions.														*/
/*==========================================================================*/

#if GDISP_DRIVER_VMT_FLUSH == GFXSOME
	#define autoflush(g)	if (gvmt(g)->flush) gdisp_lld_flush(g)
#elif GDISP_DRIVER_VMT_FLUSH
	#define autoflush(g)	gdisp_lld_flush(g)
#else
	#define autoflush(g)
#endif

#if GDISP_NEED_ORIENTATION
	static void rot0pnt(gPoint *p) {
		(void)p;
	}
	static void rot0rect(gRect *r) {
		(void)r;
	}
	static void rot90pnt(gPoint *p) {
		//TODO
	}
	static void rot90rect(gRect *r) {
		//TODO
	}
	static void rot180pnt(gPoint *p) {
		//TODO
	}
	static void rot180rect(gRect *r) {
		//TODO
	}
	static void rot270pnt(gPoint *p) {
		//TODO
	}
	static void rot270rect(gRect *r) {
		//TODO
	}
#endif

#if GDISP_DRIVER_VMT_SETPOS != GFXON
	static void nsp_drawpixel(GDisplay *g) {
		if (CHKPOS(g, g->p.pos.x, g->p.pos.y) || CHKWIN(g, g->p.pos.x, g->p.pos.y, g->p.pos.x, g->p.pos.y)) {
			SETWIN(g, g->p.pos.x, g->p.pos.y, g->g.Width-1, g->g.Height-1);
			gdisp_lld_start(g);
		}
		g->p.e.cnt = 1;
		gdisp_lld_write(g);
	}
	static void nsp_fill(GDisplay *g) {
		gdisp_lld_start(g);
		
		// We write at most one horizontal line at a time - this makes it easier for the driver to calculate positions
		g->p.e.cnt = g->win.r.p2.x - g->win.r.p1.x + 1;
		do {
			gdisp_lld_write(g);
		} while (g->win.p.y != g->win.r.p1.y);
	}
	static void nsp_hline(GDisplay *g) {
		if (CHKPOS(g, g->p.pos.x, g->p.pos.y) || CHKWIN(g, g->p.pos.x, g->p.pos.y, g->p.e.pos2.x, g->p.e.pos2.y)) {
			SETWIN(g, g->p.pos.x, g->p.pos.y, g->p.e.pos2.x, g->g.Height-1);
			gdisp_lld_start(g);
		}
		g->p.e.cnt = g->p.e.pos2.x - g->p.pos.x + 1;
		gdisp_lld_write(g);
	}
#endif
#if GDISP_DRIVER_VMT_SETPOS != GFXOFF
	static void wsp_drawpixel(GDisplay *g) {
		if (CHKWIN(g, g->p.pos.x, g->p.pos.y, g->p.pos.x, g->p.pos.y)) {
			SETWIN(g, 0, 0, g->g.Width-1, g->g.Height-1);
			gdisp_lld_start(g);
		}
		if (CHKPOS(g, g->p.pos.x, g->p.pos.y)) {
			SETPOS(g, g->p.pos.x, g->p.pos.y);
			gdisp_lld_setpos(g);
		}
		g->p.e.cnt = 1;
		gdisp_lld_write(g);
	}
	static void wsp_fill(GDisplay *g) {
		gdisp_lld_start(g);
		if (CHKPOS(g, g->win.r.p1.x, g->win.r.p1.y)) {
			SETPOS(g, g->win.r.p1.x, g->win.r.p1.y);
			gdisp_lld_setpos(g);
		}
		
		// We write at most one horizontal line at a time - this makes it easier for the driver to calculate positions
		g->p.e.cnt = g->win.r.p2.x - g->win.r.p1.x + 1;
		do {
			gdisp_lld_write(g);
		} while (g->win.p.y != g->win.r.p1.y);
	}
	static void wsp_hline(GDisplay *g) {
		if (CHKWIN(g, g->p.pos.x, g->p.pos.y, g->p.e.pos2.x, g->p.e.pos2.y)) {
			SETWIN(g, 0, 0, g->g.Width-1, g->g.Height-1);
			gdisp_lld_start(g);
		}
		if (CHKPOS(g, g->p.pos.x, g->p.pos.y)) {
			SETPOS(g, g->p.pos.x, g->p.pos.y);
			gdisp_lld_setpos(g);
		}
		g->p.e.cnt = g->p.e.pos2.x - g->p.pos.x + 1;
		gdisp_lld_write(g);
	}
#endif

static void xsp_vline(GDisplay *g) {
	// Do a fill area of the 1 pixel wide
	SETWIN(g, g->p.pos.x, g->p.pos.y, g->p.pos.x, g->p.e.pos2.y);						\
	g->f_fill(g);	
}

// drawpixel_clip(g)
// Parameters:	p.pos, p.color
// Alters:		win, p.e
static void drawpixel_clip(GDisplay *g) {
	#if GDISP_NEED_VALIDATION || GDISP_NEED_CLIP
		if (g->p.pos.x < g->clip.p1.x || g->p.pos.x >= g->clip.p2.x || g->p.pos.y < g->clip.p1.y || g->p.pos.y >= g->clip.p2.y)
			return;
	#endif

	g->f_pnt(g);
}

// fillarea(g)
// Parameters:	win and p.color
// Alters:		p.e
// Note:		This is not clipped
static GFXINLINE void fillarea(GDisplay *g) {
	// Rotate the rect in here
	
	g->f_fill(g);
}

// Parameters:	p.pos, p.e.pos2.x, p.color
// Alters:		win, p.pos, p.e
static void hline_clip(GDisplay *g) {
	// Swap the points if necessary so it always goes from x to x1
	if (g->p.e.pos2.x < g->p.pos.x) {
		g->p.e.pos2.y = g->p.pos.x; g->p.pos.x = g->p.e.pos2.x; g->p.e.pos2.x = g->p.e.pos2.y;
	}

	// Clipping
	#if GDISP_NEED_VALIDATION || GDISP_NEED_CLIP
		if (g->p.pos.y < g->clip.p1.y || g->p.pos.y >= g->clip.p2.y) return;
		if (g->p.pos.x < g->clip.p1.x) g->p.pos.x = g->clip.p1.x;
		if (g->p.e.pos2.x >= g->clip.p2.x) g->p.e.pos2.x = g->clip.p2.x - 1;
		if (g->p.e.pos2.x < g->p.pos.x) return;
	#endif

	g->f_hline(g);
}

// Parameters:	p.pos, p.e.pos2.y, p.color
// Alters:		win, p.e
static void vline_clip(GDisplay *g) {
	// Swap the points if necessary so it always goes from y to y1
	if (g->p.e.pos2.y < g->p.pos.y) {
		g->p.e.pos2.x = g->p.pos.y; g->p.pos.y = g->p.e.pos2.y; g->p.e.pos2.y = g->p.e.pos2.x;
	}

	// Clipping
	#if GDISP_NEED_VALIDATION || GDISP_NEED_CLIP
		if (g->p.pos.x < g->clip.p1.x || g->p.pos.x >= g->clip.p2.x) return;
		if (g->p.pos.y < g->clip.p1.y) g->p.pos.y = g->clip.p1.y;
		if (g->p.e.pos2.y >= g->clip.p2.y) g->p.e.pos2.y = g->clip.p2.y - 1;
		if (g->p.e.pos2.y < g->p.pos.y) return;
	#endif

	// Optimise a single pixel draw
	// NB: The optimisation here is not the processing for this line but the better end effect it has for subsequent operations
	if (g->p.pos.y == g->p.e.pos2.y) {
		g->f_pnt(g);
		return;
	}

	g->f_vline(g);
}

// Parameters:	p.pos, p.e.pos2, p.color
// Alters:		win, p.e
static void line_clip(GDisplay *g) {
	int16_t dy, dx;
	int16_t addx, addy;
	int16_t P, diff, i;

	// Is this a horizontal line (or a point)
	if (g->p.pos.y == g->p.e.pos2.y) {
		hline_clip(g);
		return;
	}

	// Is this a vertical line (or a point)
	if (g->p.pos.x == g->p.e.pos2.x) {
		vline_clip(g);
		return;
	}

	// Not horizontal or vertical

	// Use Bresenham's line drawing algorithm.
	//	This should be replaced with fixed point slope based line drawing
	//	which is more efficient on modern processors as it branches less.
	//	When clipping is needed, all the clipping could also be done up front
	//	instead of on each pixel.

	if (g->p.e.pos2.x >= g->p.pos.x) {
		dx = g->p.e.pos2.x - g->p.pos.x;
		addx = 1;
	} else {
		dx = g->p.pos.x - g->p.e.pos2.x;
		addx = -1;
	}
	if (g->p.e.pos2.y >= g->p.pos.y) {
		dy = g->p.e.pos2.y - g->p.pos.y;
		addy = 1;
	} else {
		dy = g->p.pos.y - g->p.e.pos2.y;
		addy = -1;
	}

	if (dx >= dy) {
		dy <<= 1;
		P = dy - dx;
		diff = P - dx;

		for(i=0; i<=dx; ++i) {
			drawpixel_clip(g);
			if (P < 0) {
				P  += dy;
				g->p.pos.x += addx;
			} else {
				P  += diff;
				g->p.pos.x += addx;
				g->p.pos.y += addy;
			}
		}
	} else {
		dx <<= 1;
		P = dx - dy;
		diff = P - dy;

		for(i=0; i<=dy; ++i) {
			drawpixel_clip(g);
			if (P < 0) {
				P  += dx;
				g->p.pos.y += addy;
			} else {
				P  += diff;
				g->p.pos.x += addx;
				g->p.pos.y += addy;
			}
		}
	}
}

#if GDISP_STARTUP_LOGO_TIMEOUT > 0
	static bool_t	gdispInitDone;
	static void StartupLogoDisplay(GDisplay *g) {
		gCoord			x, y, w;
		const gCoord *	p;
		static const gCoord blks[] = {
				// u
				2, 6, 1, 10,
				3, 11, 4, 1,
				6, 6, 1, 6,
				// G
				8, 0, 1, 12,
				9, 0, 6, 1,
				9, 11, 6, 1,
				14, 6, 1, 5,
				12, 6, 2, 1,
				// F
				16, 0, 1, 12,
				17, 0, 6, 1,
				17, 6, 3, 1,
				// X
				22, 6, 7, 1,
				24, 0, 1, 6,
				22, 7, 1, 5,
				28, 0, 1, 6,
				26, 7, 1, 5,
		};

		// Get a starting position and a scale
		// Work on a 8x16 grid for each char, 4 chars (uGFX) in 1 line, using half the screen
		w = g->g.Width/(8*4*2);
		if (!w) w = 1;
		x = (g->g.Width - (8*4)*w)/2;
		y = (g->g.Height - (16*1)*w)/2;

		// Simple but crude!
		for(p = blks; p < blks+sizeof(blks)/sizeof(blks[0]); p+=4)
			gdispGFillArea(g, x+p[0]*w, y+p[1]*w, p[2]*w, p[3]*w, GDISP_STARTUP_LOGO_COLOR);
	}
#endif

#if GDISP_NEED_TIMERFLUSH
	static void FlushTimerFn(void *param) {
		GDisplay *	g;
		(void)		param;

		for(g = (GDisplay *)gdriverGetNext(GDRIVER_TYPE_DISPLAY, 0); g; g = (GDisplay *)gdriverGetNext(GDRIVER_TYPE_DISPLAY, (GDriver *)g))
			gdispGFlush(g);
	}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

void _gdispInit(void)
{
	const GDISPVMT **p;
	unsigned cnt;
	
	for(p = GDISP_DRIVER_LIST; *p; p++) {
		if (!(p[0]->d.flags & GDISP_VFLG_DYNAMIC)) {
			for(cnt = p[0]->count(p[0]); cnt; cnt--)
				gdriverRegister(&p[0]->d, 0);
		}
	}

	// Re-clear the display after the timeout if we added the logo
	#if GDISP_STARTUP_LOGO_TIMEOUT > 0
		{
			GDisplay	*g;

			gfxSleepMilliseconds(GDISP_STARTUP_LOGO_TIMEOUT);

			for(g = (GDisplay *)gdriverGetNext(GDRIVER_TYPE_DISPLAY, 0); g; g = (GDisplay *)gdriverGetNext(GDRIVER_TYPE_DISPLAY, (GDriver *)g)) {
				gdispGClear(g, GDISP_STARTUP_COLOR);
				#if GDISP_DRIVER_FLUSH
					gdispGFlush(g);
				#endif
			}

			gdispInitDone = GTrue;
		}
	#endif

	// Start the automatic timer flush (if required)
	#if GDISP_NEED_TIMERFLUSH
		gtimerInit(&FlushTimer);
		gtimerStart(&FlushTimer, FlushTimerFn, 0, GTrue, GDISP_NEED_TIMERFLUSH);
	#endif
}

void _gdispDeinit(void)
{
	/* ToDo */
}

bool_t _gdispInitDriver(GDriver *g, void *param, unsigned driverinstance, unsigned systeminstance) {
	#define		gd		((GDisplay *)g)
	bool_t		ret;

	// Intialise fields
	gd->systemdisplay = systeminstance;
	gd->controllerdisplay = driverinstance;
	gd->flags = 0;
	gd->p.e.ptr = param;
	MUTEX_INIT(gd);

	// Call the driver init
	MUTEX_ENTER(gd);
	ret = gdisp_lld_init(gd);
	MUTEX_EXIT(gd);
	
	return ret;

	#undef gd
}

void _gdispPostInitDriver(GDriver *g) {
	#define		gd		((GDisplay *)g)

	// Set up the initial drawing routines
	// TODO - orientation handling
	#if GDISP_DRIVER_VMT_SETPOS == GFXON
		gd->f_pnt	= wsp_drawpixel;
		gd->f_fill	= wsp_fill;
		gd->f_hline = wsp_hline;
		gd->f_vline = xsp_vline;
	#elif GDISP_DRIVER_VMT_SETPOS == GFXOFF
		gd->f_pnt	= nsp_drawpixel;
		gd->f_fill	= nsp_fill;
		gd->f_hline = nsp_hline;
		gd->f_vline = xsp_vline;
	#else
		if (gvmt(gd)->setpos) {														\
			gd->f_pnt	= wsp_drawpixel;
			gd->f_fill	= wsp_fill;
			gd->f_hline = wsp_hline;
			gd->f_vline = xsp_vline;
		} else {
			gd->f_pnt	= nsp_drawpixel;
			gd->f_fill	= nsp_fill;
			gd->f_hline = nsp_hline;
			gd->f_vline = xsp_vline;
		}
	#endif

	// Set orientation, clip
	#if defined(GDISP_DEFAULT_ORIENTATION) && GDISP_NEED_ORIENTATION && GDISP_DRIVER_CONTROL
		#if GDISP_NEED_PIXMAP
			// Pixmaps should stay in their created orientation (at least initially)
			if (!(gvmt(gd)->d.flags & GDISP_VFLG_PIXMAP))
		#endif
			gdispGControl(gd, GDISP_CONTROL_ORIENTATION, (void *)GDISP_DEFAULT_ORIENTATION);
	#endif
	#if GDISP_NEED_ORIENTATION
		switch(gd->g.Orientation)
		case GDISP_ROTATE_0:
		default:
			gd->f_rotpnt	= rot0pnt;
			gd->f_rotrect	= rot0rect;
			break;
		case GDISP_ROTATE_90:
			gd->f_rotpnt	= rot90pnt;
			gd->f_rotrect	= rot90rect;
			break;
		case GDISP_ROTATE_180:
			gd->f_rotpnt	= rot180pnt;
			gd->f_rotrect	= rot180rect;
			break;
		case GDISP_ROTATE_270:
			gd->f_rotpnt	= rot270pnt;
			gd->f_rotrect	= rot270rect;
			break;
		}
	#endif
	#if GDISP_NEED_VALIDATION || GDISP_NEED_CLIP
		gdispGSetClip(gd, 0, 0, gd->g.Width, gd->g.Height);
	#endif

	// Clear the Screen
	gdispGClear(gd, GDISP_STARTUP_COLOR);

	// Display the startup logo if this is a static initialised display
	#if GDISP_STARTUP_LOGO_TIMEOUT > 0
		if (!gdispInitDone)
			StartupLogoDisplay(gd);
	#endif

	// Flush
	#if GDISP_DRIVER_FLUSH
		gdispGFlush(gd);
	#endif

	// If this is the first driver set GDISP
	if (!GDISP)
		GDISP = gd;

	#undef gd
}

void _gdispDeInitDriver(GDriver *g) {
	#define		gd		((GDisplay *)g)

	if (GDISP == gd)
		GDISP = (GDisplay *)gdriverGetInstance(GDRIVER_TYPE_DISPLAY, 0);

	#if GDISP_DRIVER_DEINIT
		#if GDISP_DRIVER_DEINIT == GFXSOME
			if (gvmt(gd)->deinit)
		#endif
		{
			MUTEX_ENTER(gd);
			gdisp_lld_deinit(gd);
			MUTEX_EXIT(gd);
		}
	#endif
	MUTEX_DEINIT(gd);

	#undef gd
}

GDisplay *gdispGetDisplay(unsigned display) {
	return (GDisplay *)gdriverGetInstance(GDRIVER_TYPE_DISPLAY, display);
}

void gdispSetDisplay(GDisplay *g) {
	if (g) GDISP = g;
}

unsigned gdispGetDisplayCount(void) {
	return gdriverInstanceCount(GDRIVER_TYPE_DISPLAY);
}

gCoord gdispGGetWidth(GDisplay *g)				{ return g->g.Width; }
gCoord gdispGGetHeight(GDisplay *g)				{ return g->g.Height; }
powermode_t gdispGGetPowerMode(GDisplay *g)		{ return g->g.Powermode; }
orientation_t gdispGGetOrientation(GDisplay *g)	{ return g->g.Orientation; }
uint8_t gdispGGetBacklight(GDisplay *g)			{ return g->g.Backlight; }
uint8_t gdispGGetContrast(GDisplay *g)			{ return g->g.Contrast; }

void gdispGFlush(GDisplay *g) {
	#if GDISP_DRIVER_FLUSH
		#if GDISP_DRIVER_FLUSH == GFXSOME
			if (gvmt(g)->flush)
		#endif
		{
			MUTEX_ENTER(g);
			//if (((g)->flags & GDISP_FLG_FLUSHREQ))
				gdisp_lld_flush(g);
			MUTEX_EXIT(g);
		}
	#else
		(void) g;
	#endif
}

#if GDISP_NEED_STREAMING
	void gdispGStreamStart(GDisplay *g, gCoord x, gCoord y, gCoord cx, gCoord cy) {
		MUTEX_ENTER(g);

		#if GDISP_NEED_VALIDATION || GDISP_NEED_CLIP
			// Test if the area is valid - if not then exit
			if (x < g->clip.p1.x || x+cx > g->clip.p2.x || y < g->clip.p1.y || y+cy > g->clip.p2.y) {
				MUTEX_EXIT(g);
				return;
			}
		#endif

		g->flags |= GDISP_FLG_INSTREAM;

		SETWIN(g, x, y, x+cx-1, x+cy-1);
		SETBLK(g, x, y);

		// Don't release the mutex as gdispStreamEnd() will do that.
	}

	void gdispGStreamColor(GDisplay *g, gColor color) {
		#if !GDISP_DRIVER_STREAM_WRITE && GDISP_LINEBUF_SIZE != 0 && GDISP_DRIVER_BITFILLS
			gCoord	 sx1, sy1;
		#endif

		// Don't touch the mutex as we should already own it

		// Ignore this call if we are not streaming
		if (!(g->flags & GDISP_FLG_INSTREAM))
			return;

		g->p.color = color;
		g->p.e.cnt = 1;
		gdisp_lld_write(g);
	}

	void gdispGStreamStop(GDisplay *g) {
		// Only release the mutex and end the stream if we are actually streaming.
		if (!(g->flags & GDISP_FLG_INSTREAM))
			return;

		// Clear the flag
		g->flags &= ~GDISP_FLG_INSTREAM;

		autoflush(g);
		MUTEX_EXIT(g);
	}
#endif

void gdispGDrawPixel(GDisplay *g, gCoord x, gCoord y, gColor color) {
	MUTEX_ENTER(g);
	g->p.color	= color;
	g->p.pos.x	= x;
	g->p.pos.y	= y;
	drawpixel_clip(g);
	autoflush(g);
	MUTEX_EXIT(g);
}

void gdispGDrawLine(GDisplay *g, gCoord x0, gCoord y0, gCoord x1, gCoord y1, gColor color) {
	MUTEX_ENTER(g);
	g->p.color		= color;
	g->p.pos.x		= x0;
	g->p.pos.y		= y0;
	g->p.e.pos2.x	= x1;
	g->p.e.pos2.y	= y1;
	line_clip(g);
	autoflush(g);
	MUTEX_EXIT(g);
}

void gdispGClear(GDisplay *g, gColor color) {
	// Note - clear() ignores the clipping area. It clears the screen.
	MUTEX_ENTER(g);
	g->p.color = color;
	SETWIN(g, 0, 0, g->g.Width-1, g->g.Height-1);
	fillarea(g);
	autoflush(g);
	MUTEX_EXIT(g);
}

void gdispGFillArea(GDisplay *g, gCoord x, gCoord y, gCoord cx, gCoord cy, gColor color) {
	MUTEX_ENTER(g);

	#if GDISP_NEED_VALIDATION || GDISP_NEED_CLIP
		if (x < g->clip.p1.x)		{ cx -= g->clip.p1.x - x; x = g->clip.p1.x; }
		if (y < g->clip.p1.y)		{ cy -= g->clip.p1.y - y; y = g->clip.p1.y; }
		if (x+cx > g->clip.p2.x)	cx = g->clip.p2.x - x;
		if (y+cy > g->clip.p2.y)	cy = g->clip.p2.y - y;
		if (cx <= 0 || cy <= 0) 	{ MUTEX_EXIT(g); return; }
	#endif

	g->p.color = color;
	SETWIN(g, x, y, x+cx-1, y+cy-1);
	fillarea(g);
	autoflush(g);
	MUTEX_EXIT(g);
}

void gdispGBlitArea(GDisplay *g, gCoord x, gCoord y, gCoord cx, gCoord cy, gCoord srcx, gCoord srcy, gCoord srccx, const gPixel *buffer) {
	MUTEX_ENTER(g);

	#if GDISP_NEED_VALIDATION || GDISP_NEED_CLIP
		// This is a different clipping to fillarea(g) as it needs to take into account srcx,srcy
		if (x < g->clip.p1.x) { cx -= g->clip.p1.x - x; srcx += g->clip.p1.x - x; x = g->clip.p1.x; }
		if (y < g->clip.p1.y) { cy -= g->clip.p1.y - y; srcy += g->clip.p1.y - x; y = g->clip.p1.y; }
		if (x+cx > g->clip.p2.x)	cx = g->clip.p2.x - x;
		if (y+cy > g->clip.p2.y)	cy = g->clip.p2.y - y;
		if (srcx+cx > srccx) cx = srccx - srcx;
		if (cx <= 0 || cy <= 0) { MUTEX_EXIT(g); return; }
	#endif

	// Translate buffer to the real image data, use srcx,srcy as the end point, srccx as the buffer line gap
	buffer += srcy*srccx+srcx;
	srcx = x+cx-1;
	srcy = y+cy-1;
	g->p.e.cnt = 1;

	// If cy == 1 we can potentially optimise following operations
	if (cy == 1) {
		
		SETWINPOS(g, x, y, srcx, srcy, g->g.Width-1);
		do {
			g->p.color = *buffer++;
			gdisp_lld_write(g);
		} while(--cx);
		
	} else {
	
		srccx -= cx;					// srccx now contains the inter-line gap

		SETWIN(g, x, y, srcx, srcy);
		SETBLK(g, x, y);
		do {
			do {
				g->p.color = *buffer++;
				gdisp_lld_write(g);
			} while (g->win.p.x != x);
			buffer += srccx;
		} while(g->win.p.x != y);
	}
	
	autoflush(g);
	MUTEX_EXIT(g);
}

#if GDISP_NEED_CLIP || GDISP_NEED_VALIDATION
	void gdispGSetClip(GDisplay *g, gCoord x, gCoord y, gCoord cx, gCoord cy) {
		MUTEX_ENTER(g);
		if (x < 0) { cx += x; x = 0; }
		if (y < 0) { cy += y; y = 0; }
		if (cx <= 0 || cy <= 0 || x >= g->g.Width || y >= g->g.Height) { x = y = cx = cy = 0; }
		g->clip.p1.x = x;
		g->clip.p1.y = y;
		g->clip.p2.x = x+cx;	if (g->clip.p2.x > g->g.Width) g->clip.p2.x = g->g.Width;
		g->clip.p2.y = y+cy;	if (g->clip.p2.y > g->g.Height) g->clip.p2.y = g->g.Height;
		MUTEX_EXIT(g);
	}
#endif

#if GDISP_NEED_CIRCLE
	void gdispGDrawCircle(GDisplay *g, gCoord x, gCoord y, gCoord radius, gColor color) {
		gCoord a, b, P;

		MUTEX_ENTER(g);

		// Calculate intermediates
		a = 1;
		b = radius;
		P = 4 - radius;
		g->p.color = color;

		// Away we go using Bresenham's circle algorithm
		// Optimized to prevent double drawing
		g->p.pos.x = x; g->p.pos.y = y + b; drawpixel_clip(g);
		g->p.pos.x = x; g->p.pos.y = y - b; drawpixel_clip(g);
		g->p.pos.x = x + b; g->p.pos.y = y; drawpixel_clip(g);
		g->p.pos.x = x - b; g->p.pos.y = y; drawpixel_clip(g);
		do {
			g->p.pos.x = x + a; g->p.pos.y = y + b; drawpixel_clip(g);
			g->p.pos.x = x + a; g->p.pos.y = y - b; drawpixel_clip(g);
			g->p.pos.x = x + b; g->p.pos.y = y + a; drawpixel_clip(g);
			g->p.pos.x = x - b; g->p.pos.y = y + a; drawpixel_clip(g);
			g->p.pos.x = x - a; g->p.pos.y = y + b; drawpixel_clip(g);
			g->p.pos.x = x - a; g->p.pos.y = y - b; drawpixel_clip(g);
			g->p.pos.x = x + b; g->p.pos.y = y - a; drawpixel_clip(g);
			g->p.pos.x = x - b; g->p.pos.y = y - a; drawpixel_clip(g);
			if (P < 0)
				P += 3 + 2*a++;
			else
				P += 5 + 2*(a++ - b--);
		} while(a < b);
		g->p.pos.x = x + a; g->p.pos.y = y + b; drawpixel_clip(g);
		g->p.pos.x = x + a; g->p.pos.y = y - b; drawpixel_clip(g);
		g->p.pos.x = x - a; g->p.pos.y = y + b; drawpixel_clip(g);
		g->p.pos.x = x - a; g->p.pos.y = y - b; drawpixel_clip(g);

		autoflush(g);
		MUTEX_EXIT(g);
	}
#endif

#if GDISP_NEED_CIRCLE
	void gdispGFillCircle(GDisplay *g, gCoord x, gCoord y, gCoord radius, gColor color) {
		gCoord a, b, P;

		MUTEX_ENTER(g);

		// Calculate intermediates
		a = 1;
		b = radius;
		P = 4 - radius;
		g->p.color = color;

		// Away we go using Bresenham's circle algorithm
		// This is optimized to prevent overdrawing by drawing a line only when a variable is about to change value
		g->p.pos.y = y; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);
		g->p.pos.y = y+b; g->p.pos.x = x; drawpixel_clip(g);
		g->p.pos.y = y-b; g->p.pos.x = x; drawpixel_clip(g);
		do {
			g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);
			g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);
			if (P < 0) {
				P += 3 + 2*a++;
			} else {
				g->p.pos.y = y+b; g->p.pos.x = x-a; g->p.e.pos2.x = x+a; hline_clip(g);
				g->p.pos.y = y-b; g->p.pos.x = x-a; g->p.e.pos2.x = x+a; hline_clip(g);
				P += 5 + 2*(a++ - b--);
			}
		} while(a < b);
		g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);
		g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);

		autoflush(g);
		MUTEX_EXIT(g);
	}
#endif

#if GDISP_NEED_DUALCIRCLE

	#define DRAW_DUALLINE(yval, r1, r2) 										\
		g->p.pos.y = yval;															\
		g->p.pos.x = x-r1;   g->p.e.pos2.x = x-r2+1; hline_clip(g);						\
		g->p.pos.x = x-r2;   g->p.e.pos2.x = x+r2;   g->p.color = color2; hline_clip(g);	\
		g->p.pos.x = x+r2+1; g->p.e.pos2.x = x+r1;   g->p.color = color1; hline_clip(g)
	#define DRAW_SINGLELINE(yval, r)	g->p.pos.y = yval; g->p.pos.x = x-r; g->p.e.pos2.x = x+r; hline_clip(g)

	void gdispGFillDualCircle(GDisplay *g, gCoord x, gCoord y, gCoord radius1, gColor color1, gCoord radius2, gColor color2) {
		gCoord a, b1, b2, p1, p2;

		MUTEX_ENTER(g);

		// Do the combined circle where the inner circle < 45 deg (and outer circle)
		g->p.color = color1;
		a = 0; b1 = radius1; b2 = radius2; p1 = p2 = 1;
		do {
			DRAW_DUALLINE(y+a, b1, b2);
			DRAW_DUALLINE(y-a, b1, b2);
			if (p1 >= 0) p1 -= b1--;
			p1 += a;
			if (p2 >= 0) p2 -= b2--;
			p2 += a;
		} while(++a < b2);

		// Do the combined circle where inner circle > 45 deg, outer circle < 45
		do {
			DRAW_DUALLINE(y+a, b1, b2);
			DRAW_DUALLINE(y-a, b1, b2);
			if (p1 >= 0) p1 -= b1--;
			p1 += a;
			do { p2 -= --b2; } while (p2+a >= b2);
			p2 += a;
		} while(++a <= radius2 && a < b1);
		
		if (a < radius2) {
			// Do the combined circle where inner circle > 45 deg, outer circle > 45
			do {
				DRAW_DUALLINE(y+a, b1, b2);
				DRAW_DUALLINE(y-a, b1, b2);
				do { p1 -= --b1; } while (p1+a >= b1);
				p1 += a;
				do { p2 -= --b2; } while (p2+a >= b2);
				p2 += a++;
			} while(b2 > 0);
			
		} else {
			// Do the outer circle above the inner circle but < 45 deg
			do {
				DRAW_SINGLELINE(y+a, b1);
				DRAW_SINGLELINE(y-a, b1);
				if (p1 >= 0) p1 -= b1--;
				p1 += a++;
			} while(a < b1);
			DRAW_SINGLELINE(y+a, b1);
			DRAW_SINGLELINE(y-a, b1);
		}

		// Do the top and bottom part of the outer circle (outer circle > 45deg and above inner circle)
		a = 0; b1 = radius1; p1 = 1;
		do {
			if (p1 >= 0) {
				DRAW_SINGLELINE(y+b1, a);
				DRAW_SINGLELINE(y-b1, a);
				p1 -= b1--;
			}
			p1 += a++;
		} while(b1 > radius2 && a < b1);

		autoflush(g);
		MUTEX_EXIT(g);
	}
	#undef DRAW_DUALLINE
	#undef DRAW_SINGLELINE
#endif

#if GDISP_NEED_ELLIPSE
	void gdispGDrawEllipse(GDisplay *g, gCoord x, gCoord y, gCoord a, gCoord b, gColor color) {
		gCoord	dx, dy;
		int32_t	a2, b2;
		int32_t	err, e2;

		MUTEX_ENTER(g);

		// Calculate intermediates
		dx = 0;
		dy = b;
		a2 = a*a;
		b2 = b*b;
		err = b2-(2*b-1)*a2;
		g->p.color = color;

		// Away we go using Bresenham's ellipse algorithm
		do {
			g->p.pos.x = x + dx; g->p.pos.y = y + dy; drawpixel_clip(g);
			g->p.pos.x = x - dx; g->p.pos.y = y + dy; drawpixel_clip(g);
			g->p.pos.x = x - dx; g->p.pos.y = y - dy; drawpixel_clip(g);
			g->p.pos.x = x + dx; g->p.pos.y = y - dy; drawpixel_clip(g);

			e2 = 2*err;
			if(e2 <  (2*dx+1)*b2) {
				dx++;
				err += (2*dx+1)*b2;
			}
			if(e2 > -(2*dy-1)*a2) {
				dy--;
				err -= (2*dy-1)*a2;
			}
		} while(dy >= 0);

		autoflush(g);
		MUTEX_EXIT(g);
	}
#endif

#if GDISP_NEED_ELLIPSE
	void gdispGFillEllipse(GDisplay *g, gCoord x, gCoord y, gCoord a, gCoord b, gColor color) {
		gCoord	dx, dy;
		int32_t	a2, b2;
		int32_t	err, e2;

		MUTEX_ENTER(g);

		// Calculate intermediates
		dx = 0;
		dy = b;
		a2 = a*a;
		b2 = b*b;
		err = b2-(2*b-1)*a2;
		g->p.color = color;

		// Away we go using Bresenham's ellipse algorithm
		// This is optimized to prevent overdrawing by drawing a line only when a y is about to change value
		do {
			e2 = 2*err;
			if(e2 <  (2*dx+1)*b2) {
				dx++;
				err += (2*dx+1)*b2;
			}
			if(e2 > -(2*dy-1)*a2) {
				g->p.pos.y = y + dy; g->p.pos.x = x - dx; g->p.e.pos2.x = x + dx; hline_clip(g);
				if (y) { g->p.pos.y = y - dy; g->p.pos.x = x - dx; g->p.e.pos2.x = x + dx; hline_clip(g); }
				dy--;
				err -= (2*dy-1)*a2;
			}
		} while(dy >= 0);

		autoflush(g);
		MUTEX_EXIT(g);
	}
#endif

#if GDISP_NEED_ARCSECTORS
	void gdispGDrawArcSectors(GDisplay *g, gCoord x, gCoord y, gCoord radius, uint8_t sectors, gColor color) {
		gCoord a, b, P;

		MUTEX_ENTER(g);

		// Calculate intermediates
		a = 1;              // x in many explanations
		b = radius;         // y in many explanations
		P = 4 - radius;
		g->p.color = color;

		// Away we go using Bresenham's circle algorithm
		// Optimized to prevent double drawing
		if (sectors & 0x06) { g->p.pos.x = x; g->p.pos.y = y - b; drawpixel_clip(g); }				// Upper upper
		if (sectors & 0x60) { g->p.pos.x = x; g->p.pos.y = y + b; drawpixel_clip(g); }				// Lower lower
		if (sectors & 0x81) { g->p.pos.x = x + b; g->p.pos.y = y; drawpixel_clip(g); }				// Right right
		if (sectors & 0x18) { g->p.pos.x = x - b; g->p.pos.y = y; drawpixel_clip(g); }				// Left left

		do {
			if (sectors & 0x01) { g->p.pos.x = x + b; g->p.pos.y = y - a; drawpixel_clip(g); }		// Upper right right
			if (sectors & 0x02) { g->p.pos.x = x + a; g->p.pos.y = y - b; drawpixel_clip(g); }		// Upper upper right
			if (sectors & 0x04) { g->p.pos.x = x - a; g->p.pos.y = y - b; drawpixel_clip(g); }		// Upper upper left
			if (sectors & 0x08) { g->p.pos.x = x - b; g->p.pos.y = y - a; drawpixel_clip(g); }		// Upper left  left
			if (sectors & 0x10) { g->p.pos.x = x - b; g->p.pos.y = y + a; drawpixel_clip(g); }		// Lower left  left
			if (sectors & 0x20) { g->p.pos.x = x - a; g->p.pos.y = y + b; drawpixel_clip(g); }		// Lower lower left
			if (sectors & 0x40) { g->p.pos.x = x + a; g->p.pos.y = y + b; drawpixel_clip(g); }		// Lower lower right
			if (sectors & 0x80) { g->p.pos.x = x + b; g->p.pos.y = y + a; drawpixel_clip(g); }		// Lower right right
			if (P < 0)
				P += 3 + 2*a++;
			else
				P += 5 + 2*(a++ - b--);
		} while(a < b);

		if (sectors & 0xC0) { g->p.pos.x = x + a; g->p.pos.y = y + b; drawpixel_clip(g); }			// Lower right
		if (sectors & 0x03) { g->p.pos.x = x + a; g->p.pos.y = y - b; drawpixel_clip(g); }			// Upper right
		if (sectors & 0x30) { g->p.pos.x = x - a; g->p.pos.y = y + b; drawpixel_clip(g); }			// Lower left
		if (sectors & 0x0C) { g->p.pos.x = x - a; g->p.pos.y = y - b; drawpixel_clip(g); }			// Upper left

		autoflush(g);
		MUTEX_EXIT(g);
	}
#endif

#if GDISP_NEED_ARCSECTORS
	void gdispGFillArcSectors(GDisplay *g, gCoord x, gCoord y, gCoord radius, uint8_t sectors, gColor color) {
		gCoord a, b, P;

		MUTEX_ENTER(g);

		// Calculate intermediates
		a = 1;              // x in many explanations
		b = radius;         // y in many explanations
		P = 4 - radius;
		g->p.color = color;

		// Away we go using Bresenham's circle algorithm
		// Optimized to prevent double drawing
		if (sectors & 0x06) { g->p.pos.x = x; g->p.pos.y = y - b; drawpixel_clip(g); }					// Upper upper
		if (sectors & 0x60) { g->p.pos.x = x; g->p.pos.y = y + b; drawpixel_clip(g); }					// Lower lower
		if (sectors & 0x81) {																	// Center right
			g->p.pos.y = y; g->p.pos.x = x; g->p.e.pos2.x = x + b;
			if (sectors & 0x18) g->p.pos.x -= b;													// Left right
			hline_clip(g);
		} else if (sectors & 0x18) {															// Left center
			g->p.pos.x = x - b; g->p.e.pos2.x = x; g->p.pos.y = y;
			hline_clip(g);
		}

		do {
			// Top half
			switch(sectors & 0x0F) {
			case 0x01:
				g->p.pos.y = y - a; g->p.pos.x = x + a; g->p.e.pos2.x = x + b; hline_clip(g);
				break;
			case 0x02:
				g->p.pos.y = y - b; g->p.pos.x = x; g->p.e.pos2.x = x + a; hline_clip(g);
				g->p.pos.y = y - a; g->p.pos.x = x; g->p.e.pos2.x = x + a; hline_clip(g);
				break;
			case 0x03:
				g->p.pos.y = y - b; g->p.pos.x = x; g->p.e.pos2.x = x + a; hline_clip(g);
				g->p.pos.y = y - a; g->p.pos.x = x; g->p.e.pos2.x = x + b; hline_clip(g);
				break;
			case 0x04:
				g->p.pos.y = y - b; g->p.pos.x = x - a; g->p.e.pos2.x = x; hline_clip(g);
				g->p.pos.y = y - a; g->p.pos.x = x - a; g->p.e.pos2.x = x; hline_clip(g);
				break;
			case 0x05:
				g->p.pos.y = y - b; g->p.pos.x = x - a; g->p.e.pos2.x = x; hline_clip(g);
				g->p.pos.y = y - a; g->p.pos.x = x - a; g->p.e.pos2.x = x; hline_clip(g);
				g->p.pos.y = y - a; g->p.pos.x = x + a; g->p.e.pos2.x = x + b; hline_clip(g);
				break;
			case 0x06:
				g->p.pos.y = y - b; g->p.pos.x = x - a; g->p.e.pos2.x = x + a; hline_clip(g);
				g->p.pos.y = y - a; g->p.pos.x = x - a; g->p.e.pos2.x = x + a; hline_clip(g);
				break;
			case 0x07:
				g->p.pos.y = y - b; g->p.pos.x = x - a; g->p.e.pos2.x = x + a; hline_clip(g);
				g->p.pos.y = y - a; g->p.pos.x = x - a; g->p.e.pos2.x = x + b; hline_clip(g);
				break;
			case 0x08:
				g->p.pos.y = y - a; g->p.pos.x = x - b; g->p.e.pos2.x = x - a; hline_clip(g);
				break;
			case 0x09:
				g->p.pos.y = y - a; g->p.pos.x = x - b; g->p.e.pos2.x = x - a; hline_clip(g);
				g->p.pos.y = y - a; g->p.pos.x = x + a; g->p.e.pos2.x = x + b; hline_clip(g);
				break;
			case 0x0A:
				g->p.pos.y = y - b; g->p.pos.x = x; g->p.e.pos2.x = x + a; hline_clip(g);
				g->p.pos.y = y - a; g->p.pos.x = x - b; g->p.e.pos2.x = x - a; hline_clip(g);
				g->p.pos.y = y - a; g->p.pos.x = x; g->p.e.pos2.x = x + a; hline_clip(g);
				break;
			case 0x0B:
				g->p.pos.y = y - b; g->p.pos.x = x; g->p.e.pos2.x = x + a; hline_clip(g);
				g->p.pos.y = y - a; g->p.pos.x = x - b; g->p.e.pos2.x = x - a; hline_clip(g);
				g->p.pos.y = y - a; g->p.pos.x = x; g->p.e.pos2.x = x + b; hline_clip(g);
				break;
			case 0x0C:
				g->p.pos.y = y - b; g->p.pos.x = x - a; g->p.e.pos2.x = x; hline_clip(g);
				g->p.pos.y = y - a; g->p.pos.x = x - b; g->p.e.pos2.x = x; hline_clip(g);
				break;
			case 0x0D:
				g->p.pos.y = y - b; g->p.pos.x = x - a; g->p.e.pos2.x = x; hline_clip(g);
				g->p.pos.y = y - a; g->p.pos.x = x - b; g->p.e.pos2.x = x; hline_clip(g);
				g->p.pos.y = y - a; g->p.pos.x = x + a; g->p.e.pos2.x = x + b; hline_clip(g);
				break;
			case 0x0E:
				g->p.pos.y = y - b; g->p.pos.x = x - a; g->p.e.pos2.x = x + a; hline_clip(g);
				g->p.pos.y = y - a; g->p.pos.x = x - b; g->p.e.pos2.x = x + a; hline_clip(g);
				break;
			case 0x0F:
				g->p.pos.y = y - b; g->p.pos.x = x - a; g->p.e.pos2.x = x + a; hline_clip(g);
				g->p.pos.y = y - a; g->p.pos.x = x - b; g->p.e.pos2.x = x + b; hline_clip(g);
				break;
			}

			// Bottom half
			switch((sectors & 0xF0)>>4) {
			case 0x01:
				g->p.pos.y = y + a; g->p.pos.x = x - b; g->p.e.pos2.x = x - a; hline_clip(g);
				break;
			case 0x02:
				g->p.pos.y = y + b; g->p.pos.x = x - a; g->p.e.pos2.x = x; hline_clip(g);
				g->p.pos.y = y + a; g->p.pos.x = x - a; g->p.e.pos2.x = x; hline_clip(g);
				break;
			case 0x03:
				g->p.pos.y = y + b; g->p.pos.x = x - a; g->p.e.pos2.x = x; hline_clip(g);
				g->p.pos.y = y + a; g->p.pos.x = x - b; g->p.e.pos2.x = x; hline_clip(g);
				break;
			case 0x04:
				g->p.pos.y = y + b; g->p.pos.x = x; g->p.e.pos2.x = x + a; hline_clip(g);
				g->p.pos.y = y + a; g->p.pos.x = x; g->p.e.pos2.x = x + a; hline_clip(g);
				break;
			case 0x05:
				g->p.pos.y = y + b; g->p.pos.x = x; g->p.e.pos2.x = x + a; hline_clip(g);
				g->p.pos.y = y + a; g->p.pos.x = x - b; g->p.e.pos2.x = x - a; hline_clip(g);
				g->p.pos.y = y + a; g->p.pos.x = x; g->p.e.pos2.x = x + a; hline_clip(g);
				break;
			case 0x06:
				g->p.pos.y = y + b; g->p.pos.x = x - a; g->p.e.pos2.x = x + a; hline_clip(g);
				g->p.pos.y = y + a; g->p.pos.x = x - a; g->p.e.pos2.x = x + a; hline_clip(g);
				break;
			case 0x07:
				g->p.pos.y = y + b; g->p.pos.x = x - a; g->p.e.pos2.x = x + a; hline_clip(g);
				g->p.pos.y = y + a; g->p.pos.x = x - b; g->p.e.pos2.x = x + a; hline_clip(g);
				break;
			case 0x08:
				g->p.pos.y = y + a; g->p.pos.x = x + a; g->p.e.pos2.x = x + b; hline_clip(g);
				break;
			case 0x09:
				g->p.pos.y = y + a; g->p.pos.x = x - b; g->p.e.pos2.x = x - a; hline_clip(g);
				g->p.pos.y = y + a; g->p.pos.x = x + a; g->p.e.pos2.x = x + b; hline_clip(g);
				break;
			case 0x0A:
				g->p.pos.y = y + b; g->p.pos.x = x - a; g->p.e.pos2.x = x; hline_clip(g);
				g->p.pos.y = y + a; g->p.pos.x = x - a; g->p.e.pos2.x = x; hline_clip(g);
				g->p.pos.y = y + a; g->p.pos.x = x + a; g->p.e.pos2.x = x + b; hline_clip(g);
				break;
			case 0x0B:
				g->p.pos.y = y + b; g->p.pos.x = x - a; g->p.e.pos2.x = x; hline_clip(g);
				g->p.pos.y = y + a; g->p.pos.x = x - b; g->p.e.pos2.x = x; hline_clip(g);
				g->p.pos.y = y + a; g->p.pos.x = x + a; g->p.e.pos2.x = x + b; hline_clip(g);
				break;
			case 0x0C:
				g->p.pos.y = y + b; g->p.pos.x = x; g->p.e.pos2.x = x + a; hline_clip(g);
				g->p.pos.y = y + a; g->p.pos.x = x; g->p.e.pos2.x = x + b; hline_clip(g);
				break;
			case 0x0D:
				g->p.pos.y = y + b; g->p.pos.x = x; g->p.e.pos2.x = x + a; hline_clip(g);
				g->p.pos.y = y + a; g->p.pos.x = x - b; g->p.e.pos2.x = x - a; hline_clip(g);
				g->p.pos.y = y + a; g->p.pos.x = x; g->p.e.pos2.x = x + b; hline_clip(g);
				break;
			case 0x0E:
				g->p.pos.y = y + b; g->p.pos.x = x - a; g->p.e.pos2.x = x + a; hline_clip(g);
				g->p.pos.y = y + a; g->p.pos.x = x - a; g->p.e.pos2.x = x + b; hline_clip(g);
				break;
			case 0x0F:
				g->p.pos.y = y + b; g->p.pos.x = x - a; g->p.e.pos2.x = x + a; hline_clip(g);
				g->p.pos.y = y + a; g->p.pos.x = x - b; g->p.e.pos2.x = x + b; hline_clip(g);
				break;
			}

			if (P < 0)
				P += 3 + 2*a++;
			else
				P += 5 + 2*(a++ - b--);
		} while(a < b);

		// Top half
		if (sectors & 0x02)			{ g->p.pos.y = y - a; g->p.pos.x = x; g->p.e.pos2.x = x + a; hline_clip(g); }
		else if (sectors & 0x01)	{ g->p.pos.y = y - a; g->p.pos.x = x + a; drawpixel_clip(g); }
		if (sectors & 0x04)			{ g->p.pos.y = y - a; g->p.pos.x = x - a; g->p.e.pos2.x = x; hline_clip(g); }
		else if (sectors & 0x08)	{ g->p.pos.y = y - a; g->p.pos.x = x - a; drawpixel_clip(g); }

		// Bottom half
		if (sectors & 0x40)			{ g->p.pos.y = y + a; g->p.pos.x = x; g->p.e.pos2.x = x + a; hline_clip(g); }
		else if (sectors & 0x80)	{ g->p.pos.y = y + a; g->p.pos.x = x + a; drawpixel_clip(g); }
		if (sectors & 0x20)			{ g->p.pos.y = y + a; g->p.pos.x = x - a; g->p.e.pos2.x = x; hline_clip(g); }
		else if (sectors & 0x10)	{ g->p.pos.y = y + a; g->p.pos.x = x - a; drawpixel_clip(g); }

		autoflush(g);
		MUTEX_EXIT(g);
	}
#endif

#if GDISP_NEED_ARC
	#if (!GMISC_NEED_FIXEDTRIG && !GMISC_NEED_FASTTRIG) || !GFX_USE_GMISC
		#include <math.h>
	#endif

	void gdispGDrawArc(GDisplay *g, gCoord x, gCoord y, gCoord radius, gCoord start, gCoord end, gColor color) {
		gCoord a, b, P, sedge, eedge;
		uint8_t	full, sbit, ebit, tbit;

		// Normalize the angles
		if (start < 0)
			start -= (start/360-1)*360;
		else if (start >= 360)
			start %= 360;
		if (end < 0)
			end -= (end/360-1)*360;
		else if (end >= 360)
			end %= 360;

		sbit = 1<<(start/45);
		ebit = 1<<(end/45);
		full = 0;
		if (start == end) {
			full = 0xFF;
		} else if (end < start) {
			for(tbit=sbit<<1; tbit; tbit<<=1) full |= tbit;
			for(tbit=ebit>>1; tbit; tbit>>=1) full |= tbit;
		} else if (sbit < 0x80) {
			for(tbit=sbit<<1; tbit < ebit; tbit<<=1) full |= tbit;
		}
		tbit = start%45 == 0 ? sbit : 0;

		MUTEX_ENTER(g);
		g->p.color = color;

		if (full) {
			// Draw full sectors
			// Optimized to prevent double drawing
			a = 1;
			b = radius;
			P = 4 - radius;
			if (full & 0x60) { g->p.pos.y = y+b; g->p.pos.x = x; drawpixel_clip(g); }
			if (full & 0x06) { g->p.pos.y = y-b; g->p.pos.x = x; drawpixel_clip(g); }
			if (full & 0x81) { g->p.pos.y = y; g->p.pos.x = x+b; drawpixel_clip(g); }
			if (full & 0x18) { g->p.pos.y = y; g->p.pos.x = x-b; drawpixel_clip(g); }
			do {
				if (full & 0x01) { g->p.pos.x = x+b; g->p.pos.y = y-a; drawpixel_clip(g); }
				if (full & 0x02) { g->p.pos.x = x+a; g->p.pos.y = y-b; drawpixel_clip(g); }
				if (full & 0x04) { g->p.pos.x = x-a; g->p.pos.y = y-b; drawpixel_clip(g); }
				if (full & 0x08) { g->p.pos.x = x-b; g->p.pos.y = y-a; drawpixel_clip(g); }
				if (full & 0x10) { g->p.pos.x = x-b; g->p.pos.y = y+a; drawpixel_clip(g); }
				if (full & 0x20) { g->p.pos.x = x-a; g->p.pos.y = y+b; drawpixel_clip(g); }
				if (full & 0x40) { g->p.pos.x = x+a; g->p.pos.y = y+b; drawpixel_clip(g); }
				if (full & 0x80) { g->p.pos.x = x+b; g->p.pos.y = y+a; drawpixel_clip(g); }
				if (P < 0)
					P += 3 + 2*a++;
				else
					P += 5 + 2*(a++ - b--);
			} while(a < b);
			if (full & 0xC0) { g->p.pos.x = x+a; g->p.pos.y = y+b; drawpixel_clip(g); }
			if (full & 0x0C) { g->p.pos.x = x-a; g->p.pos.y = y-b; drawpixel_clip(g); }
			if (full & 0x03) { g->p.pos.x = x+a; g->p.pos.y = y-b; drawpixel_clip(g); }
			if (full & 0x30) { g->p.pos.x = x-a; g->p.pos.y = y+b; drawpixel_clip(g); }
			if (full == 0xFF) {
				autoflush(g);
				MUTEX_EXIT(g);
				return;
			}
		}

		#if GFX_USE_GMISC && GMISC_NEED_FIXEDTRIG
			sedge = NONFIXED(radius * ((sbit & 0x99) ? ffsin(start) : ffcos(start)) + FIXED0_5);
			eedge = NONFIXED(radius * ((ebit & 0x99) ? ffsin(end) : ffcos(end)) + FIXED0_5);
		#elif GFX_USE_GMISC && GMISC_NEED_FASTTRIG
			sedge = floor(radius * ((sbit & 0x99) ? fsin(start) : fcos(start)) + 0.5);
			eedge = floor(radius * ((ebit & 0x99) ? fsin(end) : fcos(end)) + 0.5);
		#else
			sedge = floor(radius * ((sbit & 0x99) ? sin(start*GFX_PI/180) : cos(start*GFX_PI/180)) + 0.5);
			eedge = floor(radius * ((ebit & 0x99) ? sin(end*GFX_PI/180) : cos(end*GFX_PI/180)) + 0.5);
		#endif
		if (sbit & 0xB4) sedge = -sedge;
		if (ebit & 0xB4) eedge = -eedge;

		if (sbit != ebit) {
			// Draw start and end sectors
			// Optimized to prevent double drawing
			a = 1;
			b = radius;
			P = 4 - radius;
			if ((sbit & 0x20) || (tbit & 0x40) || (ebit & 0x40)) { g->p.pos.x = x; g->p.pos.y = y+b; drawpixel_clip(g); }
			if ((sbit & 0x02) || (tbit & 0x04) || (ebit & 0x04)) { g->p.pos.x = x; g->p.pos.y = y-b; drawpixel_clip(g); }
			if ((sbit & 0x80) || (tbit & 0x01) || (ebit & 0x01)) { g->p.pos.x = x+b; g->p.pos.y = y; drawpixel_clip(g); }
			if ((sbit & 0x08) || (tbit & 0x10) || (ebit & 0x10)) { g->p.pos.x = x-b; g->p.pos.y = y; drawpixel_clip(g); }
			do {
				if (((sbit & 0x01) && a >= sedge) || ((ebit & 0x01) && a <= eedge)) { g->p.pos.x = x+b; g->p.pos.y = y-a; drawpixel_clip(g); }
				if (((sbit & 0x02) && a <= sedge) || ((ebit & 0x02) && a >= eedge)) { g->p.pos.x = x+a; g->p.pos.y = y-b; drawpixel_clip(g); }
				if (((sbit & 0x04) && a >= sedge) || ((ebit & 0x04) && a <= eedge)) { g->p.pos.x = x-a; g->p.pos.y = y-b; drawpixel_clip(g); }
				if (((sbit & 0x08) && a <= sedge) || ((ebit & 0x08) && a >= eedge)) { g->p.pos.x = x-b; g->p.pos.y = y-a; drawpixel_clip(g); }
				if (((sbit & 0x10) && a >= sedge) || ((ebit & 0x10) && a <= eedge)) { g->p.pos.x = x-b; g->p.pos.y = y+a; drawpixel_clip(g); }
				if (((sbit & 0x20) && a <= sedge) || ((ebit & 0x20) && a >= eedge)) { g->p.pos.x = x-a; g->p.pos.y = y+b; drawpixel_clip(g); }
				if (((sbit & 0x40) && a >= sedge) || ((ebit & 0x40) && a <= eedge)) { g->p.pos.x = x+a; g->p.pos.y = y+b; drawpixel_clip(g); }
				if (((sbit & 0x80) && a <= sedge) || ((ebit & 0x80) && a >= eedge)) { g->p.pos.x = x+b; g->p.pos.y = y+a; drawpixel_clip(g); }
				if (P < 0)
					P += 3 + 2*a++;
				else
					P += 5 + 2*(a++ - b--);
			} while(a < b);
			if (((sbit & 0x40) && a >= sedge) || ((ebit & 0x40) && a <= eedge) || ((sbit & 0x80) && a <= sedge) || ((ebit & 0x80) && a >= eedge))
				{ g->p.pos.x = x+a; g->p.pos.y = y+b; drawpixel_clip(g); }
			if (((sbit & 0x04) && a >= sedge) || ((ebit & 0x04) && a <= eedge) || ((sbit & 0x08) && a <= sedge) || ((ebit & 0x08) && a >= eedge))
				{ g->p.pos.x = x-a; g->p.pos.y = y-b; drawpixel_clip(g); }
			if (((sbit & 0x01) && a >= sedge) || ((ebit & 0x01) && a <= eedge) || ((sbit & 0x02) && a <= sedge) || ((ebit & 0x02) && a >= eedge))
				{ g->p.pos.x = x+a; g->p.pos.y = y-b; drawpixel_clip(g); }
			if (((sbit & 0x10) && a >= sedge) || ((ebit & 0x10) && a <= eedge) || ((sbit & 0x20) && a <= sedge) || ((ebit & 0x20) && a >= eedge))
				{ g->p.pos.x = x-a; g->p.pos.y = y+b; drawpixel_clip(g); }
		} else if (end < start) {
			// Draw start/end sector where it is a non-internal angle
			// Optimized to prevent double drawing
			a = 1;
			b = radius;
			P = 4 - radius;
			if ((sbit & 0x60) || (tbit & 0xC0)) { g->p.pos.x = x; g->p.pos.y = y+b; drawpixel_clip(g); }
			if ((sbit & 0x06) || (tbit & 0x0C)) { g->p.pos.x = x; g->p.pos.y = y-b; drawpixel_clip(g); }
			if ((sbit & 0x81) || (tbit & 0x03)) { g->p.pos.x = x+b; g->p.pos.y = y; drawpixel_clip(g); }
			if ((sbit & 0x18) || (tbit & 0x30)) { g->p.pos.x = x-b; g->p.pos.y = y; drawpixel_clip(g); }
			do {
				if ((sbit & 0x01) && (a >= sedge || a <= eedge)) { g->p.pos.x = x+b; g->p.pos.y = y-a; drawpixel_clip(g); }
				if ((sbit & 0x02) && (a <= sedge || a >= eedge)) { g->p.pos.x = x+a; g->p.pos.y = y-b; drawpixel_clip(g); }
				if ((sbit & 0x04) && (a >= sedge || a <= eedge)) { g->p.pos.x = x-a; g->p.pos.y = y-b; drawpixel_clip(g); }
				if ((sbit & 0x08) && (a <= sedge || a >= eedge)) { g->p.pos.x = x-b; g->p.pos.y = y-a; drawpixel_clip(g); }
				if ((sbit & 0x10) && (a >= sedge || a <= eedge)) { g->p.pos.x = x-b; g->p.pos.y = y+a; drawpixel_clip(g); }
				if ((sbit & 0x20) && (a <= sedge || a >= eedge)) { g->p.pos.x = x-a; g->p.pos.y = y+b; drawpixel_clip(g); }
				if ((sbit & 0x40) && (a >= sedge || a <= eedge)) { g->p.pos.x = x+a; g->p.pos.y = y+b; drawpixel_clip(g); }
				if ((sbit & 0x80) && (a <= sedge || a >= eedge)) { g->p.pos.x = x+b; g->p.pos.y = y+a; drawpixel_clip(g); }
				if (P < 0)
					P += 3 + 2*a++;
				else
					P += 5 + 2*(a++ - b--);
			} while(a < b);
			if (((sbit & 0x04) && (a >= sedge || a <= eedge)) || ((sbit & 0x08) && (a <= sedge || a >= eedge)))
				{ g->p.pos.x = x-a; g->p.pos.y = y-b; drawpixel_clip(g); }
			if (((sbit & 0x40) && (a >= sedge || a <= eedge)) || ((sbit & 0x80) && (a <= sedge || a >= eedge)))
				{ g->p.pos.x = x+a; g->p.pos.y = y+b; drawpixel_clip(g); }
			if (((sbit & 0x01) && (a >= sedge || a <= eedge)) || ((sbit & 0x02) && (a <= sedge || a >= eedge)))
				{ g->p.pos.x = x+a; g->p.pos.y = y-b; drawpixel_clip(g); }
			if (((sbit & 0x10) && (a >= sedge || a <= eedge)) || ((sbit & 0x20) && (a <= sedge || a >= eedge)))
				{ g->p.pos.x = x-a; g->p.pos.y = y+b; drawpixel_clip(g); }
		} else {
			// Draw start/end sector where it is a internal angle
			// Optimized to prevent double drawing
			a = 1;
			b = radius;
			P = 4 - radius;
			if (((sbit & 0x20) && !eedge) || ((sbit & 0x40) && !sedge)) { g->p.pos.x = x; g->p.pos.y = y+b; drawpixel_clip(g); }
			if (((sbit & 0x02) && !eedge) || ((sbit & 0x04) && !sedge)) { g->p.pos.x = x; g->p.pos.y = y-b; drawpixel_clip(g); }
			if (((sbit & 0x80) && !eedge) || ((sbit & 0x01) && !sedge)) { g->p.pos.x = x+b; g->p.pos.y = y; drawpixel_clip(g); }
			if (((sbit & 0x08) && !eedge) || ((sbit & 0x10) && !sedge)) { g->p.pos.x = x-b; g->p.pos.y = y; drawpixel_clip(g); }
			do {
				if (((sbit & 0x01) && a >= sedge && a <= eedge)) { g->p.pos.x = x+b; g->p.pos.y = y-a; drawpixel_clip(g); }
				if (((sbit & 0x02) && a <= sedge && a >= eedge)) { g->p.pos.x = x+a; g->p.pos.y = y-b; drawpixel_clip(g); }
				if (((sbit & 0x04) && a >= sedge && a <= eedge)) { g->p.pos.x = x-a; g->p.pos.y = y-b; drawpixel_clip(g); }
				if (((sbit & 0x08) && a <= sedge && a >= eedge)) { g->p.pos.x = x-b; g->p.pos.y = y-a; drawpixel_clip(g); }
				if (((sbit & 0x10) && a >= sedge && a <= eedge)) { g->p.pos.x = x-b; g->p.pos.y = y+a; drawpixel_clip(g); }
				if (((sbit & 0x20) && a <= sedge && a >= eedge)) { g->p.pos.x = x-a; g->p.pos.y = y+b; drawpixel_clip(g); }
				if (((sbit & 0x40) && a >= sedge && a <= eedge)) { g->p.pos.x = x+a; g->p.pos.y = y+b; drawpixel_clip(g); }
				if (((sbit & 0x80) && a <= sedge && a >= eedge)) { g->p.pos.x = x+b; g->p.pos.y = y+a; drawpixel_clip(g); }
				if (P < 0)
					P += 3 + 2*a++;
				else
					P += 5 + 2*(a++ - b--);
			} while(a < b);
			if (((sbit & 0x04) && a >= sedge && a <= eedge) || ((sbit & 0x08) && a <= sedge && a >= eedge))
				{ g->p.pos.x = x-a; g->p.pos.y = y-b; drawpixel_clip(g); }
			if (((sbit & 0x40) && a >= sedge && a <= eedge) || ((sbit & 0x80) && a <= sedge && a >= eedge))
				{ g->p.pos.x = x+a; g->p.pos.y = y+b; drawpixel_clip(g); }
			if (((sbit & 0x01) && a >= sedge && a <= eedge) || ((sbit & 0x02) && a <= sedge && a >= eedge))
				{ g->p.pos.x = x+a; g->p.pos.y = y-b; drawpixel_clip(g); }
			if (((sbit & 0x10) && a >= sedge && a <= eedge) || ((sbit & 0x20) && a <= sedge && a >= eedge))
				{ g->p.pos.x = x-a; g->p.pos.y = y+b; drawpixel_clip(g); }
		}

		autoflush(g);
		MUTEX_EXIT(g);
	}
#endif

#if GDISP_NEED_ARC
	#if (!GMISC_NEED_FIXEDTRIG && !GMISC_NEED_FASTTRIG) || !GFX_USE_GMISC
		#include <math.h>
	#endif

	void gdispGDrawThickArc(GDisplay *g, gCoord xc, gCoord yc, gCoord radiusStart, gCoord radiusEnd, gCoord start, gCoord end, gColor color) {
		gCoord x, y, d, r;
		gCoord startTan, endTan, curangle;
		gCoord precision = 512;

		// Normalize the angles
		if (start < 0)
			start -= (start/360-1)*360;
		else if (start >= 360)
			start %= 360;
		if (end < 0)
			end -= (end/360-1)*360;
		else if (end >= 360)
			end %= 360;

		#if GFX_USE_GMISC && GMISC_NEED_FIXEDTRIG
			if((start / 45) % 2 == 0){
				startTan = ffsin(start % 45) * precision / ffcos(start % 45) + start / 45 * precision;}
			else{
				startTan = ffsin(start % 45 - 45) * precision / ffcos(start % 45 - 45) + start / 45 * precision + precision;}

			if((end / 45) % 2 == 0){
				endTan = ffsin(end % 45) * precision / ffcos(end % 45) + end / 45 * precision;}
			else{
				endTan = ffsin(end % 45 - 45) * precision / ffcos(end % 45 - 45) + end / 45 * precision + precision;}
		#elif GFX_USE_GMISC && GMISC_NEED_FASTTRIG
			if((start / 45) % 2 == 0){
				startTan = fsin(start % 45) * precision / fcos(start % 45) + start / 45 * precision;}
			else{
				startTan = fsin(start % 45 - 45) * precision / fcos(start % 45 - 45) + start / 45 * precision + precision;}

			if((end / 45) % 2 == 0){
				endTan = fsin(end % 45) * precision / fcos(end % 45) + end / 45 * precision;}
			else{
				endTan = fsin(end % 45 - 45) * precision / fcos(end % 45 - 45) + end / 45 * precision + precision;}
		#else
			if((start / 45) % 2 == 0){
				startTan = (tan((start % 45)*GFX_PI/180) + start / 45)* precision;}
			else{
				startTan = (1+tan((start % 45 - 45)*GFX_PI/180) + start / 45)* precision;}

			if((end / 45) % 2 == 0){
				endTan = (tan((end % 45) *GFX_PI/180) + end / 45) * precision;}
			else{
				endTan = (1+tan((end % 45 - 45) *GFX_PI/180) + end / 45) * precision;}
		#endif

		MUTEX_ENTER(g);
		g->p.color = color;

		//Draw concentric circles using Andres algorithm
		for(r = radiusStart; r <= radiusEnd; r++)
		{
			x = 0;
			y = r;
			d = r - 1;

			while (y >= x){
				//approximate tan
				curangle = x*precision/y;

				if(end > start){
					g->p.color = color;
					//Draw points by symmetry
					if(curangle > startTan && curangle < endTan){g->p.pos.y = yc - x; g->p.pos.x = xc + y; drawpixel_clip(g);}
					if(curangle + 2*precision > startTan && curangle + 2*precision < endTan){g->p.pos.y = yc - y; g->p.pos.x = xc - x; drawpixel_clip(g);}
					if(curangle + 4*precision > startTan && curangle + 4*precision < endTan){g->p.pos.y = yc + x; g->p.pos.x = xc - y; drawpixel_clip(g);}
					if(curangle + 6*precision > startTan && curangle + 6*precision < endTan){g->p.pos.y = yc + y; g->p.pos.x = xc + x; drawpixel_clip(g);}

					curangle = precision - curangle;

					if(curangle + precision > startTan && curangle + precision < endTan){g->p.pos.y = yc - y; g->p.pos.x = xc + x; drawpixel_clip(g);}
					if(curangle + 3*precision > startTan && curangle + 3*precision < endTan){g->p.pos.y = yc - x; g->p.pos.x = xc - y; drawpixel_clip(g);}
					if(curangle + 5*precision > startTan && curangle + 5*precision < endTan){g->p.pos.y = yc + y; g->p.pos.x = xc - x; drawpixel_clip(g);}
					if(curangle + 7*precision > startTan && curangle + 7*precision < endTan){g->p.pos.y = yc + x; g->p.pos.x = xc + y; drawpixel_clip(g);}
						
				}
				else{
					//Draw points by symmetry
					if(curangle > startTan || curangle < endTan){g->p.pos.y = yc - x; g->p.pos.x = xc + y; drawpixel_clip(g);}
					if(curangle + 2*precision > startTan || curangle + 2*precision < endTan){g->p.pos.y = yc - y; g->p.pos.x = xc - x; drawpixel_clip(g);}
					if(curangle + 4*precision > startTan || curangle + 4*precision < endTan){g->p.pos.y = yc + x; g->p.pos.x = xc - y; drawpixel_clip(g);}
					if(curangle + 6*precision > startTan || curangle + 6*precision < endTan){g->p.pos.y = yc + y; g->p.pos.x = xc + x; drawpixel_clip(g);}

					curangle = precision - curangle;

					if(curangle + precision > startTan || curangle + precision < endTan){g->p.pos.y = yc - y; g->p.pos.x = xc + x; drawpixel_clip(g);}
					if(curangle + 3*precision > startTan || curangle + 3*precision < endTan){g->p.pos.y = yc - x; g->p.pos.x = xc - y; drawpixel_clip(g);}
					if(curangle + 5*precision > startTan || curangle + 5*precision < endTan){g->p.pos.y = yc + y; g->p.pos.x = xc - x; drawpixel_clip(g);}
					if(curangle + 7*precision > startTan || curangle + 7*precision < endTan){g->p.pos.y = yc + x; g->p.pos.x = xc + y; drawpixel_clip(g);}					
				}

				//Compute next point
				if (d >= 2 * x){
					d -= 2 * x + 1;
					x++;
				}
				else if (d < 2 * (r - y)){
					d += 2 * y - 1;
					y--;
				}
				else{
					d += 2 * (y - x - 1);
					y--;
					x++;
				}
			}
		}

		autoflush(g);
		MUTEX_EXIT(g);
	}
#endif

#if GDISP_NEED_ARC
	void gdispGFillArc(GDisplay *g, gCoord x, gCoord y, gCoord radius, gCoord start, gCoord end, gColor color) {
		gCoord a, b, P;
		gCoord	sy, ey;
		fixed	sxa, sxb, sxd, exa, exb, exd;
		uint8_t	qtr;

		MUTEX_ENTER(g);

		// We add a half pixel so that we are drawing from the centre of the pixel
		//	instead of the left edge of the pixel. This also fixes the implied floor()
		//	when converting back to a gCoord
		sxa = exa = FIXED(x) + FIXED0_5;

		// Do the trig to get the formulas for the start and end lines.
		#if GFX_USE_GMISC && GMISC_NEED_FIXEDTRIG
			sxb = radius*ffcos(start);	sy = NONFIXED(FIXED0_5 - radius*ffsin(start));
			exb = radius*ffcos(end);	ey = NONFIXED(FIXED0_5 - radius*ffsin(end));
		#elif GFX_USE_GMISC && GMISC_NEED_FASTTRIG
			sxb = FP2FIXED(radius*fcos(start));	sy = floor(0.5-radius*fsin(start));
			exb = FP2FIXED(radius*fcos(end));	ey = floor(0.5-radius*fsin(end));
		#else
			sxb = FP2FIXED(radius*cos(start*GFX_PI/180));	sy = floor(0.5-radius*sin(start*GFX_PI/180));
			exb = FP2FIXED(radius*cos(end*GFX_PI/180));		ey = floor(0.5-radius*sin(end*GFX_PI/180));
		#endif
		sxd = sy ? sxb/sy : sxb;
		exd = ey ? exb/ey : exb;

		// Calculate which quarters and which direction we are traveling
		qtr = 0;
		if (sxb > 0)	qtr |= 0x01;		// S1=0001(1), S2=0000(0), S3=0010(2), S4=0011(3)
		if (sy > 0) 	qtr |= 0x02;
		if (exb > 0)	qtr |= 0x04;		// E1=0100(4), E2=0000(0), E3=1000(8), E4=1100(12)
		if (ey > 0) 	qtr |= 0x08;
		if (sy > ey || (sy == ey && sxb > 0))	qtr |= 0x10;		// order of start and end lines

		// Calculate intermediates
		a = 1;
		b = radius;
		P = 4 - radius;
		g->p.color = color;
		sxb += sxa;
		exb += exa;

		// Away we go using Bresenham's circle algorithm
		// This is optimized to prevent overdrawing by drawing a line only when a variable is about to change value

		switch(qtr) {
		case 0:		// S2E2 sy <= ey
		case 1:		// S1E2 sy <= ey
			if (ey && sy) {
				g->p.pos.x = x; g->p.e.pos2.x = x;								// E2S
				sxa -= sxd; exa -= exd;
			} else if (sy) {
				g->p.pos.x = x-b; g->p.e.pos2.x = x;								// C2S
				sxa -= sxd;
			} else if (ey) {
				g->p.pos.x = x; g->p.e.pos2.x = x+b;								// E2C
				exa -= exd;
			} else {
				g->p.pos.x = x-b; g->p.e.pos2.x = x+b;							// C2C
			}
			g->p.pos.y = y;
			hline_clip(g);
			do {
				if (-a >= ey) {
					g->p.pos.y = y-a; g->p.pos.x = NONFIXED(exa); g->p.e.pos2.x = NONFIXED(sxa); hline_clip(g);		// E2S
					sxa -= sxd; exa -= exd;
				} else if (-a >= sy) {
					g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = NONFIXED(sxa); hline_clip(g);					// C2S
					sxa -= sxd;
				} else if (qtr & 1) {
					g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);							// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (-b >= ey) {
						g->p.pos.y = y-b; g->p.pos.x = NONFIXED(exb); g->p.e.pos2.x = NONFIXED(sxb); hline_clip(g);	// E2S
						sxb += sxd; exb += exd;
					} else if (-b >= sy) {
						g->p.pos.y = y-b; g->p.pos.x = x-a; g->p.e.pos2.x = NONFIXED(sxb); hline_clip(g);				// C2S
						sxb += sxd;
					} else if (qtr & 1) {
						g->p.pos.y = y-b; g->p.pos.x = x-a; g->p.e.pos2.x = x+a; hline_clip(g);						// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (-a >= ey) {
				g->p.pos.y = y-a; g->p.pos.x = NONFIXED(exa); g->p.e.pos2.x = NONFIXED(sxa); hline_clip(g);			// E2S
			} else if (-a >= sy) {
				g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = NONFIXED(sxa); hline_clip(g);						// C2S
			} else if (qtr & 1) {
				g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);								// C2C
			}
			break;

		case 2:		// S3E2 sy <= ey
		case 3:		// S4E2 sy <= ey
		case 6:		// S3E1 sy <= ey
		case 7:		// S4E1 sy <= ey
		case 18:	// S3E2 sy > ey
		case 19:	// S4E2 sy > ey
		case 22:	// S3E1 sy > ey
		case 23:	// S4E1 sy > ey
			g->p.pos.y = y; g->p.pos.x = x; g->p.e.pos2.x = x+b; hline_clip(g);								// SE2C
			sxa += sxd; exa -= exd;
			do {
				if (-a >= ey) {
					g->p.pos.y = y-a; g->p.pos.x = NONFIXED(exa); g->p.e.pos2.x = x+b; hline_clip(g);			// E2C
					exa -= exd;
				} else if (!(qtr & 4)) {
					g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);					// C2C
				}
				if (a <= sy) {
					g->p.pos.y = y+a; g->p.pos.x = NONFIXED(sxa); g->p.e.pos2.x = x+b; hline_clip(g);			// S2C
					sxa += sxd;
				} else if (!(qtr & 1)) {
					g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);					// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (-b >= ey) {
						g->p.pos.y = y-b; g->p.pos.x = NONFIXED(exb); g->p.e.pos2.x = x+a; hline_clip(g);		// E2C
						exb += exd;
					} else if (!(qtr & 4)) {
						g->p.pos.y = y-b; g->p.pos.x = x-a; g->p.e.pos2.x = x+a; hline_clip(g);				// C2C
					}
					if (b <= sy) {
						g->p.pos.y = y+b; g->p.pos.x = NONFIXED(sxb); g->p.e.pos2.x = x+a; hline_clip(g);		// S2C
						sxb -= sxd;
					} else if (!(qtr & 1)) {
						g->p.pos.y = y+b; g->p.pos.x = x-a; g->p.e.pos2.x = x+a; hline_clip(g); 				// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (-a >= ey) {
				g->p.pos.y = y-a; g->p.pos.x = NONFIXED(exa); g->p.e.pos2.x = x+b; hline_clip(g);				// E2C
			} else if (!(qtr & 4)) {
				g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);						// C2C
			}
			if (a <= sy) {
				g->p.pos.y = y+a; g->p.pos.x = NONFIXED(sxa); g->p.e.pos2.x = x+a; hline_clip(g);				// S2C
			} else if (!(qtr & 1)) {
				g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = x+a; hline_clip(g);						// C2C
			}
			break;

		case 4:		// S2E1 sy <= ey
		case 5:		// S1E1 sy <= ey
			g->p.pos.y = y; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);								// C2C
			do {
				if (-a >= ey) {
					g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = NONFIXED(sxa); hline_clip(g);			// C2S
					g->p.pos.y = y-a; g->p.pos.x = NONFIXED(exa); g->p.e.pos2.x = x+b; hline_clip(g);			// E2C
					sxa -= sxd; exa -= exd;
				} else if (-a >= sy) {
					g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = NONFIXED(sxa); hline_clip(g);			// C2S
					sxa -= sxd;
				} else if (qtr & 1) {
					g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);					// C2C
				}
				g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);						// C2C
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (-b >= ey) {
						g->p.pos.y = y-b; g->p.pos.x = x-a; g->p.e.pos2.x = NONFIXED(sxb); hline_clip(g);		// C2S
						g->p.pos.y = y-b; g->p.pos.x = NONFIXED(exb); g->p.e.pos2.x = x+a; hline_clip(g);		// E2C
						sxb += sxd; exb += exd;
					} else if (-b >= sy) {
						g->p.pos.y = y-b; g->p.pos.x = x-a; g->p.e.pos2.x = NONFIXED(sxb); hline_clip(g);		// C2S
						sxb += sxd;
					} else if (qtr & 1) {
						g->p.pos.y = y-b; g->p.pos.x = x-a; g->p.e.pos2.x = x+a; hline_clip(g);				// C2C
					}
					g->p.pos.y = y+b; g->p.pos.x = x-a; g->p.e.pos2.x = x+a; hline_clip(g);					// C2C
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (-a >= ey) {
				g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = NONFIXED(sxa); hline_clip(g);				// C2S
				g->p.pos.y = y-a; g->p.pos.x = NONFIXED(exa); g->p.e.pos2.x = x+b; hline_clip(g);				// E2C
			} else if (-a >= sy) {
				g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = NONFIXED(sxa); hline_clip(g);				// C2S
			} else if (qtr & 1) {
				g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);						// C2C
			}
			g->p.pos.y = y+b; g->p.pos.x = x-a; g->p.e.pos2.x = x+a; hline_clip(g);							// C2C
			break;

		case 8:		// S2E3 sy <= ey
		case 9:		// S1E3 sy <= ey
		case 12:	// S2E4 sy <= ey
		case 13:	// S1E4 sy <= ey
		case 24:	// S2E3 sy > ey
		case 25:	// S1E3 sy > ey
		case 28:	// S2E3 sy > ey
		case 29:	// S1E3 sy > ey
			g->p.pos.y = y; g->p.pos.x = x-b; g->p.e.pos2.x = x; hline_clip(g);								// C2SE
			sxa -= sxd; exa += exd;
			do {
				if (-a >= sy) {
					g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = NONFIXED(sxa); hline_clip(g);			// C2S
					sxa -= sxd;
				} else if (qtr & 1) {
					g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);					// C2C
				}
				if (a <= ey) {
					g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = NONFIXED(exa); hline_clip(g);			// C2E
					exa += exd;
				} else if (qtr & 4) {
					g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);					// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (-b >= sy) {
						g->p.pos.y = y-b; g->p.pos.x = x-a; g->p.e.pos2.x = NONFIXED(sxb); hline_clip(g);		// C2S
						sxb += sxd;
					} else if (qtr & 1) {
						g->p.pos.y = y-b; g->p.pos.x = x-a; g->p.e.pos2.x = x+a; hline_clip(g);				// C2C
					}
					if (b <= ey) {
						g->p.pos.y = y+b; g->p.pos.x = x-a; g->p.e.pos2.x = NONFIXED(exb); hline_clip(g);		// C2E
						exb -= exd;
					} else if (qtr & 4) {
						g->p.pos.y = y+b; g->p.pos.x = x-a; g->p.e.pos2.x = x+a; hline_clip(g); 				// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (-a >= sy) {
				g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = NONFIXED(sxa); hline_clip(g);				// C2S
			} else if (qtr & 1) {
				g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);						// C2C
			}
			if (a <= ey) {
				g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = NONFIXED(exa); hline_clip(g);				// C2E
			} else if (qtr & 4) {
				g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = x+a; hline_clip(g);						// C2C
			}
			break;

		case 10:	// S3E3 sy <= ey
		case 14:	// S3E4 sy <= ey
			g->p.pos.y = y; g->p.pos.x = x; drawpixel_clip(g);													// S2E
			sxa += sxd; exa += exd;
			do {
				if (a <= sy) {
					g->p.pos.y = y+a; g->p.pos.x = NONFIXED(sxa); g->p.e.pos2.x = NONFIXED(exa); hline_clip(g);		// S2E
					sxa += sxd; exa += exd;
				} else if (a <= ey) {
					g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = NONFIXED(exa); hline_clip(g);					// C2E
					exa += exd;
				} else if (qtr & 4) {
					g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);							// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (b <= sy) {
						g->p.pos.y = y+b; g->p.pos.x = NONFIXED(sxb); g->p.e.pos2.x = NONFIXED(exb); hline_clip(g);	// S2E
						sxb -= sxd; exb -= exd;
					} else if (b <= ey) {
						g->p.pos.y = y+b; g->p.pos.x = x-a; g->p.e.pos2.x = NONFIXED(exb); hline_clip(g);				// C2E
						exb -= exd;
					} else if (qtr & 4) {
						g->p.pos.y = y+b; g->p.pos.x = x-a; g->p.e.pos2.x = x+a; hline_clip(g);						// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (a <= sy) {
				g->p.pos.y = y+a; g->p.pos.x = NONFIXED(sxa); g->p.e.pos2.x = NONFIXED(exa); hline_clip(g);			// S2E
			} else if (a <= ey) {
				g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = NONFIXED(exa); hline_clip(g);						// C2E
			} else if (qtr & 4) {
				g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);								// C2C
			}
			break;

		case 11:	// S4E3 sy <= ey
		case 15:	// S4E4 sy <= ey
			g->p.pos.y = y; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);									// C2C
			do {
				g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);							// C2C
				if (a <= sy) {
					g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = NONFIXED(exa); hline_clip(g);				// C2E
					g->p.pos.y = y+a; g->p.pos.x = NONFIXED(sxa); g->p.e.pos2.x = x+b; hline_clip(g);				// S2C
					sxa += sxd; exa += exd;
				} else if (a <= ey) {
					g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = NONFIXED(exa); hline_clip(g);				// C2E
					exa += exd;
				} else if (qtr & 4) {
					g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);						// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					g->p.pos.y = y-b; g->p.pos.x = x-a; g->p.e.pos2.x = x+a; hline_clip(g);						// C2C
					if (b <= sy) {
						g->p.pos.y = y+b; g->p.pos.x = x-a; g->p.e.pos2.x = NONFIXED(exb); hline_clip(g);			// C2E
						g->p.pos.y = y+b; g->p.pos.x = NONFIXED(sxb); g->p.e.pos2.x = x+a; hline_clip(g);			// S2C
						sxb -= sxd; exb -= exd;
					} else if (b <= ey) {
						g->p.pos.y = y+b; g->p.pos.x = x-a; g->p.e.pos2.x = NONFIXED(exb); hline_clip(g);			// C2E
						exb -= exd;
					} else if (qtr & 4) {
						g->p.pos.y = y+b; g->p.pos.x = x-a; g->p.e.pos2.x = x+a; hline_clip(g);					// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);								// C2C
			if (a <= sy) {
				g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = NONFIXED(exa); hline_clip(g);					// C2E
				g->p.pos.y = y+a; g->p.pos.x = NONFIXED(sxa); g->p.e.pos2.x = x+b; hline_clip(g);					// S2C
			} else if (a <= ey) {
				g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = NONFIXED(exa); hline_clip(g);					// C2E
			} else if (qtr & 4) {
				g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);							// C2C
			}
			break;

		case 16:	// S2E2	sy > ey
		case 20:	// S2E1 sy > ey
			g->p.pos.y = y; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);									// C2C
			sxa -= sxd; exa -= exd;
			do {
				if (-a >= sy) {
					g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = NONFIXED(sxa); hline_clip(g);				// C2S
					g->p.pos.y = y-a; g->p.pos.x = NONFIXED(exa); g->p.e.pos2.x = x+b; hline_clip(g);				// E2C
					sxa -= sxd; exa -= exd;
				} else if (-a >= ey) {
					g->p.pos.y = y-a; g->p.pos.x = NONFIXED(exa); g->p.e.pos2.x = x+b; hline_clip(g);				// E2C
					exa -= exd;
				} else if (!(qtr & 4)){
					g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g); 						// C2C
				}
				g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g); 							// C2C
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (-b >= sy) {
						g->p.pos.y = y-b; g->p.pos.x = x-a; g->p.e.pos2.x = NONFIXED(sxb); hline_clip(g);			// C2S
						g->p.pos.y = y-b; g->p.pos.x = NONFIXED(exb); g->p.e.pos2.x = x+a; hline_clip(g);			// E2C
						sxb += sxd; exb += exd;
					} else if (-b >= ey) {
						g->p.pos.y = y-b; g->p.pos.x = NONFIXED(exb); g->p.e.pos2.x = x+a; hline_clip(g);			// E2C
						exb += exd;
					} else if (!(qtr & 4)){
						g->p.pos.y = y-b; g->p.pos.x = x-a; g->p.e.pos2.x = x+a; hline_clip(g); 					// C2C
					}
					g->p.pos.y = y+b; g->p.pos.x = x-a; g->p.e.pos2.x = x+a; hline_clip(g); 						// C2C
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (-a >= sy) {
				g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = NONFIXED(sxa); hline_clip(g);					// C2S
				g->p.pos.y = y-a; g->p.pos.x = NONFIXED(exa); g->p.e.pos2.x = x+b; hline_clip(g);					// E2C
			} else if (-a >= ey) {
				g->p.pos.y = y-a; g->p.pos.x = NONFIXED(exa); g->p.e.pos2.x = x+b; hline_clip(g);					// E2C
			} else if (!(qtr & 4)){
				g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g); 							// C2C
			}
			g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g); 								// C2C
			break;

		case 17:	// S1E2 sy > ey
		case 21:	// S1E1 sy > ey
			if (sy) {
				g->p.pos.x = x; g->p.e.pos2.x = x;																// E2S
				sxa -= sxd; exa -= exd;
			} else {
				g->p.pos.x = x; g->p.e.pos2.x = x+b;																// E2C
				exa -= exd;
			}
			g->p.pos.y = y;
			hline_clip(g);
			do {
				if (-a >= sy) {
					g->p.pos.y = y-a; g->p.pos.x = NONFIXED(exa); g->p.e.pos2.x = NONFIXED(sxa); hline_clip(g);		// E2S
					sxa -= sxd; exa -= exd;
				} else if (-a >= ey) {
					g->p.pos.y = y-a; g->p.pos.x = NONFIXED(exa); g->p.e.pos2.x = x+b; hline_clip(g);					// E2C
					exa -= exd;
				} else if (!(qtr & 4)) {
					g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);							// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (-b >= sy) {
						g->p.pos.y = y-b; g->p.pos.x = NONFIXED(exb); g->p.e.pos2.x = NONFIXED(sxb); hline_clip(g);	// E2S
						sxb += sxd; exb += exd;
					} else if (-b >= ey) {
						g->p.pos.y = y-b; g->p.pos.x = NONFIXED(exb); g->p.e.pos2.x = x+a; hline_clip(g);				// E2C
						exb += exd;
					} else if (!(qtr & 4)) {
						g->p.pos.y = y-b; g->p.pos.x = x-a; g->p.e.pos2.x = x+a; hline_clip(g);						// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (-a >= sy) {
				g->p.pos.y = y-a; g->p.pos.x = NONFIXED(exa); g->p.e.pos2.x = NONFIXED(sxa); hline_clip(g);			// E2S
			} else if (-a >= ey) {
				g->p.pos.y = y-a; g->p.pos.x = NONFIXED(exa); g->p.e.pos2.x = x+b; hline_clip(g);						// E2C
			} else if (!(qtr & 4)) {
				g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);								// C2C
			}
			break;

		case 26:	// S3E3 sy > ey
		case 27:	// S4E3 sy > ey
			g->p.pos.y = y; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);									// C2C
			do {
				g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);							// C2C
				if (a <= ey) {
					g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = NONFIXED(exa); hline_clip(g);				// C2E
					g->p.pos.y = y+a; g->p.pos.x = NONFIXED(sxa); g->p.e.pos2.x = x+b; hline_clip(g);				// S2C
					sxa += sxd; exa += exd;
				} else if (a <= sy) {
					g->p.pos.y = y+a; g->p.pos.x = NONFIXED(sxa); g->p.e.pos2.x = x+b; hline_clip(g);				// S2C
					sxa += sxd;
				} else if (!(qtr & 1)) {
					g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);						// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					g->p.pos.y = y-b; g->p.pos.x = x-a; g->p.e.pos2.x = x+a; hline_clip(g);						// C2C
					if (b <= ey) {
						g->p.pos.y = y+b; g->p.pos.x = x-a; g->p.e.pos2.x = NONFIXED(exb); hline_clip(g);			// C2E
						g->p.pos.y = y+b; g->p.pos.x = NONFIXED(sxb); g->p.e.pos2.x = x+a; hline_clip(g);			// S2C
						sxb -= sxd; exb -= exd;
					} else if (b <= sy) {
						g->p.pos.y = y+b; g->p.pos.x = NONFIXED(sxb); g->p.e.pos2.x = x+a; hline_clip(g);			// S2C
						sxb -= sxd;
					} else if (!(qtr & 1)) {
						g->p.pos.y = y+b; g->p.pos.x = x-a; g->p.e.pos2.x = x+a; hline_clip(g);					// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			g->p.pos.y = y-a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);								// C2C
			if (a <= ey) {
				g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = NONFIXED(exa); hline_clip(g);					// C2E
				g->p.pos.y = y+a; g->p.pos.x = NONFIXED(sxa); g->p.e.pos2.x = x+b; hline_clip(g);					// S2C
			} else if (a <= sy) {
				g->p.pos.y = y+a; g->p.pos.x = NONFIXED(sxa); g->p.e.pos2.x = x+b; hline_clip(g);					// S2C
			} else if (!(qtr & 4)) {
				g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);							// C2C
			}
			break;

		case 30:	// S3E4 sy > ey
		case 31:	// S4E4 sy > ey
			do {
				if (a <= ey) {
					g->p.pos.y = y+a; g->p.pos.x = NONFIXED(sxa); g->p.e.pos2.x = NONFIXED(exa); hline_clip(g);		// S2E
					sxa += sxd; exa += exd;
				} else if (a <= sy) {
					g->p.pos.y = y+a; g->p.pos.x = NONFIXED(sxa); g->p.e.pos2.x = x+b; hline_clip(g);					// S2C
					sxa += sxd;
				} else if (!(qtr & 1)) {
					g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);							// C2C
				}
				if (P < 0) {
					P += 3 + 2*a++;
				} else {
					if (b <= ey) {
						g->p.pos.y = y+b; g->p.pos.x = NONFIXED(sxb); g->p.e.pos2.x = NONFIXED(exb); hline_clip(g);	// S2E
						sxb -= sxd; exb -= exd;
					} else if (b <= sy) {
						g->p.pos.y = y+b; g->p.pos.x = NONFIXED(sxb); g->p.e.pos2.x = x+a; hline_clip(g);				// S2C
						sxb -= sxd;
					} else if (!(qtr & 1)) {
						g->p.pos.y = y+b; g->p.pos.x = x-a; g->p.e.pos2.x = x+a; hline_clip(g);						// C2C
					}
					P += 5 + 2*(a++ - b--);
				}
			} while(a < b);
			if (a <= ey) {
				g->p.pos.y = y+a; g->p.pos.x = NONFIXED(sxa); g->p.e.pos2.x = NONFIXED(exa); hline_clip(g);			// S2E
			} else if (a <= sy) {
				g->p.pos.y = y+a; g->p.pos.x = NONFIXED(sxa); g->p.e.pos2.x = x+b; hline_clip(g);						// S2C
			} else if (!(qtr & 4)) {
				g->p.pos.y = y+a; g->p.pos.x = x-b; g->p.e.pos2.x = x+b; hline_clip(g);								// C2C
			}
			break;
		}

		autoflush(g);
		MUTEX_EXIT(g);
	}
#endif

#if GDISP_NEED_ARC || GDISP_NEED_ARCSECTORS
	void gdispGDrawRoundedBox(GDisplay *g, gCoord x, gCoord y, gCoord cx, gCoord cy, gCoord radius, gColor color) {
		if (2*radius > cx || 2*radius > cy) {
			gdispGDrawBox(g, x, y, cx, cy, color);
			return;
		}

		#if GDISP_NEED_ARCSECTORS
			gdispGDrawArcSectors(g, x+radius, y+radius, radius, 0x0C, color);
			gdispGDrawArcSectors(g, x+cx-1-radius, y+radius, radius, 0x03, color);
			gdispGDrawArcSectors(g, x+cx-1-radius, y+cy-1-radius, radius, 0xC0, color);
			gdispGDrawArcSectors(g, x+radius, y+cy-1-radius, radius, 0x30, color);
		#else
			gdispGDrawArc(g, x+radius, y+radius, radius, 90, 180, color);
			gdispGDrawArc(g, x+cx-1-radius, y+radius, radius, 0, 90, color);
			gdispGDrawArc(g, x+cx-1-radius, y+cy-1-radius, radius, 270, 360, color);
			gdispGDrawArc(g, x+radius, y+cy-1-radius, radius, 180, 270, color);
		#endif
		gdispGDrawLine(g, x+radius+1, y, x+cx-2-radius, y, color);
		gdispGDrawLine(g, x+cx-1, y+radius+1, x+cx-1, y+cy-2-radius, color);
		gdispGDrawLine(g, x+radius+1, y+cy-1, x+cx-2-radius, y+cy-1, color);
		gdispGDrawLine(g, x, y+radius+1, x, y+cy-2-radius, color);
	}
#endif

#if GDISP_NEED_ARC || GDISP_NEED_ARCSECTORS
	void gdispGFillRoundedBox(GDisplay *g, gCoord x, gCoord y, gCoord cx, gCoord cy, gCoord radius, gColor color) {
		gCoord radius2;

		radius2 = radius*2;
		if (radius2 > cx || radius2 > cy) {
			gdispGFillArea(g, x, y, cx, cy, color);
			return;
		}
		#if GDISP_NEED_ARCSECTORS
			gdispGFillArcSectors(g, x+radius, y+radius, radius, 0x0C, color);
			gdispGFillArcSectors(g, x+cx-1-radius, y+radius, radius, 0x03, color);
			gdispGFillArcSectors(g, x+cx-1-radius, y+cy-1-radius, radius, 0xC0, color);
			gdispGFillArcSectors(g, x+radius, y+cy-1-radius, radius, 0x30, color);
		#else
			gdispGFillArc(g, x+radius, y+radius, radius, 90, 180, color);
			gdispGFillArc(g, x+cx-1-radius, y+radius, radius, 0, 90, color);
			gdispGFillArc(g, x+cx-1-radius, y+cy-1-radius, radius, 270, 360, color);
			gdispGFillArc(g, x+radius, y+cy-1-radius, radius, 180, 270, color);
		#endif
		gdispGFillArea(g, x+radius+1, y, cx-radius2, radius, color);
		gdispGFillArea(g, x+radius+1, y+cy-radius, cx-radius2, radius, color);
		gdispGFillArea(g, x, y+radius, cx, cy-radius2, color);
	}
#endif

#if GDISP_NEED_PIXELREAD
	gColor gdispGGetPixelColor(GDisplay *g, gCoord x, gCoord y) {
		gColor		c;

		/* Always synchronous as it must return a value */
		MUTEX_ENTER(g);
		#if GDISP_DRIVER_PIXELREAD
			#if GDISP_DRIVER_PIXELREAD == GFXSOME
				if (gvmt(g)->get)
			#endif
			{
				// Best is direct pixel read
				g->p.pos.x = x;
				g->p.pos.y = y;
				c = gdisp_lld_get_pixel_color(g);
				MUTEX_EXIT(g);
				return c;
			}
		#endif
		#if GDISP_DRIVER_PIXELREAD != GFXON && GDISP_DRIVER_STREAM_READ
			#if GDISP_DRIVER_STREAM_READ == GFXSOME
				if (gvmt(g)->readcolor)
			#endif
			{
				// Next best is hardware streaming
				g->p.pos.x = x;
				g->p.pos.y = y;
				g->p.cx = 1;
				g->p.cy = 1;
				gdisp_lld_read_start(g);
				c = gdisp_lld_read_color(g);
				gdisp_lld_read_stop(g);
				MUTEX_EXIT(g);
				return c;
			}
		#endif
		#if GDISP_DRIVER_PIXELREAD != GFXON && GDISP_DRIVER_STREAM_READ != GFXON
			#if !GDISP_DRIVER_PIXELREAD && !GDISP_DRIVER_STREAM_READ
				// Worst is "not possible"
				#error "GDISP: GDISP_NEED_PIXELREAD has been set but there is no hardware support for reading the display"
			#endif
			MUTEX_EXIT(g);
			return 0;
		#endif
	}
#endif

//TODO: Up to here in API translation

#if GDISP_NEED_SCROLL
	void gdispGVerticalScroll(GDisplay *g, gCoord x, gCoord y, gCoord cx, gCoord cy, int lines, gColor bgcolor) {
		gCoord		abslines;
		#if GDISP_DRIVER_SCROLL != GFXON
			gCoord 	fy, dy, ix, fx, i, j;
		#endif

		if (!lines) return;

		MUTEX_ENTER(g);
		#if GDISP_NEED_VALIDATION || GDISP_NEED_CLIP
			if (x < g->clip.p1.x) { cx -= g->clip.p1.x - x; x = g->clip.p1.x; }
			if (y < g->clip.p1.y) { cy -= g->clip.p1.y - y; y = g->clip.p1.y; }
			if (cx <= 0 || cy <= 0 || x >= g->clip.p2.x || y >= g->clip.p2.y) { MUTEX_EXIT(g); return; }
			if (x+cx > g->clip.p2.x)	cx = g->clip.p2.x - x;
			if (y+cy > g->clip.p2.y)	cy = g->clip.p2.y - y;
		#endif

		abslines = lines < 0 ? -lines : lines;
		if (abslines >= cy) {
			abslines = cy;
			cy = 0;
		} else {
			// Best is hardware scroll
			#if GDISP_DRIVER_SCROLL
				#if GDISP_DRIVER_SCROLL == GFXSOME
					if (gvmt(g)->vscroll)
				#endif
				{
					g->p.pos.x = x;
					g->p.pos.y = y;
					g->p.cx = cx;
					g->p.cy = cy;
					g->p.e.pos2.y = lines;
					g->p.color = bgcolor;
					gdisp_lld_vertical_scroll(g);
					cy -= abslines;
				}
				#if GDISP_DRIVER_SCROLL == GFXSOME
					else
				#endif
			#elif GDISP_LINEBUF_SIZE == 0
				#error "GDISP: GDISP_NEED_SCROLL is set but there is no hardware support and GDISP_LINEBUF_SIZE is zero."
			#endif

			// Scroll Emulation
			#if GDISP_DRIVER_SCROLL != GFXON
				{
					cy -= abslines;
					if (lines < 0) {
						fy = y+cy-1;
						dy = -1;
					} else {
						fy = y;
						dy = 1;
					}
					// Move the screen - one line at a time
					for(i = 0; i < cy; i++, fy += dy) {

						// Handle where the buffer is smaller than a line
						for(ix=0; ix < cx; ix += GDISP_LINEBUF_SIZE) {

							// Calculate the data we can move in one operation
							fx = cx - ix;
							if (fx > GDISP_LINEBUF_SIZE)
								fx = GDISP_LINEBUF_SIZE;

							// Read one line of data from the screen

							// Best line read is hardware streaming
							#if GDISP_DRIVER_STREAM_READ
								#if GDISP_DRIVER_STREAM_READ == GFXSOME
									if (gvmt(g)->readstart)
								#endif
								{
									g->p.pos.x = x+ix;
									g->p.pos.y = fy+lines;
									g->p.cx = fx;
									g->p.cy = 1;
									gdisp_lld_read_start(g);
									for(j=0; j < fx; j++)
										g->linebuf[j] = gdisp_lld_read_color(g);
									gdisp_lld_read_stop(g);
								}
								#if GDISP_DRIVER_STREAM_READ == GFXSOME
									else
								#endif
							#endif

							// Next best line read is single pixel reads
							#if GDISP_DRIVER_STREAM_READ != GFXON && GDISP_DRIVER_PIXELREAD
								#if GDISP_DRIVER_PIXELREAD == GFXSOME
									if (gvmt(g)->get)
								#endif
								{
									for(j=0; j < fx; j++) {
										g->p.pos.x = x+ix+j;
										g->p.pos.y = fy+lines;
										g->linebuf[j] = gdisp_lld_get_pixel_color(g);
									}
								}
								#if GDISP_DRIVER_PIXELREAD == GFXSOME
									else {
										// Worst is "not possible"
										MUTEX_EXIT(g);
										return;
									}
								#endif
							#endif

							// Worst is "not possible"
							#if !GDISP_DRIVER_STREAM_READ && !GDISP_DRIVER_PIXELREAD
								#error "GDISP: GDISP_NEED_SCROLL is set but there is no hardware support for scrolling or reading pixels."
							#endif

							// Write that line to the new location

							// Best line write is hardware bitfills
							#if GDISP_DRIVER_BITFILLS
								#if GDISP_DRIVER_BITFILLS == GFXSOME
									if (gvmt(g)->blit)
								#endif
								{
									g->p.pos.x = x+ix;
									g->p.pos.y = fy;
									g->p.cx = fx;
									g->p.cy = 1;
									g->p.e.pos2.x = 0;
									g->p.e.pos2.y = 0;
									g->p.pos.x2 = fx;
									g->p.e.ptr = (void *)g->linebuf;
									gdisp_lld_blit_area(g);
								}
								#if GDISP_DRIVER_BITFILLS == GFXSOME
									else
								#endif
							#endif

							// Next best line write is hardware streaming
							#if GDISP_DRIVER_BITFILLS != GFXON && GDISP_DRIVER_STREAM_WRITE
								#if GDISP_DRIVER_STREAM_WRITE == GFXSOME
									if (gvmt(g)->writestart)
								#endif
								{
									g->p.pos.x = x+ix;
									g->p.pos.y = fy;
									g->p.cx = fx;
									g->p.cy = 1;
									gdisp_lld_start(g);
									#if GDISP_DRIVER_VMT_SETPOS
										gdisp_lld_setpos(g);
									#endif
									for(j = 0; j < fx; j++) {
										g->p.color = g->linebuf[j];
										gdisp_lld_write(g);
									}
									gdisp_lld_write_stop(g);
								}
								#if GDISP_DRIVER_STREAM_WRITE == GFXSOME
									else
								#endif
							#endif

							// Next best line write is drawing pixels in combination with filling
							#if GDISP_DRIVER_BITFILLS != GFXON && GDISP_DRIVER_STREAM_WRITE != GFXON && GDISP_DRIVER_FILLS && GDISP_DRIVER_DRAWPIXEL
								// We don't need to test for auto-detect on drawpixel as we know we have it because we don't have streaming.
								#if GDISP_DRIVER_FILLS == GFXSOME
									if (gvmt(g)->fill)
								#endif
								{
									g->p.pos.y = fy;
									g->p.cy = 1;
									g->p.pos.x = x+ix;
									g->p.cx = 1;
									for(j = 0; j < fx; ) {
										g->p.color = g->linebuf[j];
										if (j + g->p.cx < fx && g->linebuf[j] == g->linebuf[j + g->p.cx])
											g->p.cx++;
										else if (g->p.cx == 1) {
											gdisp_lld_draw_pixel(g);
											j++;
											g->p.pos.x++;
										} else {
											gdisp_lld_fill_area(g);
											j += g->p.cx;
											g->p.pos.x += g->p.cx;
											g->p.cx = 1;
										}
									}
								}
								#if GDISP_DRIVER_FILLS == GFXSOME
									else
								#endif
							#endif

							// Worst line write is drawing pixels
							#if GDISP_DRIVER_BITFILLS != GFXON && GDISP_DRIVER_STREAM_WRITE != GFXON && GDISP_DRIVER_FILLS != GFXON && GDISP_DRIVER_DRAWPIXEL
								// The following test is unneeded because we are guaranteed to have draw pixel if we don't have streaming
								//#if GDISP_DRIVER_DRAWPIXEL == GFXSOME
								//	if (gvmt(g)->pixel)
								//#endif
								{
									g->p.pos.y = fy;
									for(g->p.pos.x = x+ix, j = 0; j < fx; g->p.pos.x++, j++) {
										g->p.color = g->linebuf[j];
										gdisp_lld_draw_pixel(g);
									}
								}
							#endif
						}
					}
				}
			#endif
		}

		/* fill the remaining gap */
		g->p.pos.x = x;
		g->p.pos.y = lines > 0 ? (y+cy) : y;
		g->p.cx = cx;
		g->p.cy = abslines;
		g->p.color = bgcolor;
		fillarea(g);
		autoflush(g);
		MUTEX_EXIT(g);
	}
#endif

#if GDISP_NEED_CONTROL
	#if GDISP_DRIVER_CONTROL
		void gdispGControl(GDisplay *g, unsigned what, void *value) {
			#if GDISP_DRIVER_CONTROL == GFXSOME
				if (!gvmt(g)->control)
					return;
			#endif
			MUTEX_ENTER(g);
			g->p.pos.x = what;
			g->p.e.ptr = value;
			if (what == GDISP_CONTROL_ORIENTATION) {
				switch ((orientation_t) value) {
				case GDISP_ROTATE_LANDSCAPE:
					g->p.e.ptr = g->g.Width >= g->g.Height ? (void *)GDISP_ROTATE_0 : (void *)GDISP_ROTATE_90;
					break;
				case GDISP_ROTATE_PORTRAIT:
					g->p.e.ptr = g->g.Width >= g->g.Height ? (void *)GDISP_ROTATE_90 : (void *)GDISP_ROTATE_0;
					break;
				default:
					break;
				}
			}
			gdisp_lld_control(g);
			#if GDISP_NEED_CLIP || GDISP_NEED_VALIDATION
				if (what == GDISP_CONTROL_ORIENTATION) {
					g->clip.p1.x = 0;
					g->clip.p1.y = 0;
					g->clip.p2.x = g->g.Width;
					g->clip.p2.y = g->g.Height;
				}
			#endif
			MUTEX_EXIT(g);
		}
	#else
		void gdispGControl(GDisplay *g, unsigned what, void *value) {
			(void)g;
			(void)what;
			(void)value;
			/* Ignore everything */
		}
	#endif
#endif

#if GDISP_NEED_QUERY
	#if GDISP_DRIVER_QUERY
		void *gdispGQuery(GDisplay *g, unsigned what) {
			void *res;

			#if GDISP_DRIVER_QUERY == GFXSOME
				if (!gvmt(g)->query)
					return -1;
			#endif
			MUTEX_ENTER(g);
			g->p.pos.x = (gCoord)what;
			res = gdisp_lld_query(g);
			MUTEX_EXIT(g);
			return res;
		}
	#else
		void *gdispGQuery(GDisplay *g, unsigned what) {
			(void) what;
			return (void *)-1;
		}
	#endif
#endif

/*===========================================================================*/
/* High Level Driver Routines.                                               */
/*===========================================================================*/

void gdispGDrawBox(GDisplay *g, gCoord x, gCoord y, gCoord cx, gCoord cy, gColor color) {
	if (cx <= 0 || cy <= 0) return;
	cx = x+cx-1; cy = y+cy-1;			// cx, cy are now the end point.

	MUTEX_ENTER(g);

	g->p.color = color;

	if (cx - x > 2) {
		g->p.pos.x = x; g->p.pos.y = y; g->p.e.pos2.x = cx; hline_clip(g);
		if (y != cy) {
			g->p.pos.x = x; g->p.pos.y = cy; g->p.e.pos2.x = cx; hline_clip(g);
			if (cy - y > 2) {
				y++; cy--;
				g->p.pos.x = x; g->p.pos.y = y; g->p.e.pos2.y = cy; vline_clip(g);
				g->p.pos.x = cx; g->p.pos.y = y; g->p.e.pos2.y = cy; vline_clip(g);
			}
		}
	} else {
		g->p.pos.x = x; g->p.pos.y = y; g->p.e.pos2.y = cy; vline_clip(g);
		if (x != cx) {
			g->p.pos.x = cx; g->p.pos.y = y; g->p.e.pos2.y = cy; vline_clip(g);
		}
	}

	autoflush(g);
	MUTEX_EXIT(g);
}

#if GDISP_NEED_CONVEX_POLYGON
	void gdispGDrawPoly(GDisplay *g, gCoord tx, gCoord ty, const gPoint *pntarray, unsigned cnt, gColor color) {
		const gPoint	*epnt, *p;

		epnt = &pntarray[cnt-1];

		MUTEX_ENTER(g);
		g->p.color = color;
		for(p = pntarray; p < epnt; p++) {
			g->p.pos.x=tx+p->x; g->p.pos.y=ty+p->y; g->p.e.pos2.x=tx+p[1].x; g->p.e.pos2.y=ty+p[1].y; line_clip(g);
		}
		g->p.pos.x=tx+p->x; g->p.pos.y=ty+p->y; g->p.e.pos2.x=tx+pntarray->x; g->p.e.pos2.y=ty+pntarray->y; line_clip(g);

		autoflush(g);
		MUTEX_EXIT(g);
	}

	void gdispGFillConvexPoly(GDisplay *g, gCoord tx, gCoord ty, const gPoint *pntarray, unsigned cnt, gColor color) {
		const gPoint	*lpnt, *rpnt, *epnts;
		fixed			lx, rx, lk, rk;
		gCoord			y, ymax, lxc, rxc;

		epnts = &pntarray[cnt-1];

		/* Find a top point */
		rpnt = pntarray;
		for(lpnt=pntarray+1; lpnt <= epnts; lpnt++) {
			if (lpnt->y < rpnt->y)
				rpnt = lpnt;
		}
		lx = rx = FIXED(rpnt->x);
		y = rpnt->y;

		/* Work out the slopes of the two attached line segs */
		for (lpnt = rpnt <= pntarray ? epnts : rpnt-1; lpnt->y == y; cnt--) {
			if (!cnt) return;
			lx = FIXED(lpnt->x);
			lpnt = lpnt <= pntarray ? epnts : lpnt-1;
		}
		for (rpnt = rpnt >= epnts ? pntarray : rpnt+1; rpnt->y == y; cnt--) {
			if (!cnt) return;
			rx = FIXED(rpnt->x);
			rpnt = rpnt >= epnts ? pntarray : rpnt+1;
		}
		lk = (FIXED(lpnt->x) - lx) / (lpnt->y - y);
		rk = (FIXED(rpnt->x) - rx) / (rpnt->y - y);

		// Add error correction for rounding
		lx += FIXED0_5;
		rx += FIXED0_5;

		// Do all the line segments
		MUTEX_ENTER(g);
		g->p.color = color;
		while(1) {
			/* Determine our boundary */
			ymax = rpnt->y < lpnt->y ? rpnt->y : lpnt->y;

			/* Scan down the line segments until we hit a boundary */
			for(; y < ymax; y++) {
				lxc = NONFIXED(lx);
				rxc = NONFIXED(rx);
				/*
				 * Doesn't print the right hand point in order to allow polygon joining.
				 * Also ensures that we draw from left to right with the minimum number
				 * of pixels.
				 */
				if (lxc < rxc) {
					g->p.pos.x=tx+lxc; g->p.pos.y=ty+y; g->p.e.pos2.x=tx+rxc-1; hline_clip(g);
				} else if (lxc > rxc) {
					g->p.pos.x=tx+rxc; g->p.pos.y=ty+y; g->p.e.pos2.x=tx+lxc-1; hline_clip(g);
				}

				lx += lk;
				rx += rk;
			}

			if (!cnt) {
				autoflush(g);
				MUTEX_EXIT(g);
				return;
			}
			cnt--;

			/* Replace the appropriate point */
			if (ymax == lpnt->y) {
				lx -= FIXED0_5;
				for (lpnt = lpnt <= pntarray ? epnts : lpnt-1; lpnt->y == y; cnt--) {
					if (!cnt) {
						autoflush(g);
						MUTEX_EXIT(g);
						return;
					}
					lx = FIXED(lpnt->x);
					lpnt = lpnt <= pntarray ? epnts : lpnt-1;
				}
				lk = (FIXED(lpnt->x) - lx) / (lpnt->y - y);
				lx += FIXED0_5;
			} else {
				rx -= FIXED0_5;
				for (rpnt = rpnt >= epnts ? pntarray : rpnt+1; rpnt->y == y; cnt--) {
					if (!cnt) {
						autoflush(g);
						MUTEX_EXIT(g);
						return;
					}
					rx = FIXED(rpnt->x);
					rpnt = rpnt >= epnts ? pntarray : rpnt+1;
				}
				rk = (FIXED(rpnt->x) - rx) / (rpnt->y - y);
				rx += FIXED0_5;
			}
		}
	}

	static int32_t rounding_div(const int32_t n, const int32_t d)
	{
		if ((n < 0) != (d < 0))
			return (n - d/2) / d;
		else
			return (n + d/2) / d;
	}

	/* Find a vector (nx, ny) that is perpendicular to (dx, dy) and has length
	 * equal to 'norm'. */
	static void get_normal_vector(gCoord dx, gCoord dy, gCoord norm, gCoord *nx, gCoord *ny)
	{
		gCoord absDx, absDy;
		int32_t len_n, len, len2;
		char maxSteps;

		/* Take the absolute value of dx and dy, multiplied by 2 for precision */
		absDx = (dx >= 0 ? dx : -dx) * 2;
		absDy = (dy >= 0 ? dy : -dy) * 2;

		/* Compute the quadrate length */
		len2 = absDx * absDx + absDy * absDy;

		/* First aproximation : length = |dx| + |dy| */
		len = absDx + absDy;

		/* Give a max number of steps, the calculation usually takes 3 or 4 */
		for(maxSteps = 8; maxSteps > 0; maxSteps--)
		{
			/* Use an adapted version of Newton's algorithm to find the correct length 
			 * This calculation converge quadratically towards the correct length
			 * n(x+1) = (n(x) + len^2 / n(x)) / 2 
			 */
			len_n = (len + len2 / len) / 2;

			/* We reach max precision when the last result is equal or greater than the previous one */ 
			if(len_n >= len){
				break;
			}

			len = len_n;
		}

		/* Compute the normal vector using nx = dy * desired length / vector length
		 * The solution is rounded to the nearest integer
		 */
		*nx = rounding_div(dy * norm * 2, len);
		*ny = rounding_div(-dx * norm * 2, len);
		return;
	}

	void gdispGDrawThickLine(GDisplay *g, gCoord x0, gCoord y0, gCoord x1, gCoord y1, gColor color, gCoord width, bool_t round) {
		gCoord dx, dy, nx = 0, ny = 0;

		/* Compute the direction vector for the line */
		dx = x1 - x0;
		dy = y1 - y0;

		/* Draw a small dot if the line length is zero. */
		if (dx == 0 && dy == 0)
			dx += 1;

		/* Compute a normal vector with length 'width'. */
		get_normal_vector(dx, dy, width, &nx, &ny);

		/* Handle 1px wide lines gracefully */
		if (nx == 0 && ny == 0)
			nx = 1;

		/* Offset the x0,y0 by half the width of the line. This way we
		 * can keep the width of the line accurate even if it is not evenly
		 * divisible by 2.
		 */
		{
			x0 -= rounding_div(nx, 2);
			y0 -= rounding_div(ny, 2);
		}

		/* Fill in the point array */
		if (!round) {
			/* We use 4 points for the basic line shape:
			 *
			 *  pt1                                      pt2
			 * (+n) ------------------------------------ (d+n)
			 *   |                                       |
			 * (0,0) ----------------------------------- (d)
			 *  pt0                                      pt3
			 */
			gPoint pntarray[4];

			pntarray[0].x = 0;
			pntarray[0].y = 0;
			pntarray[1].x = nx;
			pntarray[1].y = ny;
			pntarray[2].x = dx + nx;
			pntarray[2].y = dy + ny;
			pntarray[3].x = dx;
			pntarray[3].y = dy;

			gdispGFillConvexPoly(g, x0, y0, pntarray, 4, color);
		} else {
			/* We use 4 points for basic shape, plus 4 extra points for ends:
			 *
			 *           pt3 ------------------ pt4
			 *          /                         \
			 *        pt2                        pt5
			 *         |                          |
			 *        pt1                        pt6
			 *         \                         /
			 *          pt0 -------------------pt7
			 */
			gPoint pntarray[8];
			gCoord nx2, ny2;

			/* Magic numbers:
			 * 75/256  = sin(45) / (1 + sqrt(2))		diagonal octagon segments
			 * 106/256 = 1 / (1 + sqrt(2))				octagon side
			 * 53/256  = 0.5 / (1 + sqrt(2))			half of octagon side
			 * 150/256 = 1 - 1 / (1 + sqrt(2))	  		octagon height minus one side
			 */

			/* Rotate the normal vector 45 deg counter-clockwise and reduce
			 * to 1 / (1 + sqrt(2)) length, for forming octagonal ends. */
			nx2 = rounding_div((nx * 75 + ny * 75), 256);
			ny2 = rounding_div((-nx * 75 + ny * 75), 256);

			/* Offset and extend the line so that the center of the octagon
			 * is at the specified points. */
			x0 += ny * 53 / 256;
			y0 -= nx * 53 / 256;
			dx -= ny * 106 / 256;
			dy += nx * 106 / 256;

			/* Now fill in the points by summing the calculated vectors. */
			pntarray[0].x = 0;
			pntarray[0].y = 0;
			pntarray[1].x = nx2;
			pntarray[1].y = ny2;
			pntarray[2].x = nx2 + nx * 106/256;
			pntarray[2].y = ny2 + ny * 106/256;
			pntarray[3].x = nx;
			pntarray[3].y = ny;
			pntarray[4].x = dx + nx;
			pntarray[4].y = dy + ny;
			pntarray[5].x = dx + nx - nx2;
			pntarray[5].y = dy + ny - ny2;
			pntarray[6].x = dx + nx * 150/256 - nx2;
			pntarray[6].y = dy + ny * 150/256 - ny2;
			pntarray[7].x = dx;
			pntarray[7].y = dy;

			gdispGFillConvexPoly(g, x0, y0, pntarray, 8, color);
		}
	}
#endif

#if GDISP_NEED_TEXT
	#include "mcufont/mcufont.h"

	#if GDISP_NEED_ANTIALIAS && GDISP_DRIVER_PIXELREAD
		static void drawcharline(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void *state) {
			#define GD	((GDisplay *)state)
			if (y < GD->t.clip.p1.y || y >= GD->t.clip.p2.y || x+count <= GD->t.clip.p1.x || x >= GD->t.clip.p2.x)
				return;
			if (x < GD->t.clip.p1.x) {
				count -= GD->t.clip.p1.x - x;
				x = GD->t.clip.p1.x;
			}
			if (x+count > GD->t.clip.p2.x)
				count = GD->t.clip.p2.x - x;
			if (alpha == 255) {
				GD->p.pos.x = x; GD->p.pos.y = y; GD->p.e.pos2.x = x+count-1; GD->p.color = GD->t.color;
				hline_clip(GD);
			} else {
				for (; count; count--, x++) {
					GD->p.pos.x = x; GD->p.pos.y = y;
					GD->p.color = gdispBlendColor(GD->t.color, gdisp_lld_get_pixel_color(GD), alpha);
					drawpixel_clip(GD);
				}
			}
			#undef GD
		}
	#else
		static void drawcharline(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void *state) {
			#define GD	((GDisplay *)state)
			if (y < GD->t.clip.p1.y || y >= GD->t.clip.p2.y || x+count <= GD->t.clip.p1.x || x >= GD->t.clip.p2.x)
				return;
			if (x < GD->t.clip.p1.x) {
				count -= GD->t.clip.p1.x - x;
				x = GD->t.clip.p1.x;
			}
			if (x+count > GD->t.clip.p2.x)
				count = GD->t.clip.p2.x - x;
			if (alpha > 0x80) {			// A best approximation when using anti-aliased fonts but we can't actually draw them anti-aliased
				GD->p.pos.x = x; GD->p.pos.y = y; GD->p.e.pos2.x = x+count-1; GD->p.color = GD->t.color;
				hline_clip(GD);
			}
			#undef GD
		}
	#endif

	#if GDISP_NEED_ANTIALIAS
		static void fillcharline(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void *state) {
			#define GD	((GDisplay *)state)
			if (y < GD->t.clip.p1.y || y >= GD->t.clip.p2.y || x+count <= GD->t.clip.p1.x || x >= GD->t.clip.p2.x)
				return;
			if (x < GD->t.clip.p1.x) {
				count -= GD->t.clip.p1.x - x;
				x = GD->t.clip.p1.x;
			}
			if (x+count > GD->t.clip.p2.x)
				count = GD->t.clip.p2.x - x;
			if (alpha == 255) {
				GD->p.color = GD->t.color;
			} else {
				GD->p.color = gdispBlendColor(GD->t.color, GD->t.bgcolor, alpha);
			}
			GD->p.pos.x = x; GD->p.pos.y = y; GD->p.e.pos2.x = x+count-1;
			hline_clip(GD);
			#undef GD
		}
	#else
		#define fillcharline	drawcharline
	#endif

	/* Callback to render characters. */
	static uint8_t drawcharglyph(int16_t x, int16_t y, mf_char ch, void *state) {
		#define GD	((GDisplay *)state)
			return mf_render_character(GD->t.font, x, y, ch, drawcharline, state);
		#undef GD
	}

	/* Callback to render characters. */
	static uint8_t fillcharglyph(int16_t x, int16_t y, mf_char ch, void *state) {
		#define GD	((GDisplay *)state)
			return mf_render_character(GD->t.font, x, y, ch, fillcharline, state);
		#undef GD
	}

	/* Callback to render string boxes with word wrap. */
	#if GDISP_NEED_TEXT_WORDWRAP
		static bool mf_countline_callback(mf_str line, uint16_t count, void *state) {
			(void) line;
			(void) count;

			((coord_t*)state)[0]++;
			return GTrue;
		}
		static bool mf_drawline_callback(mf_str line, uint16_t count, void *state) {
			#define GD	((GDisplay *)state)
				mf_render_aligned(GD->t.font, GD->t.wrapx, GD->t.wrapy, GD->t.lrj, line, count, drawcharglyph, state);
				GD->t.wrap.y += GD->t.font->line_height;
			#undef GD
			return GTrue;
		}
		static bool mf_fillline_callback(mf_str line, uint16_t count, void *state) {
			#define GD	((GDisplay *)state)
				mf_render_aligned(GD->t.font, GD->t.wrapx, GD->t.wrapy, GD->t.lrj, line, count, fillcharglyph, state);
				GD->t.wrap.y += GD->t.font->line_height;
			#undef GD
			return GTrue;
		}	
	#endif

	void gdispGDrawChar(GDisplay *g, gCoord x, gCoord y, uint16_t c, font_t font, gColor color) {
		if (!font)
			return;
		MUTEX_ENTER(g);
		g->t.font = font;
		g->t.clip.p1.x = x;
		g->t.clip.p1.y = y;
		g->t.clip.p2.x = x + mf_character_width(font, c) + font->baseline_x;
		g->t.clip.p2.y = y + font->height;
		g->t.color = color;
		mf_render_character(font, x, y, c, drawcharline, g);
		autoflush(g);
		MUTEX_EXIT(g);
	}

	void gdispGFillChar(GDisplay *g, gCoord x, gCoord y, uint16_t c, font_t font, gColor color, gColor bgcolor) {
		if (!font)
			return;
		MUTEX_ENTER(g);
		g->p.cx = mf_character_width(font, c) + font->baseline_x;
		g->p.cy = font->height;
		g->t.font = font;
		g->t.clip.p1.x = g->p.pos.x = x;
		g->t.clip.p1.y = g->p.pos.y = y;
		g->t.clip.p2.x = g->p.pos.x+g->p.cx;
		g->t.clip.p2.y = g->p.pos.y+g->p.cy;
		g->t.color = color;
		g->t.bgcolor = g->p.color = bgcolor;

		TEST_CLIP_AREA(g) {
			fillarea(g);
			mf_render_character(font, x, y, c, fillcharline, g);
		}
		autoflush(g);
		MUTEX_EXIT(g);
	}

	void gdispGDrawString(GDisplay *g, gCoord x, gCoord y, const char *str, font_t font, gColor color) {
		if (!font)
			return;
		MUTEX_ENTER(g);
		g->t.font = font;
		g->t.clip.p1.x = x;
		g->t.clip.p1.y = y;
		g->t.clip.p2.x = 32768;	//x + mf_get_string_width(font, str, 0, 0) + font->baseline_x;
		g->t.clip.p2.y = y + font->height;
		g->t.color = color;

		mf_render_aligned(font, x+font->baseline_x, y, MF_ALIGN_LEFT, str, 0, drawcharglyph, g);
		autoflush(g);
		MUTEX_EXIT(g);
	}

	void gdispGFillString(GDisplay *g, gCoord x, gCoord y, const char *str, font_t font, gColor color, gColor bgcolor) {
		if (!font)
			return;
		MUTEX_ENTER(g);
		g->p.cx = mf_get_string_width(font, str, 0, 0) + font->baseline_x;
		g->p.cy = font->height;
		g->t.font = font;
		g->t.clip.p1.x = g->p.pos.x = x;
		g->t.clip.p1.y = g->p.pos.y = y;
		g->t.clip.p2.x = g->p.pos.x+g->p.cx;
		g->t.clip.p2.y = g->p.pos.y+g->p.cy;
		g->t.color = color;
		g->t.bgcolor = g->p.color = bgcolor;

		TEST_CLIP_AREA(g) {
			fillarea(g);
			mf_render_aligned(font, x+font->baseline_x, y, MF_ALIGN_LEFT, str, 0, fillcharglyph, g);
		}

		autoflush(g);
		MUTEX_EXIT(g);
	}

	void gdispGDrawStringBox(GDisplay *g, gCoord x, gCoord y, gCoord cx, gCoord cy, const char* str, font_t font, gColor color, justify_t justify) {
		gCoord		totalHeight;

		if (!font)
			return;
		MUTEX_ENTER(g);

		// Apply padding
		#if GDISP_NEED_TEXT_BOXPADLR != 0 || GDISP_NEED_TEXT_BOXPADTB != 0
			if (!(justify & justifyNoPad)) {
				#if GDISP_NEED_TEXT_BOXPADLR != 0
					x += GDISP_NEED_TEXT_BOXPADLR;
					cx -= 2*GDISP_NEED_TEXT_BOXPADLR;
				#endif
				#if GDISP_NEED_TEXT_BOXPADTB != 0
					y += GDISP_NEED_TEXT_BOXPADTB;
					cy -= 2*GDISP_NEED_TEXT_BOXPADTB;
				#endif
			}
		#endif
			
		// Save the clipping area
		g->t.clip.p1.x = x;
		g->t.clip.p1.y = y;
		g->t.clip.p2.x = x+cx;
		g->t.clip.p2.y = y+cy;

		// Calculate the total text height
		#if GDISP_NEED_TEXT_WORDWRAP
			if (!(justify & justifyNoWordWrap)) {
				// Count the number of lines
				totalHeight = 0;
				mf_wordwrap(font, cx, str, mf_countline_callback, &totalHeight);
				totalHeight *= font->height;
			} else
		#endif
		totalHeight = font->height;

		// Select the anchor position
		switch((justify & JUSTIFYMASK_TOPBOTTOM)) {
		case justifyTop:
			break;
		case justifyBottom:
			y += cy - totalHeight;
			break;
		default:	// justifyMiddle
			y += (cy+1 - totalHeight)/2;
			break;
		}
		switch((justify & JUSTIFYMASK_LEFTRIGHT)) {
		case justifyCenter:
			x += (cx + 1) / 2;
			break;
		case justifyRight:
			x += cx;
			break;
		default:	// justifyLeft
			break;
		}

		/* Render */
		g->t.font = font;
		g->t.color = color;
		#if GDISP_NEED_TEXT_WORDWRAP
			if (!(justify & justifyNoWordWrap)) {
				g->t.lrj = (justify & JUSTIFYMASK_LEFTRIGHT);
				g->t.wrap.x = x;
				g->t.wrap.y = y;
				
				mf_wordwrap(font, cx, str, mf_drawline_callback, g);
			} else
		#endif
		mf_render_aligned(font, x, y, (justify & JUSTIFYMASK_LEFTRIGHT), str, 0, drawcharglyph, g);

		autoflush(g);
		MUTEX_EXIT(g);
	}

	void gdispGFillStringBox(GDisplay *g, gCoord x, gCoord y, gCoord cx, gCoord cy, const char* str, font_t font, gColor color, gColor bgcolor, justify_t justify) {
		gCoord		totalHeight;

		if (!font)
			return;
		MUTEX_ENTER(g);

		SETWIN(g, x, y, x+cx-1, y+cy-1);

		TEST_CLIP_AREA(g) {

			// background fill
			g->p.color = bgcolor;
			fillarea(g);

			// Apply padding
			#if GDISP_NEED_TEXT_BOXPADLR != 0 || GDISP_NEED_TEXT_BOXPADTB != 0
				if (!(justify & justifyNoPad)) {
					#if GDISP_NEED_TEXT_BOXPADLR != 0
						x += GDISP_NEED_TEXT_BOXPADLR;
						cx -= 2*GDISP_NEED_TEXT_BOXPADLR;
					#endif
					#if GDISP_NEED_TEXT_BOXPADTB != 0
						y += GDISP_NEED_TEXT_BOXPADTB;
						cy -= 2*GDISP_NEED_TEXT_BOXPADTB;
					#endif
				}
			#endif
			
			// Save the clipping area
			g->t.clip.p1.x = x;
			g->t.clip.p1.y = y;
			g->t.clip.p2.x = x+cx;
			g->t.clip.p2.y = y+cy;

			// Calculate the total text height
			#if GDISP_NEED_TEXT_WORDWRAP
				if (!(justify & justifyNoWordWrap)) {
					// Count the number of lines
					totalHeight = 0;
					mf_wordwrap(font, cx, str, mf_countline_callback, &totalHeight);
					totalHeight *= font->height;
				} else
			#endif
			totalHeight = font->height;
	
			// Select the anchor position
			switch((justify & JUSTIFYMASK_TOPBOTTOM)) {
			case justifyTop:
				break;
			case justifyBottom:
				y += cy - totalHeight;
				break;
			default:	// justifyMiddle
				y += (cy+1 - totalHeight)/2;
				break;
			}
			switch((justify & JUSTIFYMASK_LEFTRIGHT)) {
			case justifyCenter:
				x += (cx + 1) / 2;
				break;
			case justifyRight:
				x += cx;
				break;
			default:	// justifyLeft
				break;
			}

			/* Render */
			g->t.font = font;
			g->t.color = color;
			g->t.bgcolor = bgcolor;
			#if GDISP_NEED_TEXT_WORDWRAP
				if (!(justify & justifyNoWordWrap)) {
					g->t.lrj = (justify & JUSTIFYMASK_LEFTRIGHT);
					g->t.wrap.x = x;
					g->t.wrap.y = y;
					
					mf_wordwrap(font, cx, str, mf_fillline_callback, g);
				} else
			#endif
			mf_render_aligned(font, x, y, (justify & JUSTIFYMASK_LEFTRIGHT), str, 0, fillcharglyph, g);
		}

		autoflush(g);
		MUTEX_EXIT(g);
	}

	gCoord gdispGetFontMetric(font_t font, fontmetric_t metric) {
		if (!font)
			return 0;
		/* No mutex required as we only read static data */
		switch(metric) {
		case fontHeight:			return font->height;
		case fontDescendersHeight:	return font->height - font->baseline_y;
		case fontLineSpacing:		return font->line_height;
		case fontCharPadding:		return 0;
		case fontMinWidth:			return font->min_x_advance;
		case fontMaxWidth:			return font->max_x_advance;
		case fontBaselineX:			return font->baseline_x;
		case fontBaselineY:			return font->baseline_y;
		}
		return 0;
	}

	gCoord gdispGetCharWidth(char c, font_t font) {
		if (!font)
			return 0;
		/* No mutex required as we only read static data */
		return mf_character_width(font, c);
	}

	gCoord gdispGetStringWidthCount(const char* str, font_t font, uint16_t count) {
		if (!str || !font)
			return 0;

		// No mutex required as we only read static data
		#if GDISP_NEED_TEXT_KERNING
			return mf_get_string_width(font, str, count, GTrue);
		#else
			return mf_get_string_width(font, str, count, GFalse);
		#endif
	}

	gCoord gdispGetStringWidth(const char* str, font_t font) {
		return gdispGetStringWidthCount(str, font, 0);
	}
#endif

gColor gdispBlendColor(gColor fg, gColor bg, uint8_t alpha)
{
	uint16_t fg_ratio = alpha + 1;
	uint16_t bg_ratio = 256 - alpha;
	uint16_t r, g, b;

	r = RED_OF(fg) * fg_ratio;
	g = GREEN_OF(fg) * fg_ratio;
	b = BLUE_OF(fg) * fg_ratio;

	r += RED_OF(bg) * bg_ratio;
	g += GREEN_OF(bg) * bg_ratio;
	b += BLUE_OF(bg) * bg_ratio;

	r >>= 8;
	g >>= 8;
	b >>= 8;

	return RGB2COLOR(r, g, b);
}

gColor gdispContrastColor(gColor color) {
	uint16_t r, g, b;

	r = RED_OF(color) > 128 ? 0 : 255;
	g = GREEN_OF(color) > 128 ? 0 : 255;
	b = BLUE_OF(color) > 128 ? 0 : 255;

	return RGB2COLOR(r, g, b);
}

#if (!defined(gdispPackPixels) && !defined(GDISP_PIXELFORMAT_CUSTOM))
	void gdispPackPixels(gPixel *buf, gCoord cx, gCoord x, gCoord y, gColor color) {
		/* No mutex required as we only read static data */
		#if defined(GDISP_PIXELFORMAT_RGB888)
			#error "GDISP: Packed pixels not supported yet"
		#elif defined(GDISP_PIXELFORMAT_RGB444)
			#error "GDISP: Packed pixels not supported yet"
		#elif defined(GDISP_PIXELFORMAT_RGB666)
			#error "GDISP: Packed pixels not supported yet"
		#elif
			#error "GDISP: Unsupported packed pixel format"
		#endif
	}
#endif

#endif /* GFX_USE_GDISP */
