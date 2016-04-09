#include "widget.h"
#include "romimage.h"
#include "smallfs.h"
#include "ets_sys.h"
#include "osapi.h"
#include "alloc.h"
#include <string.h>
#include "protos.h"
#include "error.h"

LOCAL void ICACHE_FLASH_ATTR romimage_redraw(widget_t *w, int x, int y, gfxinfo_t *gfx)
{
    romimage_t *r = ROMIMAGE(w);

    if ((r->w<0) || (r->h<0) || (r->fb==NULL))
        return;

    pixel_t *pix = &gfx->fb[x];
    pix += y * gfx->stride;
    pixel_t *src = &r->fb[0];
    int cx,cy;
    for (cy=0;cy<r->h;cy++) {
        for (cx=0;cx<r->w;cx++) {
            // TODO: Clip
            pix[cx] = *src;
            src++;
        }
        pix += gfx->stride;
    }
}

LOCAL int ICACHE_FLASH_ATTR romimage_set_width(widget_t *w, const uint16_t *width)
{
    romimage_t *r = ROMIMAGE(w);
    r->w = *width;
    return 0;
}

LOCAL int ICACHE_FLASH_ATTR romimage_set_height(widget_t *w, const uint16_t *height)
{
    romimage_t *r = ROMIMAGE(w);
    r->h = *height;
    return 0;
}

LOCAL int ICACHE_FLASH_ATTR romimage_set_filename(widget_t *w, const char *v)
{
    romimage_t *r = ROMIMAGE(w);

    if ((r->w<0)||(r->h<0))
        return -1;

    if (r->fb != NULL) {
        void *oldfb = r->fb;
        r->fb = NULL;
        os_free(oldfb);
    }


    // Open it up.
    struct smallfsfile file;
    strncpy(r->filename,v,sizeof(r->filename));
    int ret = NOERROR;

    if (smallfs__start() == 0) {
        if (smallfs__open(smallfs__getfs(), &file, r->filename)==0) {
            // Found. Allocate and link
            unsigned size = r->w * r->h;
            r->fb = (pixel_t*)os_malloc(size);
            if (r->fb) {
                if (smallfsfile__valid(&file)) {
                    smallfsfile__read(&file, r->fb, size);
                } else {
                    os_free(r->fb);
                    r->fb = NULL;
                    ret = EINTERNALERROR;
                    os_printf("Cannot open file %s!!!\n", r->filename);
                }
            } else {
                ret = ENOMEM;
            }
        } else {
            os_printf("Cannot open file %s!!!\n", r->filename);
            ret = ENOTFOUND;
        }
        smallfs__end(smallfs__getfs());
    } else {
        os_printf("Could not open smallfs!!!\n");
        ret = EINTERNALERROR;
    }

    if (r->fb == NULL) {
        r->filename[0] = '\0';
    }
    return ret;
}

LOCAL void *ICACHE_FLASH_ATTR romimage_new(void*what)
{
    romimage_t *r = os_calloc(sizeof(romimage_t),1);
    if (r) {
        r->fb = NULL;
        r->w = -1;
        r->h = -1;
        r->filename[0] = '\0';
    }
    return r;
}

LOCAL void ICACHE_FLASH_ATTR romimage_destroy(void*what)
{
    romimage_t *r = ROMIMAGE( (widget_t*)what);
    if (r->fb) {
        os_free(r->fb);
    }
    os_free(what);
}

LOCAL int ICACHE_FLASH_ATTR romimage_get_width(widget_t *w, uint16_t *dest)
{
    romimage_t *r = ROMIMAGE(w);
    *dest = r->w;
    return 0;
}

LOCAL int ICACHE_FLASH_ATTR romimage_get_height(widget_t *w, uint16_t *dest)
{
    romimage_t *r = ROMIMAGE(w);
    *dest = r->h;
    return 0;
}


STRING_GETTER( romimage_t, filename );

static property_t properties[] = {
    { 1, T_UINT16,"width",  SETTER(romimage_set_width),  GETTER(romimage_get_width) },
    { 2, T_UINT16,"height", SETTER(romimage_set_height), GETTER(romimage_get_height) },
    { 3, T_STRING,"filename",   SETTER(romimage_set_filename), GETTER(romimage_t_get_filename) },
    END_OF_PROPERTIES
};

widgetdef_t romimage_widget = {
    .name = "romimage",
    .properties = properties,
    .alloc = &romimage_new,
    .redraw = &romimage_redraw,
    .destroy = &romimage_destroy
};
