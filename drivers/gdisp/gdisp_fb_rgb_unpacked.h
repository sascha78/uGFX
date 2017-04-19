/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GDISP_FB_RGB_UNPACKED_H
#define _GDISP_FB_RGB_UNPACKED_H

//--------------------------------------------------------------------------
// Framebuffer Options
//--------------------------------------------------------------------------

/**
 * Setting this to GFXON forces a flush after each pixel burst change.
 * This significantly slows down the screen display.
 * The only time you might want to turn this on is
 * if you are debugging drawing and want to see each
 * pixel as it is set.
 */
#ifndef GDISP_FRAMEBUFFER_FORCEREDRAW
	#define GDISP_FRAMEBUFFER_FORCEREDRAW	GFXOFF
#endif

/**
 * Setting this to GFXON creates a "ua" gRect in the device private area
 *	which tracks the pixel coordinates that require flushing. This is
 *	useful for drivers that have slow flushing but are capable
 *	of flushing a partial display area.
 *	If not set then a flag is used to indicate if any flushing is required
 *	and the whole framebuffer must be flushed. This is however computationally
 *	cheaper.
 */
#ifndef GDISP_FRAMEBUFFER_REGIONFLUSH
	#define GDISP_FRAMEBUFFER_REGIONFLUSH	GFXOFF
#endif

//--------------------------------------------------------------------------
// Define our driver private area
//--------------------------------------------------------------------------

typedef struct GDISPDRIVERID(DriverPrivateArea) {
	uint8_t			*fb;				// The framebuffer
	unsigned		linebytes;			// The line width in bytes
	#if GDISP_DRIVER_FLUSH && GDISP_FRAMEBUFFER_REGIONFLUSH
		gRect		ua;					// The update area
	#endif
} GDISPDRIVERID(DriverPrivateArea);

typedef struct GDISPDRIVERID(Driver) {
	GDisplay							g;
	GDISPDRIVERID(DriverPrivateArea)	dpa;
	GDISPDRIVERID(BoardPrivateArea)		bpa;
} GDISPDRIVERID(Driver);

#define myg		((GDISPDRIVERID(Driver) *)g)

#if GDISP_DRIVER_FLUSH && GDISP_FRAMEBUFFER_REGIONFLUSH
	#define GDISP_FRAMEBUFFER_INIT(fbp, linesz)	{ 		\
		myg->dpa.fb = (fbp);								\
		myg->dpa.linebytes = (linesz);					\
		myg->dpa.ua.p1.x = myg->dpa.ua.p1.y = GCOORD_MAX;	\
		myg->dpa.ua.p2.x = myg->dpa.ua.p2.y = GCOORD_MIN;	\
	}
#else
	#define GDISP_FRAMEBUFFER_INIT(fb, linebytes)	{		\
		myg->dpa.fb = (fb);									\
		myg->dpa.linebytes = (linebytes);					\
	}
#endif

#if GDISP_DRIVER_FLUSH && GDISP_FRAMEBUFFER_REGIONFLUSH
	#define GDISP_FRAMEBUFFER_FLUSHTEST()	if (myg->dpa.ua.p1.x > myg->dpa.ua.p2.x) return
	#define GDISP_FRAMEBUFFER_FLUSHRESET()	{				\
		myg->dpa.ua.p1.x = myg->dpa.ua.p1.y = GCOORD_MAX;	\
		myg->dpa.ua.p2.x = myg->dpa.ua.p2.y = GCOORD_MIN;	\
	}
#elif GDISP_DRIVER_FLUSH && !GDISP_FRAMEBUFFER_REGIONFLUSH
	#define GDISP_FRAMEBUFFER_FLUSHTEST()	if (!(g->flags & GDISP_FLG_FLUSHREQ)) return
	#define GDISP_FRAMEBUFFER_FLUSHRESET()	g->flags &= ~GDISP_FLG_FLUSHREQ
#endif

static	void GDISPDRIVERID(start)(GDisplay *g) {
	#if GDISP_DRIVER_SETPOS
		(void) g;
	#else
		g->win.p.x = g->win.r.p1.x;
		g->win.p.y = g->win.r.p1.y;
	#endif
}

