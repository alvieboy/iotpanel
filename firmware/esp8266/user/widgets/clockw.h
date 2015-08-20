#ifndef __CLOCKW_H__
#define __CLOCKW_H__

#include "gfx.h"
#include "color.h"
#include "text.h"
#include "widget.h"

#define CLOCK(w) ((clockw_t*)((w)->priv))

typedef struct
{
    text_t *text;
    unsigned millis;
    uint8 h,m,s;
    uint8 is24h;
} clockw_t;

#endif
