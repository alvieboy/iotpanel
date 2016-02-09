#include "font.h"
#include "ets_sys.h"
#include "osapi.h"
#include <string.h>

extern const font_t tom_thumb_font;
extern const font_t apple4x6_font;
extern const font_t apple5x7_font;
extern const font_t apple6x10_font;
extern const font_t font16x16_font;
extern const font_t font12x16_font;

LOCAL const font_t *fonts[] = {
    &tom_thumb_font,
    &apple4x6_font,
    &apple5x7_font,
    &apple6x10_font,
    &font16x16_font,
    &font12x16_font,
};

const font_t * ICACHE_FLASH_ATTR font_find(const char *name)
{
    unsigned i;
    for (i=0; i<sizeof(fonts)/sizeof(fonts[0]); i++) {
        if (strcmp(name,fonts[i]->name)==0)
            return fonts[i];
    }
    return NULL;
}
