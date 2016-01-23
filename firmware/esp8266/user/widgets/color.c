#include "color.h"
#include <string.h>
#include "ets_sys.h"
#include "osapi.h"

struct color_entry {
    const char *name;
    color_t color;
};
struct color_entry color_list[] = {
    { "white", 0xff },
    { "red",   0x7 },
    { "green", 0x38 },
    { "blue",  0xC0},
    { "purple",  0xC7 },
    { "cyan",  0xF8 },
    { "black",  0x0 },
    { "yellow",  0x3F },
    { 0,0 }
};

int ICACHE_FLASH_ATTR color_parse(const char *name, color_t *color)
{
    struct color_entry *e = &color_list[0];
    while (e->name) {
        if (strcmp(name,e->name)==0) {
            *color = e->color;
            return 0;
        }
        e++;
    }
    return -1;
}

const char * ICACHE_FLASH_ATTR color_name(color_t color)
{
    struct color_entry *e = &color_list[0];
    while (e->name) {
        if (color == e->color) {
            return e->name;
        }
        e++;
    }
    return color_list[0].name;
}
