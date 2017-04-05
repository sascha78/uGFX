/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/gdisp_driver_vmtdetect.h
 * @brief   GDISP driver - Detect which driver VMT routines need to be defined for a particular driver.
 */

// Load the definitions for the current driver
#include "gdisp_driver_options.h"

// Make sure all the VMT capabilities are defined
#ifndef GDISP_DRIVER_VMT_DEINIT
	#define GDISP_DRIVER_VMT_DEINIT			GFXOFF
#endif
#ifndef GDISP_DRIVER_VMT_FLUSH
	#define GDISP_DRIVER_VMT_FLUSH			GFXOFF
#endif
#ifndef GDISP_DRIVER_VMT_READ
	#define GDISP_DRIVER_VMT_READ			GFXOFF
#endif
#ifndef GDISP_DRIVER_VMT_SETPOS
	#define GDISP_DRIVER_VMT_DEINIT			GFXOFF
#endif
#ifndef GDISP_DRIVER_VMT_MOVE
	#define GDISP_DRIVER_VMT_MOVE			GFXOFF
#endif
#ifndef GDISP_DRIVER_VMT_IOCTL
	#define GDISP_DRIVER_VMT_IOCTL			GFXOFF
#endif

// Include all the drivers valid values into the VMT
#if GDISP_DRIVER_DEINIT
	#if !GDISP_MULTIPLE_DRIVERS
		#undef GDISP_DRIVER_VMT_DEINIT
		#define GDISP_DRIVER_VMT_DEINIT		GFXON
	#elif !GDISP_DRIVER_VMT_DEINIT
		#undef GDISP_DRIVER_VMT_DEINIT
		#define GDISP_DRIVER_VMT_DEINIT		GFXSOME
	#endif
#elif GDISP_DRIVER_VMT_DEINIT == GFXON
	#undef GDISP_DRIVER_VMT_DEINIT
	#define GDISP_DRIVER_VMT_DEINIT			GFXSOME
#endif

#if GDISP_DRIVER_FLUSH
	#if !GDISP_MULTIPLE_DRIVERS
		#undef GDISP_DRIVER_VMT_FLUSH
		#define GDISP_DRIVER_VMT_FLUSH		GFXON
	#elif !GDISP_DRIVER_VMT_FLUSH
		#undef GDISP_DRIVER_VMT_FLUSH
		#define GDISP_DRIVER_VMT_FLUSH		GFXSOME
	#endif
#elif GDISP_DRIVER_VMT_FLUSH == GFXON
	#undef GDISP_DRIVER_VMT_FLUSH
	#define GDISP_DRIVER_VMT_FLUSH			GFXSOME
#endif

#if GDISP_DRIVER_READ
	#if !GDISP_MULTIPLE_DRIVERS
		#undef GDISP_DRIVER_VMT_READ
		#define GDISP_DRIVER_VMT_READ		GFXON
	#elif !GDISP_DRIVER_VMT_READ
		#undef GDISP_DRIVER_VMT_READ
		#define GDISP_DRIVER_VMT_READ		GFXSOME
	#endif
#elif GDISP_DRIVER_VMT_READ == GFXON
	#undef GDISP_DRIVER_VMT_READ
	#define GDISP_DRIVER_VMT_READ			GFXSOME
#endif

#if GDISP_DRIVER_SETPOS
	#if !GDISP_MULTIPLE_DRIVERS
		#undef GDISP_DRIVER_VMT_SETPOS
		#define GDISP_DRIVER_VMT_SETPOS		GFXON
	#elif !GDISP_DRIVER_VMT_SETPOS
		#undef GDISP_DRIVER_VMT_SETPOS
		#define GDISP_DRIVER_VMT_SETPOS		GFXSOME
	#endif
#elif GDISP_DRIVER_VMT_SETPOS == GFXON
	#undef GDISP_DRIVER_VMT_SETPOS
	#define GDISP_DRIVER_VMT_SETPOS			GFXSOME
#endif

#if GDISP_DRIVER_MOVE
	#if !GDISP_MULTIPLE_DRIVERS
		#undef GDISP_DRIVER_VMT_MOVE
		#define GDISP_DRIVER_VMT_MOVE		GFXON
	#elif !GDISP_DRIVER_VMT_MOVE
		#undef GDISP_DRIVER_VMT_MOVE
		#define GDISP_DRIVER_VMT_MOVE		GFXSOME
	#endif
#elif GDISP_DRIVER_VMT_MOVE == GFXON
	#undef GDISP_DRIVER_VMT_MOVE
	#define GDISP_DRIVER_VMT_MOVE			GFXSOME
#endif

#if GDISP_DRIVER_IOCTL
	#if !GDISP_MULTIPLE_DRIVERS
		#undef GDISP_DRIVER_VMT_IOCTL
		#define GDISP_DRIVER_VMT_IOCTL		GFXON
	#elif !GDISP_DRIVER_VMT_IOCTL
		#undef GDISP_DRIVER_VMT_IOCTL
		#define GDISP_DRIVER_VMT_IOCTL		GFXSOME
	#endif
#elif GDISP_DRIVER_VMT_IOCTL == GFXON
	#undef GDISP_DRIVER_VMT_IOCTL
	#define GDISP_DRIVER_VMT_IOCTL			GFXSOME
#endif

// Cleanup all the macros
#include "gdisp_driver_options.h"
