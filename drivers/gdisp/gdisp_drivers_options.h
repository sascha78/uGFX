/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gdisp/gdisp_drivers_options.h
 * @brief   GDISP sub-system drivers header file.
 *
 * @addtogroup GDISP
 * @{
 */

#ifndef _GDISP_DRIVERS_OPTIONS_H
#define _GDISP_DRIVERS_OPTIONS_H

	/**
	 * @name    GDISP Drivers
	 * @{
	 */
		/**
		 * @brief   Should the Win32 driver be included.
		 * @details	Defaults to FALSE
		 */
	#ifndef GDISP_DRIVER_WIN32
		#define GDISP_DRIVER_WIN32	FALSE
	#endif

#endif