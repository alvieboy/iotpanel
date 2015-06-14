#ifndef __RECTANGLE_H__
#define __RECTANGLE_H__

#include "color.h"

#define RECTANGLE(w) ((rectangle_t*)((w)->priv))

typedef struct {
    int w, h;
    color_t border_color;
    color_t bg_color;
    int fill:1;
    int border:1;
} rectangle_t;

#endif
