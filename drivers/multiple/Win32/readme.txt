To use this driver:

This driver is special in that it implements both the gdisp low level driver,
optionally a touchscreen driver, and optionally a toggle driver.

1. Add in your gfxconf.h:
	a) #define GFX_USE_GDISP			TRUE
	b) #define GDISP_DRIVER_WIN32		TRUE
	c) Optionally #define GFX_USE_GINPUT			TRUE
					#define GINPUT_USE_MOUSE		TRUE
					#define GINPUT_USE_TOGGLE		TRUE
	d) Optionally the following (with appropriate values):
		#define GDISP_WIN32_WIDTH	640
		#define GDISP_WIN32_HEIGHT	480


2. Modify your makefile to add -lws2_32 and -lgdi32 to the DLIBS line. i.e.
	DLIBS = -lws2_32 -lgdi32
