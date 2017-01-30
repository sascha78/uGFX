/**
 * This file has a different license to the rest of the GFX system.
 * You can copy, modify and distribute this file as you see fit.
 * You do not need to publish your source modifications to this file.
 * The only thing you are not permitted to do is to relicense it
 * under a different license.
 */

#ifndef _GFXCONF_H
#define _GFXCONF_H

/* The operating system to use. One of these must be defined - preferably in your Makefile */
//#define GFX_USE_OS_CHIBIOS            GFXOFF
//#define GFX_USE_OS_WIN32              GFXOFF
//#define GFX_USE_OS_LINUX              GFXOFF
//#define GFX_USE_OS_OSX                GFXOFF

/* GFX sub-systems to turn on */
#define GFX_USE_GDISP                   GFXON

/* Features for the GDISP sub-system. */
#define GDISP_NEED_VALIDATION           GFXOFF
#define GDISP_NEED_CLIP                 GFXOFF
#define GDISP_NEED_TEXT                 GFXOFF
#define GDISP_NEED_CIRCLE               GFXON

#endif /* _GFXCONF_H */
