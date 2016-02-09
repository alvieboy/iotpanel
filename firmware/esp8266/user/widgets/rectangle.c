#include "widget.h"
#include "rectangle.h"
#include "ets_sys.h"
#include "osapi.h"
#include "alloc.h"

LOCAL  int ICACHE_FLASH_ATTR rectangle_is_border(rectangle_t*r,int x, int y)
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

        if (r->flashcount > r->flash) {
            r->flashcount=0;
            r->alt = ! r->alt;
        }
    } else {
        r->alt = 0;
    }

    pixel_t *pix = &gfx->fb[x];
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

LOCAL int ICACHE_FLASH_ATTR rectangle_set_width(widget_t *w, const uint16_t *width)
{
    rectangle_t *r = RECTANGLE(w);
    r->w = *width;
    return 0;
}

LOCAL int ICACHE_FLASH_ATTR rectangle_set_height(widget_t *w, const uint16_t *height)
{
    rectangle_t *r = RECTANGLE(w);
    r->h = *height;
    return 0;
}

LOCAL int ICACHE_FLASH_ATTR rectangle_set_fill(widget_t *w, const uint8_t *v)
{
    rectangle_t *r = RECTANGLE(w);
    r->fill = *v;
    return 0;
}

LOCAL int ICACHE_FLASH_ATTR rectangle_set_border(widget_t *w, const uint8_t *v)
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

LOCAL int ICACHE_FLASH_ATTR rectangle_set_flash(widget_t *w, const uint8_t *v)
{
    rectangle_t *r = RECTANGLE(w);
    r->flash = *v;
    return 0;
}

LOCAL void *ICACHE_FLASH_ATTR rectangle_new(void*what)
{
    rectangle_t *r = os_calloc(sizeof(rectangle_t),1);
    r->alt = 0;
    r->flash = 0;
    r->flashcount = 0;
    return r;
}

LOCAL void ICACHE_FLASH_ATTR rectangle_destroy(void*what)
{
    os_free(what);
}

LOCAL void ICACHE_FLASH_ATTR rectangle_get_width(widget_t *w, uint16_t *dest)
{
    rectangle_t *r = RECTANGLE(w);
    *dest = r->w;
}

LOCAL void ICACHE_FLASH_ATTR rectangle_get_height(widget_t *w, uint16_t *dest)
{
    rectangle_t *r = RECTANGLE(w);
    *dest = r->h;
}

LOCAL void ICACHE_FLASH_ATTR rectangle_get_fill(widget_t *w, uint8_t *dest)
{
    rectangle_t *r = RECTANGLE(w);
    *dest = r->fill;
}

LOCAL void ICACHE_FLASH_ATTR rectangle_get_border(widget_t *w, uint8_t *dest)
{
    rectangle_t *r = RECTANGLE(w);
    *dest = r->border;
}

LOCAL void ICACHE_FLASH_ATTR rectangle_get_color(widget_t *w, const char **dest)
{
    rectangle_t *r = RECTANGLE(w);
    *dest = color_name(r->color);
}

LOCAL void ICACHE_FLASH_ATTR rectangle_get_bordercolor(widget_t *w, const char **dest)
{
    rectangle_t *r = RECTANGLE(w);
    *dest = color_name(r->border_color);
}

LOCAL void ICACHE_FLASH_ATTR rectangle_get_altcolor(widget_t *w, const char **dest)
{
    rectangle_t *r = RECTANGLE(w);
    *dest = color_name(r->altcolor);
}

LOCAL void ICACHE_FLASH_ATTR rectangle_get_flash(widget_t *w, uint8_t *dest)
{
    rectangle_t *r = RECTANGLE(w);
    *dest = r->flash;
}




static property_t properties[] = {
    { 1, T_UINT16,"width",  SETTER(rectangle_set_width),  GETTER(rectangle_get_width) },
    { 2, T_UINT16,"height", SETTER(rectangle_set_height), GETTER(rectangle_get_height) },
    { 3, T_BOOL,  "fill",   SETTER(rectangle_set_fill),   GETTER(rectangle_get_fill) },
    { 4, T_UINT8, "border", SETTER(rectangle_set_border), GETTER(rectangle_get_border) },
    { 5, T_STRING,"color",  SETTER(rectangle_set_color), GETTER(rectangle_get_color) },
    { 6, T_STRING,"bordercolor", SETTER(rectangle_set_bordercolor), GETTER(rectangle_get_bordercolor) },
    { 7, T_STRING,"altcolor",  SETTER(rectangle_set_altcolor), GETTER(rectangle_get_altcolor) },
    { 8, T_UINT8, "flash",        SETTER(rectangle_set_flash), GETTER(rectangle_get_flash) },
    END_OF_PROPERTIES
};

widgetdef_t rectangle_widget = {
    .name = "rectangle",
    .properties = properties,
    .alloc = &rectangle_new,
    .redraw = &rectangle_redraw,
    .destroy = &rectangle_destroy
};
