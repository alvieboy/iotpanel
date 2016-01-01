#include "widget.h"
#include "line.h"
#include "ets_sys.h"
#include "osapi.h"
#include "alloc.h"

LOCAL void ICACHE_FLASH_ATTR line_redraw(widget_t *w, int x, int y, gfxinfo_t *gfx)
{
    line_t *l = LINE(w);

    gfx_drawLine(gfx, x, y, (x+l->dx), (y+l->dy), l->color);
}

LOCAL int ICACHE_FLASH_ATTR line_set_dx(widget_t *w, const int *value)
{
    line_t *l = LINE(w);
    l->dx = *value;
    return 0;
}

LOCAL int ICACHE_FLASH_ATTR line_set_dy(widget_t *w, const int *value)
{
    line_t *l = LINE(w);
    l->dy = *value;
    return 0;
}

LOCAL int ICACHE_FLASH_ATTR line_set_color(widget_t *w, const char *v)
{
    line_t *l = LINE(w);
    return color_parse(v, &l->color);
}

LOCAL void *ICACHE_FLASH_ATTR line_new(void*what)
{
    line_t *l = os_calloc(sizeof(line_t),1);
    return l;
}

LOCAL void ICACHE_FLASH_ATTR line_destroy(void*what)
{
    os_free(what);
}

LOCAL void ICACHE_FLASH_ATTR line_get_dx(widget_t *w, int16_t *dest)
{
    line_t *l = LINE(w);
    *dest = l->dx;
}

LOCAL void ICACHE_FLASH_ATTR line_get_dy(widget_t *w, int16_t *dest)
{
    line_t *l = LINE(w);
    *dest = l->dy;
}

LOCAL void ICACHE_FLASH_ATTR line_get_color(widget_t *w, const char **dest)
{
    line_t *l = LINE(w);
    *dest = color_name(l->color);
}

static property_t properties[] = {
    { 1, T_INT16,   "dx",   SETTER(line_set_dx),  GETTER(line_get_dx) },
    { 2, T_INT16,   "dy",   SETTER(line_set_dy),  GETTER(line_get_dy) },
    { 3, T_STRING,  "color",SETTER(line_set_color), GETTER(line_get_color) },
    END_OF_PROPERTIES
};

widgetdef_t line_widget = {
    .name = "line",
    .properties = properties,
    .alloc = &line_new,
    .redraw = &line_redraw,
    .destroy = &line_destroy
};
