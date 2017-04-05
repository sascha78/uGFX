/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/gdisp_drivers_list.c
 * @brief   GDISP drivers list.
 */

#if  GDISP_DRIVER_WIN32
	#define GDISP_DRIVER_H "../multiple/Win32/Win32Config_gdisp.h"
	#define GDISP_DRIVER_C "../multiple/Win32/Win32Driver.c"
	#include GDISP_DRIVER_PROCESS
#endif

// Other drivers here

#undef GDISP_DRIVER_PROCESS
