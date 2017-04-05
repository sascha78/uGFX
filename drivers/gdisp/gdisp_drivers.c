/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/gdisp_drivers.c
 * @brief   GDISP drivers.
 */

#if GFX_USE_GDISP

#include "gdisp_drivers_private.h"

// Build each of the drivers
#define GDISP_DRIVER_PROCESS	"gdisp_driver_build.h"
#include "gdisp_drivers_list.h"

// Build the array of GDISP VMT's
static const GDISPVMT * GDISP_DRIVER_LIST[] = {
	#undef GDISP_DRIVER_NAME
	#define GDISP_DRIVER_PROCESS	"gdisp_driver_add.h"
	#include "gdisp_drivers_list.h"
	
	// One final NULL to take care of the comma left by the last driver
	//	Only needed because some compilers don't like a trailing comma in an array definition
	0
};

#endif	// GFX_USE_GDISP