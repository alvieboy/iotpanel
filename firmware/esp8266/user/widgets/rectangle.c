#include "widget.h"
#include "rectangle.h"
#include "ets_sys.h"
#include "osapi.h"
#include "alloc.h"

LOCAL void ICACHE_FLASH_ATTR rectangle_redraw(widget_t *w, int x, int y, gfxinfo_t *gfx)
{
    rectangle_t *r = RECTANGLE(w);
    uint8_t *pix = &gfx->fb[x];
    pix += y * gfx->stride;

    int cx,cy;
    for (cy=0;cy<r->h;cy++) {
        for (cx=0;cx<r->w;cx++) {
            pix[cx] = r->bg_color;
        }
        pix += gfx->stride;
    }
}

LOCAL int ICACHE_FLASH_ATTR rectangle_set_width(widget_t *w, const int *width)
{
    rectangle_t *r = RECTANGLE(w);
    r->w = *width;
    return 0;
}

LOCAL int ICACHE_FLASH_ATTR rectangle_set_height(widget_t *w, const int *height)
{
    rectangle_t *r = RECTANGLE(w);
    r->h = *height;
    return 0;
}

LOCAL int ICACHE_FLASH_ATTR rectangle_set_fill(widget_t *w, const int *v)
{
    rectangle_t *r = RECTANGLE(w);
    r->fill = *v;
    return 0;
}

LOCAL int ICACHE_FLASH_ATTR rectangle_set_border(widget_t *w, const int *v)
{
    rectangle_t *r = RECTANGLE(w);
    r->border = *v;
    return 0;
}

LOCAL int ICACHE_FLASH_ATTR rectangle_set_color(widget_t *w, const char *v)
{
    rectangle_t *r = RECTANGLE(w);
    return color_parse(v, &r->bg_color);
}

static property_t properties[] = {
    { "width",  T_INT, SETTER(rectangle_set_width),  NULL },
    { "height", T_INT, SETTER(rectangle_set_height), NULL },
    { "fill",   T_INT, SETTER(rectangle_set_fill),   NULL },
    { "border", T_INT, SETTER(rectangle_set_border), NULL },
    { "color",  T_STRING, SETTER(rectangle_set_color), NULL },
    END_OF_PROPERTIES
};

LOCAL widget_t *ICACHE_FLASH_ATTR rectangle_new(void*what)
{
    return os_malloc(sizeof(rectangle_t));
}

LOCAL void ICACHE_FLASH_ATTR rectangle_destroy(void*what)
{
    os_free(what);
}

widgetdef_t rectangle_widget = {
    .name = "rectangle",
    .properties = properties,
    .alloc = &rectangle_new,
    .redraw = &rectangle_redraw,
    .destroy = &rectangle_destroy
};
