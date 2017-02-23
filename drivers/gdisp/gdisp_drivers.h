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
#define GFXSOME			(-2)

// Work out what hardware support is available accross all drivers
#if  GDISP_DRIVER_WIN32
	#define GDISP_DRIVER_INCLUDE "../multiple/Win32/Win32Config_gdisp.h"
	#include "gdisp_drivers_detectvmt.h"
#endif
//... Other drivers here

#if !defined(GDISP_DRIVER_NAME) && !GDISP_MULTIPLE_DRIVERS
	#error "GDISP: No drivers defined"
#endif

#if GFX_IN_IMPLEMENTATION

	// Create the WIN32 VMT structure definition
	typedef struct GDISPVMT {
		GDriverVMT	d;
			#define GDISP_VFLG_DYNAMIC		0x0001		// GDrawVMT flags: This display should never be statically initialised
		bool_t	(*init)			(GDisplay *g);
		void	(*start)		(GDisplay *g);			// Uses p.win
		void	(*write)		(GDisplay *g);			// Uses p.pos, p.color, p.e.cnt
		#if GDISP_HARDWARE_VMT_DEINIT
			void	(*deinit)	(GDisplay *g);			// Uses no parameters
		#endif
		#if GDISP_HARDWARE_VMT_FLUSH
			void	(*flush)	(GDisplay *g);			// Uses no parameters
		#endif
		#if GDISP_HARDWARE_VMT_SETPOS
			void	(*setpos)	(GDisplay *g);			// Uses p.pos
		#endif
		#if GDISP_HARDWARE_VMT_READ
			gColor	(*read)		(GDisplay *g);			// Uses p.pos
		#endif
		#if GDISP_HARDWARE_VMT_MOVE
			void	(*move)		(GDisplay *g);			// Uses p.win	p.e.pos2 (=new pos)
		#endif
		#if GDISP_HARDWARE_VMT_IOCTL
			bool_t	(*ioctl)	(GDisplay *g);			// Uses p.x (=what?????)  p.e.ptr/p.e.cnt (=value)
		#endif
	} GDISPVMT;
	
	// Our gdisp driver VMT chain
	extern GDriverVMTList const * _GDISP_VMT_CHAIN;
	
	// A GDISP driver identifier
	#define GDISPDRIVERID(name)				GFXCATX(GDISP_DRIVER_NAME, name)
	
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
				gColor	GDISPDRIVERID(read)		(GDisplay *g);
			#endif
			#if GDISP_HARDWARE_VMT_MOVE
				void	GDISPDRIVERID(move)		(GDisplay *g);
			#endif
			#if GDISP_HARDWARE_VMT_IOCTL
				void	GDISPDRIVERID(ioctl)	(GDisplay *g);
			#endif
			#define gdisp_lld_init(g)		GDISPDRIVERID(init)(g)
			#define gdisp_lld_start(g)		GDISPDRIVERID(start)(g)
			#define gdisp_lld_write(g)		GDISPDRIVERID(write)(g)
			#define gdisp_lld_deinit(g)		GDISPDRIVERID(deinit)(g)
			#define gdisp_lld_flush(g)		GDISPDRIVERID(flush)(g)
			#define gdisp_lld_setpos(g)		GDISPDRIVERID(setpos)(g)
			#define gdisp_lld_read(g)		GDISPDRIVERID(read)(g)
			#define gdisp_lld_move(g)		GDISPDRIVERID(move)(g)
			#define gdisp_lld_ioctl(g)		GDISPDRIVERID(ioctl)(g)
		#else
			#define gdisp_lld_init(g)		gvmt(g)->init(g)
			#define gdisp_lld_start(g)		gvmt(g)->start(g)
			#define gdisp_lld_write(g)		gvmt(g)->write(g)
			#define gdisp_lld_deinit(g)		gvmt(g)->deinit(g)
			#define gdisp_lld_flush(g)		gvmt(g)->flush(g)
			#define gdisp_lld_setpos(g)		gvmt(g)->setpos(g)
			#define gdisp_lld_read(g)		gvmt(g)->read(g)
			#define gdisp_lld_move(g)		gvmt(g)->move(g)
			#define gdisp_lld_ioctl(g)		gvmt(g)->ioctl(g)
		#endif
	
	#ifdef __cplusplus
	}
	#endif
#endif

#endif	// _GDISP_DRIVERS_H