static	void GDISPDRIVERID(write)(GDisplay *g) {
	int			bd;
	gCoord		x, y, cnt;
	COLORREF	color;
	uint8_t		*p;

	color = gdispColor2Native(g->p.color);
	cnt = g->p.e.cnt-1;
	
	#if GDISP_DRIVER_IOCTL
		switch(g->g.Orientation) {
		case GDISP_ROTATE_0:
		default:
			x = g->win.p.x;
			y = g->win.p.y;
			bd = sizeof(GDISP_DRIVER_COLOR_TYPE);
			#if GDISP_DRIVER_FLUSH && GDISP_FRAMEBUFFER_REGIONFLUSH
				if (x < myg->dpa.ua.p1.x) myg->dpa.ua.p1.x = x;
				if (y < myg->dpa.ua.p1.y) myg->dpa.ua.p1.y = y;
				if (x+cnt > myg->dpa.ua.p2.x) myg->dpa.ua.p2.x = x+cnt;
				if (y > myg->dpa.ua.p2.y) myg->dpa.ua.p2.y = y;
			#endif
			break;
		case GDISP_ROTATE_90:
			x = g->win.p.y;
			y = g->g.Width - 1 - g->win.p.x;
			bd = -myg->dpa.linebytes;
			#if GDISP_DRIVER_FLUSH && GDISP_FRAMEBUFFER_REGIONFLUSH
				if (x < myg->dpa.ua.p1.x) myg->dpa.ua.p1.x = x;
				if (y-cnt < myg->dpa.ua.p1.y) myg->dpa.ua.p1.y = y-cnt;
				if (x > myg->dpa.ua.p2.x) myg->dpa.ua.p2.x = x;
				if (y > myg->dpa.ua.p2.y) myg->dpa.ua.p2.y = y;
			#endif
			break;
		case GDISP_ROTATE_180:
			x = g->g.Width - 1 - g->win.p.x;
			y = g->g.Height - 1 - g->win.p.y;
			bd = -sizeof(GDISP_DRIVER_COLOR_TYPE);
			#if GDISP_DRIVER_FLUSH && GDISP_FRAMEBUFFER_REGIONFLUSH
				if (x-cnt < myg->dpa.ua.p1.x) myg->dpa.ua.p1.x = x-cnt;
				if (y < myg->dpa.ua.p1.y) myg->dpa.ua.p1.y = y;
				if (x > myg->dpa.ua.p2.x) myg->dpa.ua.p2.x = x;
				if (y > myg->dpa.ua.p2.y) myg->dpa.ua.p2.y = y;
			#endif
			break;
		case GDISP_ROTATE_270:
			x = g->g.Height - 1 - g->win.p.y;
			y = g->win.p.x;
			bd = myg->dpa.linebytes;
			#if GDISP_DRIVER_FLUSH && GDISP_FRAMEBUFFER_REGIONFLUSH
				if (x < myg->dpa.ua.p1.x) myg->dpa.ua.p1.x = x;
				if (y+cnt < myg->dpa.ua.p1.y) myg->dpa.ua.p1.y = y+cnt;
				if (x > myg->dpa.ua.p2.x) myg->dpa.ua.p2.x = x;
				if (y > myg->dpa.ua.p2.y) myg->dpa.ua.p2.y = y;
			#endif
			break;
		}
	#else
		x = g->win.p.x;
		y = g->win.p.y;
		bd = sizeof(GDISP_DRIVER_COLOR_TYPE);
		#if GDISP_DRIVER_FLUSH && GDISP_FRAMEBUFFER_REGIONFLUSH
			if (x < myg->dpa.ua.p1.x) myg->dpa.ua.p1.x = x;
			if (y < myg->dpa.ua.p1.y) myg->dpa.ua.p1.y = y;
			if (x+cnt > myg->dpa.ua.p2.x) myg->dpa.ua.p2.x = x+cnt;
			if (y > myg->dpa.ua.p2.y) myg->dpa.ua.p2.y = y;
		#endif
	#endif

	p = myg->dpa.fb + y*myg->dpa.linebytes+x*sizeof(GDISP_DRIVER_COLOR_TYPE);

	for (; cnt >= 0; cnt--, p += bd)
		*((GDISP_DRIVER_COLOR_TYPE *)p) = color;

	
	// Update the cursor
	g->win.p.x += g->p.e.cnt;
	if (g->win.p.x > g->win.r.p2.x) {
		g->win.p.x = g->win.r.p1.x;
		if (++g->win.p.y > g->win.r.p2.y)
			g->win.p.y = g->win.r.p1.y;
	}

	#if GDISP_DRIVER_FLUSH
		#if !GDISP_FRAMEBUFFER_REGIONFLUSH
			g->flags |= GDISP_FLG_FLUSHREQ;
		#endif
		#if GDISP_WIN32_FORCEREDRAW
			GDISPDRIVERID(flush)(g);
		#endif
	#endif
}

