/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gdisp/gdisp_drivers_detectvmt.h
 * @brief   GDISP Detect which driver VMT routines need to be defined for a particular driver.
 */

/*
 * Detect which driver VMT routines need to be defined.
 * 
 * The driver's config include file path is in the macro GDISP_DRIVER_INCLUDE
 */

// Detect if we are now in a multiple driver situation
#ifdef GDISP_DRIVER_NAME
	#undef GDISP_DRIVER_NAME
	#undef GDISP_MULTIPLE_DRIVERS
	#define GDISP_MULTIPLE_DRIVERS			GFXON
#endif

// Include the required driver config file
#include GDISP_DRIVER_INCLUDE
#include "gdisp_drivers_defs.h"

// Make sure all the VMT capabilities are defined
#ifndef GDISP_HARDWARE_VMT_DEINIT
	#define GDISP_HARDWARE_VMT_DEINIT		GFXOFF
#endif
#ifndef GDISP_HARDWARE_VMT_FLUSH
	#define GDISP_HARDWARE_VMT_FLUSH		GFXOFF
#endif
#ifndef GDISP_HARDWARE_VMT_READ
	#define GDISP_HARDWARE_VMT_READ			GFXOFF
#endif
#ifndef GDISP_HARDWARE_VMT_SETPOS
	#define GDISP_HARDWARE_VMT_DEINIT		GFXOFF
#endif
#ifndef GDISP_HARDWARE_VMT_MOVE
	#define GDISP_HARDWARE_VMT_MOVE			GFXOFF
#endif
#ifndef GDISP_HARDWARE_VMT_IOCTL
	#define GDISP_HARDWARE_VMT_IOCTL		GFXOFF
#endif

// Include all the drivers valid values into the VMT
#if GDISP_HARDWARE_DEINIT
	#undef GDISP_HARDWARE_VMT_DEINIT
	#define GDISP_HARDWARE_VMT_DEINIT		GFXON
#endif

#if GDISP_HARDWARE_FLUSH
	#undef GDISP_HARDWARE_VMT_FLUSH
	#define GDISP_HARDWARE_VMT_FLUSH		GFXON
#endif
#if GDISP_HARDWARE_READ
	#undef GDISP_HARDWARE_VMT_READ
	#define GDISP_HARDWARE_VMT_READ			GFXON
#endif
#if GDISP_HARDWARE_SETPOS
	#undef GDISP_HARDWARE_VMT_SETPOS
	#define GDISP_HARDWARE_VMT_SETPOS		GFXON
#endif
#if GDISP_HARDWARE_MOVE
	#undef GDISP_HARDWARE_VMT_MOVE
	#define GDISP_HARDWARE_VMT_MOVE			GFXON
#endif
#if GDISP_HARDWARE_IOCTL
	#undef GDISP_HARDWARE_VMT_IOCTL
	#define GDISP_HARDWARE_VMT_IOCTL		GFXON
#endif

// Cleanup all the macros
#include "gdisp_drivers_defs.h"
#if GDISP_MULTIPLE_DRIVERS
	#undef GDISP_DRIVER_NAME
#endif
#undef GDISP_DRIVER_INCLUDE
