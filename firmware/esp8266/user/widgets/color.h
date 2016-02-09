#ifndef __COLOR_H__
#define __COLOR_H__

#include "framebuffer.h"

typedef pixel_t color_t;

int color_parse(const char *text, color_t*color);
const char *color_name(color_t color);

#endif