#if GDISP_DRIVER_SETPOS
	static void GDISPDRIVERID(setpos)(GDisplay *g) {
		(void) g;
	}
#endif

#if GDISP_DRIVER_READ
	static	gColor GDISPDRIVERID(read)(GDisplay *g) {
		gCoord		x, y;
		GDISP_DRIVER_COLOR_TYPE	color;

		#if GDISP_DRIVER_IOCTL
			switch(g->g.Orientation) {
			case GDISP_ROTATE_0:
			default:
				x = g->win.p.x;
				y = g->win.p.y;
				break;
			case GDISP_ROTATE_90:
				x = g->win.p.y;
				y = g->g.Width - 1 - g->win.p.x;
				break;
			case GDISP_ROTATE_180:
				x = g->g.Width - 1 - g->win.p.x;
				y = g->g.Height - 1 - g->win.p.y;
				break;
			case GDISP_ROTATE_270:
				x = g->g.Height - 1 - g->win.p.y;
				y = g->win.p.x;
				break;
			}
		#else
			x = g->win.p.x;
			y = g->win.p.y;
		#endif

		color = *((GDISP_DRIVER_COLOR_TYPE *)(myg->dpa.fb + y*myg->dpa.linebytes+x*sizeof(GDISP_DRIVER_COLOR_TYPE)));

		// Update the cursor
		if (++g->win.p.x > g->win.r.p2.x) {
			g->win.p.x = g->win.r.p1.x;
			if (++g->win.p.y > g->win.r.p2.y)
				g->win.p.y = g->win.r.p1.y;
		}

		return gdispNative2Color(color);
	}
#endif

