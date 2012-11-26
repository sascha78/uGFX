/*
    ChibiOS/RT - Copyright (C) 2012
                 Joel Bodenmann aka Tectu <joel@unormal.org>

    This file is part of ChibiOS/GFX.

    ChibiOS/GFX is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/GFX is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file    drivers/ginput/toggle/Pal/ginput_lld_toggle_board_example.h
 * @brief   GINPUT Toggle low level driver source for the ChibiOS PAL hardware on the example board.
 *
 * @addtogroup GINPUT_TOGGLE
 * @{
 */

#ifndef _GDISP_LLD_TOGGLE_BOARD_H
#define _GDISP_LLD_TOGGLE_BOARD_H

#ifndef _GINPUT_LLD_TOGGLE_CONFIG_H
	// Visible in ginput.h

	#define GINPUT_TOGGLE_SW1		0				// Switch 1
	#define GINPUT_TOGGLE_SW2		1				// Switch 2
	#define GINPUT_TOGGLE_UP		2				// Joystick Up
	#define GINPUT_TOGGLE_DOWN		3				// Joystick Down
	#define GINPUT_TOGGLE_LEFT		4				// Joystick Left
	#define GINPUT_TOGGLE_RIGHT		5				// Joystick Right
	#define GINPUT_TOGGLE_CENTER	6				// Joystick Center

#elif !defined(GINPUT_TOGGLE_DECLARE_CONFIG)
	// Visible in ginput_lld.h

	#define GINPUT_TOGGLE_NUM_PORTS		7				// The total number of toggle inputs
	
#else
	// Visible in ginput_lld_toggle.c

	GToggleConfig GInputToggleConfigTable[] = {
		{AT91C_BASE_PIOB,								// Switch 1 and Switch 2
			PIOB_SW1_MASK|PIOB_SW2_MASK,
			PIOB_SW1_MASK|PIOB_SW2_MASK,
			PAL_MODE_INPUT},
		{AT91C_BASE_PIOA,								// B1..4 Joystick
			PIOA_B1_MASK|PIOA_B2_MASK|PIOA_B3_MASK|PIOA_B4_MASK|PIOA_B5_MASK,
			PIOA_B1_MASK|PIOA_B2_MASK|PIOA_B3_MASK|PIOA_B4_MASK|PIOA_B5_MASK,
			PAL_MODE_INPUT},
	};

#endif

#endif /* _GDISP_LLD_TOGGLE_BOARD_H */
/** @} */
