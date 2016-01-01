#ifndef __LINE_H__
#define __LINE_H__

#include "color.h"

#define LINE(w) ((line_t*)((w)->priv))

typedef struct {
    uint16_t dx,dy;
    color_t color;
} line_t;

#endif
