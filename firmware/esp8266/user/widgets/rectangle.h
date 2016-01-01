#ifndef __RECTANGLE_H__
#define __RECTANGLE_H__

#include "color.h"

#define RECTANGLE(w) ((rectangle_t*)((w)->priv))

typedef struct {
    uint16_t w, h;
    color_t border_color;
    color_t color;
    color_t altcolor;
    uint8 flash;
    uint8 border;
    uint8 flashcount;
    unsigned int fill:1;
    unsigned int alt:1;
} rectangle_t;

#endif
