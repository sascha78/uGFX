GFXINC  +=
GFXSRC  +=
GFXLIBS +=
GFXDEFS += GDISP_DRIVER_WIN32=GFXON
# include $(GFXLIB)/drivers/multiple/Win32/driver.mk
# include $(GFXLIB)/drivers/gaudio/Win32/driver.mk

ifeq ($(OPT_OS),win32.raw32)
	GFXDEFS += GFX_OS_INIT_NO_WARNING=GFXON
endif
