/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/gdisp_driver_options.h
 * @brief   GDISP driver configuration options.
 */

/**
 * This file is included to get all the driver's operational defines.
 * It is included 2 times - the first time to define everything, the 2nd time to cleanup the definitions ready for the next driver.
 * Outcomes:
 * 		GDISP_DRIVER_NAME			- The name of the current driver
 *		GDISP_DRIVER_xxxx			- Set according to the capabilities of the current driver
 *		GDISP_DRIVER_PIXELFORMAT	- Set according to the capabilities of the current driver
 *
 * On include 1:
 * 		The driver's config include file path must be in the macro GDISP_DRIVER_H
 *
 * On include 2:
 *		All settings are undefined except GDISP_DRIVER_NAME and GDISP_DRIVER_PIXELFORMAT
 *		GDISP_DRIVER_NAME and GDISP_DRIVER_PIXELFORMAT are also undefined if GDISP_MULTIPLE_DRIVERS is set.
 */

// Make sure all values are defined
#ifndef  _GDISP_DRIVER_OPTIONS_H
	#define _GDISP_DRIVER_OPTIONS_H

	// Clean up some definitions in case they have been included in the past by a previous driver
	#undef GDISP_DRIVER_NAME
	#undef GDISP_DRIVER_PIXELFORMAT

	// Include the required driver config file
	#include GDISP_DRIVER_H

/**
 * @name    GDISP driver support
 * @details	These are defined in the drivers config file
 * @{
 */
	/**
	 * @brief   The driver name.
	 * @note	All driver function names are prefixed by this string
	 * 			eg GDISP_DRIVER_NAME = GDISP_MyDriverName
	 *				leads to driver functions named GDISP_MyDriverName_init() etc.
	 * @details	Mandatory Setting. Defined in the driver config file.
	 */
	#ifndef GDISP_DRIVER_NAME
		#define GDISP_DRIVER_NAME			GDISP_MyDriverName
		#ifndef __DOXYGEN__
			#error "GDISP: A driver must always have a defined GDISP_DRIVER_NAME"
		#endif
	#endif
	
	/**
	 * @brief   The pixel format used by the driver. This is used to provide color definition and conversion functions for the driver
	 * @details	Mandatory Setting. Defined in the driver config file.
	 */
	#ifndef GDISP_DRIVER_PIXELFORMAT
		#define GDISP_DRIVER_PIXELFORMAT	GDISP_PIXELFORMAT_RGB888
		#ifndef __DOXYGEN__
			#error "GDISP: A driver must always have a defined GDISP_DRIVER_PIXELFORMAT"
		#endif
	#endif
	
	/**
	 * @brief   Should the driver use exact color conversions when converting colors
	 * @details	Optional Setting. Defined in the driver config file.
	 * @default Defaults to GFXOFF 
	 */
	#ifndef GDISP_DRIVER_EXACTCOLORS
		#define GDISP_DRIVER_EXACTCOLORS	GFXOFF
	#endif
	
	/**
	 * @brief   The display is not static hardware. There is another API call to dynamically
	 *			create the display.
	 * @details	Optional Setting. Defined in the driver config file.
	 * @default Defaults to GFXOFF 
	 * @note	This is most useful for displays such as remote network displays.
	 */
	#ifndef GDISP_DRIVER_DYNAMIC
		#define GDISP_DRIVER_DYNAMIC		GFXOFF
	#endif

	/**
	 * @brief   The display hardware can benefit from being de-initialized when usage is complete.
	 * @details	Optional Setting. Defined in the driver config file.
	 * @default Defaults to GFXOFF 
	 * @note	This is most useful for displays such as remote network displays and other dynamic displays.
	 */
	#ifndef GDISP_DRIVER_DEINIT
		#define GDISP_DRIVER_DEINIT			GFXOFF
	#endif

	/**
	 * @brief   The display hardware can benefit from being flushed.
	 * @details	Optional Setting. Defined in the driver config file.
	 * @default Defaults to GFXOFF 
	 * @note	Some controllers ** require ** the application to flush
	 */
	#ifndef GDISP_DRIVER_FLUSH
		#define GDISP_DRIVER_FLUSH			GFXOFF
	#endif

	/**
	 * @brief   Reading of the display surface is supported.
	 * @details	Optional Setting. Defined in the driver config file.
	 * @default Defaults to GFXOFF 
	 */
	#ifndef GDISP_DRIVER_READ
		#define GDISP_DRIVER_READ			GFXOFF
	#endif

	/**
	 * @brief   Hardware supports setting the cursor position within the stream window.
	 * @details	Optional Setting. Defined in the driver config file.
	 * @default Defaults to GFXOFF 
	 * @note	This is used to optimise setting of individual pixels within a stream window.
	 * 			It should therefore not be implemented unless it is cheaper than just setting
	 * 			a new window.
	 */
	#ifndef GDISP_DRIVER_SETPOS
		#define GDISP_DRIVER_SETPOS			GFXOFF
	#endif

	/**
	 * @brief   Hardware supports moving arbitrary blocks from one position on the display to another.
	 * @details	Optional Setting. Defined in the driver config file.
	 * @default Defaults to GFXOFF 
	 * @note	This is typically used for accelerated scrolling.
	 */
	#ifndef GDISP_DRIVER_MOVE
		#define GDISP_DRIVER_MOVE			GFXOFF
	#endif

	/**
	 * @brief   The driver supports one or more control commands.
	 * @details	Optional Setting. Defined in the driver config file.
	 * @default Defaults to GFXOFF 
	 */
	#ifndef GDISP_DRIVER_IOCTL
		#define GDISP_DRIVER_IOCTL			GFXOFF
	#endif

/** @} */

#else
	#undef _GDISP_DRIVER_OPTIONS_H
	#undef GDISP_DRIVER_H
	#undef GDISP_DRIVER_C
	#undef GDISP_DRIVER_EXACTCOLORS
	#undef GDISP_DRIVER_DYNAMIC
	#undef GDISP_DRIVER_DEINIT
	#undef GDISP_DRIVER_FLUSH
	#undef GDISP_DRIVER_READ
	#undef GDISP_DRIVER_SETPOS
	#undef GDISP_DRIVER_MOVE
	#undef GDISP_DRIVER_IOCTL
	#undef GDISP_DRIVER_BYTESDRIVER
	#undef GDISP_DRIVER_BYTESBOARD
	
	// These two driver capabilities are retained if this is the only driver in the system
	#if GDISP_MULTIPLE_DRIVERS
		#undef GDISP_DRIVER_NAME
		#undef GDISP_DRIVER_PIXELFORMAT
	#endif
#endif
