#ifndef __GIF_H__
#define __GIF_H__

#ifdef ENABLE_GIF

#include "libnsgif.h"

#define GIF(w) ((gif_t*)((w)->priv))

typedef struct {
    int16_t w, h;
    char filename[16];
    pixel_t **fb;
    int frames;
    int cframe;
    int delay;
} gif_t;

#endif

#endif
