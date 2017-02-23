/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gfx_mk.c
 * @brief   Single File Make.
 */

// Include the "Single File Make" compatible parts of uGFX
#include "gfx.c"

// Include the parts not garanteed to be fully compatible with single file make.
#include "gdisp/gdisp_mk.c"
#include "ginput/ginput_mk.c"
#include "gadc/gadc_mk.c"
#include "gaudio/gaudio_mk.c"
#include "gfile/gfile_mk.c"
