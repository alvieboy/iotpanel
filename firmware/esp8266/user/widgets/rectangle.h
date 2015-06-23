#ifndef __RECTANGLE_H__
#define __RECTANGLE_H__

#include "color.h"

#define RECTANGLE(w) ((rectangle_t*)((w)->priv))

typedef struct {
    int w, h;
    color_t border_color;
    color_t color;
    color_t altcolor;
    uint8 flash;
    uint8 border;
    uint8 flashcount;
    int fill:1;
    int alt:1;

} rectangle_t;

#endif
