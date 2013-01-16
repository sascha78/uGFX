/*
    ChibiOS/GFX - Copyright (C) 2012
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
 * @file    include/tdisp/lld/tdisp_lld.h
 * @brief   TDISP driver subsystem low level driver header.
 *
 * @addtogroup TDISP
 * @{
 */

#ifndef _TDISP_LLD_H
#define _TDISP_LLD_H

#if GFX_USE_TDISP || defined(__DOXYGEN__)

#include "tdisp_lld_config.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void		tdisp_lld_write_cmd(uint8_t data);
extern void		tdisp_lld_write_data(uint8_t data);
extern bool_t	tdisp_lld_init(void);
extern void 	tdisp_lld_set_cursor(coord_t col, coord_t row);
extern void		tdisp_lld_create_char(uint8_t address, char *charmap);
extern void		tdisp_lld_clear(void);
extern void		tdisp_lld_home(void);

#ifdef __cplusplus
}
#endif

#endif /* GFX_USE_TDISP */

#endif /* _TDISP_LLD_H */
/** @} */

