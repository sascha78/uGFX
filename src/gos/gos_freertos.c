/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "../../gfx.h"
#include <string.h>

#if GFX_USE_OS_FREERTOS

#if INCLUDE_vTaskDelay != 1
	#error "GOS: INCLUDE_vTaskDelay must be defined in FreeRTOSConfig.h"
#endif

#if configUSE_MUTEXES != 1
	#error "GOS: configUSE_MUTEXES must be defined in FreeRTOSConfig.h"
#endif

#if configUSE_COUNTING_SEMAPHORES != 1
	#error "GOS: configUSE_COUNTING_SEMAPHORES must be defined in FreeRTOSConfig.h"
#endif

void _gosInit(void)
{
	#if !GFX_OS_NO_INIT
		#error "GOS: Operating System initialization for FreeRTOS is not yet implemented in uGFX. Please set GFX_OS_NO_INIT to GFXON in your gfxconf.h"
	#endif
	#if !GFX_OS_INIT_NO_WARNING
		#warning "GOS: Operating System initialization has been turned off. Make sure you call vTaskStartScheduler() before gfxInit() in your application!"
	#endif
}

void _gosDeinit(void)
{
}

void* gfxRealloc(void *ptr, size_t oldsz, size_t newsz)
{
	void *np;

	if (newsz <= oldsz)
		return ptr;

	np = gfxAlloc(newsz);
	if (!np)
		return 0;

	if (oldsz) {
		memcpy(np, ptr, oldsz);
		gfxFree(ptr);
	}

	return np;
}

void gfxSleepMilliseconds(delaytime_t ms)
{
	vTaskDelay(gfxMillisecondsToTicks(ms));
}

void gfxSleepMicroseconds(delaytime_t ms)
{

	// delay milli seconds - microsecond resolution delay is not supported in FreeRTOS
	vTaskDelay(gfxMillisecondsToTicks(ms/1000));
	// vUsDelay(ms%1000);
}

void gfxMutexInit(gfxMutex *pmutex)
{
	*pmutex = xSemaphoreCreateMutex();
	#if GFX_FREERTOS_USE_TRACE
		vTraceSetMutexName(*pmutex,"uGFXMutex");
	#endif
}

void gfxSemInit(gfxSem* psem, semcount_t val, semcount_t limit)
{
	if (val > limit)
		val = limit;

	*psem = xSemaphoreCreateCounting(limit,val);
	#if GFX_FREERTOS_USE_TRACE
		vTraceSetSemaphoreName(*psem, "uGFXSema");
	#endif
}

bool_t gfxSemWait(gfxSem* psem, delaytime_t ms)
{
	if (xSemaphoreTake(*psem, gfxMillisecondsToTicks(ms)) == pdPASS)
		return GTrue;
	return GFalse;
}

bool_t gfxSemWaitI(gfxSem* psem)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	if (xSemaphoreTakeFromISR(*psem, &xHigherPriorityTaskWoken) == pdTRUE)
		return GTrue;
	return GFalse;
}

void gfxSemSignal(gfxSem* psem)
{
	xSemaphoreGive(*psem);
	taskYIELD();
}

void gfxSemSignalI(gfxSem* psem)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	xSemaphoreGiveFromISR(*psem,&xHigherPriorityTaskWoken);
}

gfxThreadHandle gfxThreadCreate(void *stackarea, size_t stacksz, threadpriority_t prio, DECLARE_THREAD_FUNCTION((*fn),p), void *param)
{
	gfxThreadHandle task;
	(void) stackarea;

	if (stacksz < configMINIMAL_STACK_SIZE)
		stacksz = configMINIMAL_STACK_SIZE;

	task = 0;
	if (xTaskCreate(fn, "uGFX_TASK", stacksz, param, prio, &task) != pdPASS)
		return 0;

	return task;
}

#endif /* GFX_USE_OS_FREERTOS */
