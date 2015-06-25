#include "color.h"
#include <string.h>
#include "ets_sys.h"
#include "osapi.h"

struct color_entry {
    const char *name;
    color_t color;
};
struct color_entry color_list[] = {
    { "white", 0x7 },
    { "red",   0x1 },
    { "green", 0x2 },
    { "blue",  0x4 },
    { "black",  0x0 },
    { "yellow",  0x3 },
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
