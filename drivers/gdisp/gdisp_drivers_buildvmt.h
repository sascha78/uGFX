/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gdisp/gdisp_drivers_buildvmt.h
 * @brief   GDISP sub-system drivers - build a vmt for a driver.
 */

// Make sure the driver config variables were fully defined
#ifndef GDISP_HARDWARE_DEFINES
	#include "gdisp_driver_defs.h"
#endif

// How much extra space does the driver need in the GDisplay structure
#ifndef GDISP_HARDWARE_EXTRADATA
	#define GDISP_HARDWARE_EXTRADATA	0
#endif

// Define the driver VMT structure
static const GDISPVMT GDISPDRIVERID(VMT) = {
	{													// GDriverVMT....
		GDRIVER_TYPE_DISPLAY,								// type
		#if GDISP_HARDWARE_DYNAMIC							// flags
			GDISP_VFLG_DYNAMIC,
		#else
			0,
		#endif
		sizeof(GDisplay) + GDISP_HARDWARE_EXTRADATA,		// objsize
		_gdispInitDriver,									// init()
		_gdispPostInitDriver,								// postinit()
		_gdispDeInitDriver									// deinit()
	}
	, GDISPDRIVERID(init)
	, GDISPDRIVERID(start)
	, GDISPDRIVERID(write)
	#if GDISP_HARDWARE_VMT_DEINIT
		#if GDISP_HARDWARE_DEINIT
			, GDISPDRIVERID(deinit)
		#else
			, 0
		#endif
	#endif
	#if GDISP_HARDWARE_VMT_FLUSH
		#if GDISP_HARDWARE_FLUSH
			, GDISPDRIVERID(flush)
		#else
			, 0
		#endif
	#endif
	#if GDISP_HARDWARE_VMT_SETPOS
		#if GDISP_HARDWARE_SETPOS
			, GDISPDRIVERID(setpos)
		#else
			, 0
		#endif
	#endif
	#if GDISP_HARDWARE_VMT_READ
		#if GDISP_HARDWARE_READ
			, GDISPDRIVERID(read)
		#else
			, 0
		#endif
	#endif
	#if GDISP_HARDWARE_VMT_MOVE
		#if GDISP_HARDWARE_MOVE
			, GDISPDRIVERID(move)
		#else
			, 0
		#endif
	#endif
	#if GDISP_HARDWARE_VMT_IOCTL
		#if GDISP_HARDWARE_IOCTL
			, GDISPDRIVERID(ioctl)
		#else
			, 0
		#endif
	#endif
};

static const GDriverVMTList GDISPDRIVERID(VMTCHAIN) = {
	GDISP_VMT_CHAIN,
	&GDISPDRIVERID(VMT)
};

// Clean up the extra macro definitions	
#include "gdisp_driver_defs.h"
#undef GDISP_HARDWARE_EXTRADATA
#undef GDISP_VMT_CHAIN
#if	GDISP_MULTIPLE_DRIVERS
	#undef GDISP_DRIVER_NAME
#endif
