/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gdisp/gdisp_drivers.h
 * @brief   GDISP sub-system drivers header file.
 */
#ifndef _GDISP_DRIVERS_H
#define _GDISP_DRIVERS_H

#undef GDISP_DRIVER_NAME
#define GDISP_MULTIPLE_DRIVERS	GFXOFF

// Work out what hardware support is available accross all drivers
#if  GDISP_DRIVER_WIN32
	#define GDISP_DRIVER_INCLUDE "../multiple/Win32/Win32Config_gdisp.h"
	#include "gdisp_drivers_detectvmt.h"
#endif
//... Other drivers here

// Create the WIN32 VMT structure definition
typedef struct GDISPVMT {
	GDriverVMT	d;
		#define GDISP_VFLG_DYNAMIC		0x0001		// GDrawVMT flags: This display should never be statically initialised
	bool_t	(*init)			(GDisplay *g);
	void	(*start)		(GDisplay *g);			// Uses p.x,p.y  p.cx,p.cy
	void	(*write)		(GDisplay *g);			// Uses p.color  p.x1 (=count)
	#if GDISP_HARDWARE_VMT_DEINIT
		void	(*deinit)	(GDisplay *g);			// Uses no parameters
	#endif
	#if GDISP_HARDWARE_VMT_FLUSH
		void	(*flush)	(GDisplay *g);			// Uses no parameters
	#endif
	#if GDISP_HARDWARE_VMT_SETPOS
		void	(*setpos)	(GDisplay *g);			// Uses p.x,p.y
	#endif
	#if GDISP_HARDWARE_VMT_READ
		color_t	(*read)		(GDisplay *g);			// Uses no parameters
	#endif
	#if GDISP_HARDWARE_VMT_MOVE
		void	(*move)		(GDisplay *g);			// Uses p.x,p.y  p.cx,p.cy	p.x1,p.y1 (=new pos)
	#endif
	#if GDISP_HARDWARE_VMT_IOCTL
		void	(*ioctl)	(GDisplay *g);			// Uses p.x (=what)  p.ptr (=value)
	#endif
} GDISPVMT;

// Our gdisp driver VMT chain
extern GDriverVMTList const * _GDISP_VMT_CHAIN;

// A GDISP driver identifier
#define GDISPDRIVERID(name)		GDISP_DRIVER_NAME ## _ ## name

#ifdef __cplusplus
extern "C" {
#endif

	// Routines needed by the general driver VMT
	bool_t _gdispInitDriver(GDriver *g, void *param, unsigned driverinstance, unsigned systeminstance);
	void _gdispPostInitDriver(GDriver *g);
	void _gdispDeInitDriver(GDriver *g);

	// Prototypes for direct calling where there is only one driver
	#ifdef GDISP_DRIVER_NAME
		bool_t	GDISPDRIVERID(init)			(GDisplay *g);
		void	GDISPDRIVERID(start)		(GDisplay *g);
		void	GDISPDRIVERID(write)		(GDisplay *g);
		#if GDISP_HARDWARE_VMT_DEINIT
			void	GDISPDRIVERID(deinit)	(GDisplay *g);
		#endif
		#if GDISP_HARDWARE_VMT_FLUSH
			void	GDISPDRIVERID(flush)	(GDisplay *g);
		#endif
		#if GDISP_HARDWARE_VMT_SETPOS
			void	GDISPDRIVERID(setpos)	(GDisplay *g);
		#endif
		#if GDISP_HARDWARE_VMT_READ
			color_t	GDISPDRIVERID(read)		(GDisplay *g);
		#endif
		#if GDISP_HARDWARE_VMT_MOVE
			void	GDISPDRIVERID(move)		(GDisplay *g);
		#endif
		#if GDISP_HARDWARE_VMT_IOCTL
			void	GDISPDRIVERID(ioctl)	(GDisplay *g);
		#endif
	#endif

#ifdef __cplusplus
}
#endif

#endif	// _GDISP_DRIVERS_H
