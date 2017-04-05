/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/gdisp_drivers.h
 * @brief   GDISP drivers - detect if there is a single driver and set pixel format for the system
 */
#ifndef _GDISP_DRIVERS_H
#define _GDISP_DRIVERS_H

// Some safety things
#undef GDISP_MULTIPLE_DRIVERS
#undef GDISP_DRIVER_NAME
#undef GDISP_DRIVER_PIXELFORMAT
#define GDISP_DRIVER_EXTRA_API

// Detect if there are multiple drivers configured
#define GDISP_DRIVER_PROCESS	"gdisp_driver_count.h"
#include "gdisp_drivers_list.h"

#undef GDISP_DRIVER_EXTRA_API

// Safety check
#ifndef GDISP_MULTIPLE_DRIVERS
	#error "GDISP: No drivers defined"
#endif

#endif	// _GDISP_DRIVERS_H
