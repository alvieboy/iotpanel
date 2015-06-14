#ifndef __FONT_H__
#define __FONT_H__


#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"

typedef struct {
    uint8_t w;
    uint8_t h;
    const uint8_t *bitmap;
    uint8_t start;
    uint8_t end;
    const char *name;
} font_t;


const font_t *font_find(const char *name);

#endif
