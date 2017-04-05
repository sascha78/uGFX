/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/gdisp_driver_count.h
 * @brief   GDISP: Detect if there are multiple drivers.
 */

// Detect if we are now in a multiple driver situation by the precense of a previous driver name
#ifndef GDISP_MULTIPLE_DRIVERS
	#define GDISP_MULTIPLE_DRIVERS			GFXOFF
#elif !GDISP_MULTIPLE_DRIVERS
	#undef GDISP_MULTIPLE_DRIVERS
	#define GDISP_MULTIPLE_DRIVERS			GFXON
#endif

// Load the definitions for the current driver
#include "gdisp_driver_options.h"

// Cleanup all the macros
#include "gdisp_driver_options.h"
