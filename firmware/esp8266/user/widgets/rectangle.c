#include "widget.h"
#include "rectangle.h"
#include "ets_sys.h"
#include "osapi.h"
#include "alloc.h"

int ICACHE_FLASH_ATTR rectangle_is_border(rectangle_t*r,int x, int y)
{
    if ( ( x < r->border ) ||
        ( x >= (r->w) - (r->border)) ||
        (y<r->border) ||
        ( y >= (r->h) - (r->border)))
        return 1;

    return 0;
}

LOCAL void ICACHE_FLASH_ATTR rectangle_redraw(widget_t *w, int x, int y, gfxinfo_t *gfx)
{
    rectangle_t *r = RECTANGLE(w);

    if (r->flash) {
        r->flashcount++;
    }
    if (r->flashcount > r->flash) {
        r->flashcount=0;
        r->alt = ! r->alt;
    }

    uint8_t *pix = &gfx->fb[x];
    pix += y * gfx->stride;

    int cx,cy;
    for (cy=0;cy<r->h;cy++) {
        for (cx=0;cx<r->w;cx++) {
            if (rectangle_is_border(r,cx,cy)) {
                pix[cx] = r->border_color;
            } else {
                pix[cx] = r->alt ? r->altcolor : r->color;
            }
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
    return color_parse(v, &r->color);
}

LOCAL int ICACHE_FLASH_ATTR rectangle_set_altcolor(widget_t *w, const char *v)
{
    rectangle_t *r = RECTANGLE(w);
    return color_parse(v, &r->altcolor);
}

LOCAL int ICACHE_FLASH_ATTR rectangle_set_bordercolor(widget_t *w, const char *v)
{
    rectangle_t *r = RECTANGLE(w);
    return color_parse(v, &r->border_color);
}

LOCAL int ICACHE_FLASH_ATTR rectangle_set_flash(widget_t *w, const int *v)
{
    rectangle_t *r = RECTANGLE(w);
    r->flash = *v;
    if (r->flash==0)
        r->alt=0;
    return 0;
}

LOCAL void *ICACHE_FLASH_ATTR rectangle_new(void*what)
{
    rectangle_t *r = os_malloc(sizeof(rectangle_t));
    r->alt = 0;
    r->flash = 0;
    r->flashcount = 0;
}

LOCAL void ICACHE_FLASH_ATTR rectangle_destroy(void*what)
{
    os_free(what);
}

static property_t properties[] = {
    { "width",  T_INT, SETTER(rectangle_set_width),  NULL },
    { "height", T_INT, SETTER(rectangle_set_height), NULL },
    { "fill",   T_INT, SETTER(rectangle_set_fill),   NULL },
    { "border", T_INT, SETTER(rectangle_set_border), NULL },
    { "color",  T_STRING, SETTER(rectangle_set_color), NULL },
    { "bordercolor", T_STRING, SETTER(rectangle_set_bordercolor), NULL },
    { "altcolor", T_STRING, SETTER(rectangle_set_altcolor), NULL },
    { "flash",  T_INT, SETTER(rectangle_set_flash), NULL },
    END_OF_PROPERTIES
};

widgetdef_t rectangle_widget = {
    .name = "rectangle",
    .properties = properties,
    .alloc = &rectangle_new,
    .redraw = &rectangle_redraw,
    .destroy = &rectangle_destroy
};
