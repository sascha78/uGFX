#pragma once

#include "../../../gfx.h"

#ifdef __cplusplus
extern "C" {
#endif

bool_t qimage_init(GDisplay* g, gCoord width, gCoord height);
void qimage_setPixel(GDisplay* g);
gColor qimage_getPixel(GDisplay* g);

#ifdef __cplusplus
}
#endif
