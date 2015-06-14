#include "color.h"
#include <string.h>

struct color_entry {
    const char *name;
    color_t color;
};
struct color_entry color_list[] = {
    { "white", 0x7 },
    { 0,0 }
};

int color_parse(const char *name, color_t *color)
{
    struct color_entry *e = &color_list[0];
    while (e) {
        if (strcmp(name,e->name)==0) {
            *color = e->color;
            return 0;
        }
        e++;
    }
    return -1;
}
