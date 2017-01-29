/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gdisp/gdisp_drivers.c
 * @brief   GDISP sub-system drivers.
 */

#include "gfx.h"

#if GFX_USE_GDISP

#include "gdisp_drivers.h"

#define GDISP_VMT_CHAIN	0

#if GDISP_MULTIPLE_DRIVERS
	#define GDISPLLD static
#else
	#define GDISPLLD
#endif

// Include each of the drivers
#if  GDISP_DRIVER_WIN32
	#include "../multiple/Win32/Win32Driver.c"
	#include "gdisp_drivers_buildvmt.h"
	#define GDISP_VMT_CHAIN		&Win32_VMTCHAIN
#endif
//... Other drivers here

// The chain of GDISP VMT's
GDriverVMTList const * _GDISP_VMT_CHAIN = GDISP_VMT_CHAIN;

#endif	// GFX_USE_GDISP