#if GDISP_DRIVER_MOVE
	#include <string.h>
	
	static void GDISPDRIVERID(move)(GDisplay *g) {		// Uses p.win	p.e.pos2 (=new pos)
		gCoord		w, h;
		uint8_t		*s;
		uint8_t		*d;
		
		#if GDISP_DRIVER_IOCTL
			//THIS SECTION TODO
			switch(g->g.Orientation) {
			case GDISP_ROTATE_0:
			default:
				x = g->win.p.x;
				y = g->win.p.y;
				bd = sizeof(GDISP_DRIVER_COLOR_TYPE);
				#if GDISP_DRIVER_FLUSH && GDISP_FRAMEBUFFER_REGIONFLUSH
					if (x < myg->dpa.ua.p1.x) myg->dpa.ua.p1.x = x;
					if (y < myg->dpa.ua.p1.y) myg->dpa.ua.p1.y = y;
					if (x+cnt > myg->dpa.ua.p2.x) myg->dpa.ua.p2.x = x+cnt;
					if (y > myg->dpa.ua.p2.y) myg->dpa.ua.p2.y = y;
				#endif
				break;
			case GDISP_ROTATE_90:
				x = g->win.p.y;
				y = g->g.Width - 1 - g->win.p.x;
				bd = -myg->dpa.linebytes;
				#if GDISP_DRIVER_FLUSH && GDISP_FRAMEBUFFER_REGIONFLUSH
					if (x < myg->dpa.ua.p1.x) myg->dpa.ua.p1.x = x;
					if (y-cnt < myg->dpa.ua.p1.y) myg->dpa.ua.p1.y = y-cnt;
					if (x > myg->dpa.ua.p2.x) myg->dpa.ua.p2.x = x;
					if (y > myg->dpa.ua.p2.y) myg->dpa.ua.p2.y = y;
				#endif
				break;
			case GDISP_ROTATE_180:
				x = g->g.Width - 1 - g->win.p.x;
				y = g->g.Height - 1 - g->win.p.y;
				bd = -sizeof(GDISP_DRIVER_COLOR_TYPE);
				#if GDISP_DRIVER_FLUSH && GDISP_FRAMEBUFFER_REGIONFLUSH
					if (x-cnt < myg->dpa.ua.p1.x) myg->dpa.ua.p1.x = x-cnt;
					if (y < myg->dpa.ua.p1.y) myg->dpa.ua.p1.y = y;
					if (x > myg->dpa.ua.p2.x) myg->dpa.ua.p2.x = x;
					if (y > myg->dpa.ua.p2.y) myg->dpa.ua.p2.y = y;
				#endif
				break;
			case GDISP_ROTATE_270:
				x = g->g.Height - 1 - g->win.p.y;
				y = g->win.p.x;
				bd = myg->dpa.linebytes;
				#if GDISP_DRIVER_FLUSH && GDISP_FRAMEBUFFER_REGIONFLUSH
					if (x < myg->dpa.ua.p1.x) myg->dpa.ua.p1.x = x;
					if (y+cnt < myg->dpa.ua.p1.y) myg->dpa.ua.p1.y = y+cnt;
					if (x > myg->dpa.ua.p2.x) myg->dpa.ua.p2.x = x;
					if (y > myg->dpa.ua.p2.y) myg->dpa.ua.p2.y = y;
				#endif
				break;
			}
		#else
			s = myg->dpa.fb + g->win.r.p1.y*myg->dpa.linebytes + g->win.r.p1.x*sizeof(GDISP_DRIVER_COLOR_TYPE); 
			d = myg->dpa.fb + g->p.e.pos2.y*myg->dpa.linebytes + g->p.e.pos2.x*sizeof(GDISP_DRIVER_COLOR_TYPE);
			w = g->win.r.p2.x-g->win.r.p1.x+1;
			h = g->win.r.p2.y-g->win.r.p1.y+1;
			#if GDISP_DRIVER_FLUSH && GDISP_FRAMEBUFFER_REGIONFLUSH
				if (g->p.e.pos2.x < myg->dpa.ua.p1.x) myg->dpa.ua.p1.x = g->p.e.pos2.x;
				if (g->p.e.pos2.y < myg->dpa.ua.p1.y) myg->dpa.ua.p1.y = g->p.e.pos2.y;
				if (g->p.e.pos2.x+w > myg->dpa.ua.p2.x) myg->dpa.ua.p2.x = g->p.e.pos2.x+w;
				if (g->p.e.pos2.y+h > myg->dpa.ua.p2.y) myg->dpa.ua.p2.y = g->p.e.pos2.y+h;
			#endif
			w *= sizeof(GDISP_DRIVER_COLOR_TYPE);
		#endif

		// Is the destination point inside the src rectangle?
		if (p.e.pos2.x >= g->win.r.p1.x && p.e.pos2.x <= g->win.r.p2.x && p.e.pos2.y >= g->win.r.p1.y && p.e.pos2.y <= g->win.r.p2.y) {
			// We need to do this backwards
			s += myg->dpa.linebytes*(h-1);
			d += myg->dpa.linebytes*(h-1);
			for (;h--; s-=myg->dpa.linebytes, d-=myg->dpa.linebytes)
				memmove(d, s, w);
		} else {
			for (;h--; s+=myg->dpa.linebytes, d+=myg->dpa.linebytes)
				memmove(d, s, w);
		}
	}
#endif

/* TODO
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
		case GDISP_CONTROL_POWER:
		case GDISP_CONTROL_BACKLIGHT:
		case GDISP_CONTROL_CONTRAST:
		}
	}
#endif
*/

#else	// _GDISP_FB_RGB_UNPACKED_H

	// Cleanup all definitions
	#undef _GDISP_FB_RGB_UNPACKED_H
	#undef GDISP_FRAMEBUFFER_FORCEREDRAW
	#undef GDISP_FRAMEBUFFER_REGIONFLUSH
	#undef GDISP_FRAMEBUFFER_INIT
	#undef GDISP_FRAMEBUFFER_FLUSHTEST
	#undef GDISP_FRAMEBUFFER_FLUSHRESET
	#undef myg
	
#endif	// _GDISP_FB_RGB_UNPACKED_H
