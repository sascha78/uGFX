/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/gdisp_driver_build.h
 * @brief   GDISP drivers - build the driver itself.
 */

// Get the driver hardware capabilities
#include "gdisp_driver_options.h"

// Define the color macros for the specified driver pixel format
#include "gdisp_driver_colors.h"

// These are actually defined in the driver source file or the board file.
//	The defaults from the config options need to be ignored.
#undef GDISP_DRIVER_BYTESDRIVER
#undef GDISP_DRIVER_BYTESBOARD

// Include the driver source code
#include GDISP_DRIVER_C

// Re-establish defaults
#ifndef GDISP_DRIVER_BYTESDRIVER
	#define GDISP_DRIVER_BYTESDRIVER	0
#endif
#ifndef GDISP_DRIVER_BYTESBOARD
	#define GDISP_DRIVER_BYTESBOARD		0
#endif

// Define the driver VMT structure
static const GDISPVMT GDISPDRIVERID(VMT) = {
	{													// GDriverVMT....
		GDRIVER_TYPE_DISPLAY,								// type
		#if GDISP_DRIVER_DYNAMIC							// flags
			GDISP_VFLG_DYNAMIC,
		#else
			0,
		#endif
		sizeof(GDisplay) + GDISP_DRIVER_BYTESDRIVER + GDISP_DRIVER_BYTESBOARD,		// objsize
		_gdispInitDriver,									// init()
		_gdispPostInitDriver,								// postinit()
		_gdispDeInitDriver									// deinit()
	}
	, GDISPDRIVERID(count)
	, GDISPDRIVERID(init)
	, GDISPDRIVERID(start)
	, GDISPDRIVERID(write)
	#if GDISP_DRIVER_VMT_DEINIT
		#if GDISP_DRIVER_DEINIT
			, GDISPDRIVERID(deinit)
		#else
			, 0
		#endif
	#endif
	#if GDISP_DRIVER_VMT_FLUSH
		#if GDISP_DRIVER_FLUSH
			, GDISPDRIVERID(flush)
		#else
			, 0
		#endif
	#endif
	#if GDISP_DRIVER_VMT_SETPOS
		#if GDISP_DRIVER_SETPOS
			, GDISPDRIVERID(setpos)
		#else
			, 0
		#endif
	#endif
	#if GDISP_DRIVER_VMT_READ
		#if GDISP_DRIVER_READ
			, GDISPDRIVERID(read)
		#else
			, 0
		#endif
	#endif
	#if GDISP_DRIVER_VMT_MOVE
		#if GDISP_DRIVER_MOVE
			, GDISPDRIVERID(move)
		#else
			, 0
		#endif
	#endif
	#if GDISP_DRIVER_VMT_IOCTL
		#if GDISP_DRIVER_IOCTL
			, GDISPDRIVERID(ioctl)
		#else
			, 0
		#endif
	#endif
};

// Cleanup
#include "gdisp_driver_colors.h"
#include "gdisp_driver_options.h"
