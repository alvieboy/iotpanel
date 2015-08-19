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

static property_t properties[] = {
    { 1, "dx",  T_INT, SETTER(line_set_dx),  NULL },
    { 2, "dy", T_INT, SETTER(line_set_dy), NULL },
    { 3, "color",  T_STRING, SETTER(line_set_color), NULL },
    END_OF_PROPERTIES
};

widgetdef_t line_widget = {
    .name = "line",
    .properties = properties,
    .alloc = &line_new,
    .redraw = &line_redraw,
    .destroy = &line_destroy
};
