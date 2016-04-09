#include "font.h"
#include "ets_sys.h"
#include "osapi.h"
#include <string.h>
#include "smallfs.h"
#include "protos.h"
#include "alloc.h"


#define DEBUGFONT(x...) /* os_printf(x) */

extern font_t tom_thumb_font;

LOCAL font_t *fonts = &tom_thumb_font;

#ifndef ALIGN
#define ALIGN(x, alignment) (((x)+(alignment-1)) & ~(alignment-1))
#endif

const font_t * ICACHE_FLASH_ATTR font_find(const char *name)
{
    font_t *font;
    struct smallfsfile file;
    for (font = fonts; font; font=font->next) {
        if (strcmp(name,font->name)==0)
            return font;
    }

    // Font not found. Look it up
    if (smallfs__start() == 0) {
        if (smallfs__open(smallfs__getfs(), &file, name)==0) {
            // Found. Allocate and link
            font = (font_t*)os_malloc(sizeof(font_t));
            if (NULL!=font) {
                if (smallfsfile__valid(&file)) {
                    smallfsfile__read(&file, &font->hdr, sizeof(font->hdr));
                    DEBUGFONT("Loading font '%s', %dx%d, start %d end %d\n",
                              name,
                              (int)font->hdr.w,
                              (int)font->hdr.h,
                              (int)font->hdr.start,
                              (int)font->hdr.end);
                    unsigned bpp = ALIGN((font->hdr.w-1),8) / 8;
                    bpp *= ((font->hdr.end - font->hdr.start)+1);
                    bpp *= font->hdr.h;
                    strncpy(font->name,name,sizeof(font->name));
                    font->bitmap = os_malloc(bpp);
                    if (font->bitmap!=NULL) {
                        smallfsfile__read(&file, font->bitmap, bpp);
                        // Link it
                        font->next = fonts;
                        fonts = font;
                    } else {
                        os_free(font);
                        font = NULL;
                    }
                } else {
                    os_free(font);
                    font = NULL;
                }
            }
        }
        smallfs__end(smallfs__getfs());
    }

    if (NULL==font) {
        os_printf("Cannot find or alloc font '%s', reverting to 'thumb'\n", name);
        return font_find("thumb");
    }
    return font;
}
