/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gdisp/gdisp_drivers_defs.h
 * @brief   GDISP sub-system drivers header file.
 */

// Make sure all values are defined
#ifndef  GDISP_HARDWARE_DEFINES
	#define GDISP_HARDWARE_DEFINES

/**
 * @name    GDISP hardware accelerated support
 * @{
 */
	/**
	 * @brief   The display is not static hardware. There is another API call to dynamically
	 *			create the display.
	 *
	 * @note	This is most useful for displays such as remote network displays.
	 */
	#ifndef GDISP_HARDWARE_DYNAMIC
		#define GDISP_HARDWARE_DYNAMIC		GFXOFF
	#endif

	/**
	 * @brief   The display hardware can benefit from being de-initialized when usage is complete.
	 *
	 * @note	This is most useful for displays such as remote network displays and other dynamic displays.
	 */
	#ifndef GDISP_HARDWARE_DEINIT
		#define GDISP_HARDWARE_DEINIT		GFXOFF
	#endif

	/**
	 * @brief   The display hardware can benefit from being flushed.
	 * @note	Some controllers ** require ** the application to flush
	 */
	#ifndef GDISP_HARDWARE_FLUSH
		#define GDISP_HARDWARE_FLUSH		GFXOFF
	#endif

	/**
	 * @brief   Reading of the display surface is supported.
	 */
	#ifndef GDISP_HARDWARE_READ
		#define GDISP_HARDWARE_READ			GFXOFF
	#endif

	/**
	 * @brief   Hardware supports setting the cursor position within the stream window.
	 * @note	This is used to optimise setting of individual pixels within a stream window.
	 * 			It should therefore not be implemented unless it is cheaper than just setting
	 * 			a new window.
	 */
	#ifndef GDISP_HARDWARE_SETPOS
		#define GDISP_HARDWARE_SETPOS		GFXOFF
	#endif

	/**
	 * @brief   Hardware supports moving arbitrary blocks from one position on the display to another.
	 * @note	This is typically used for accelerated scrolling.
	 */
	#ifndef GDISP_HARDWARE_MOVE
		#define GDISP_HARDWARE_MOVE			GFXOFF
	#endif

	/**
	 * @brief   The driver supports one or more control commands.
	 */
	#ifndef GDISP_HARDWARE_IOCTL
		#define GDISP_HARDWARE_IOCTL		GFXOFF
	#endif

/** @} */

#else
	#undef GDISP_HARDWARE_DYNAMIC
	#undef GDISP_HARDWARE_DEINIT
	#undef GDISP_HARDWARE_FLUSH
	#undef GDISP_HARDWARE_READ
	#undef GDISP_HARDWARE_SETPOS
	#undef GDISP_HARDWARE_MOVE
	#undef GDISP_HARDWARE_IOCTL
	#undef GDISP_HARDWARE_DEFINES
#endif
