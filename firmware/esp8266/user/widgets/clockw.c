#include "clock.h"
#include "clockw.h"
#include "widget.h"
#include "alloc.h"
#include "debug.h"
#include <string.h>
#include "widget_registry.h"


int ICACHE_FLASH_ATTR clock_set_font(widget_t *w, const char *name)
{
    clockw_t *t= CLOCK(w);
    widget_t wt;
    wt.priv = t->text;
    return text_set_font(&wt, name);
}

int ICACHE_FLASH_ATTR clock_set_color(widget_t *w, const char *name)
{
    clockw_t *t= CLOCK(w);
    widget_t wt;
    wt.priv = t->text;
    return text_set_color(&wt, name);
}

int ICACHE_FLASH_ATTR clock_set_bgcolor(widget_t *w, const char *name)
{
    clockw_t *t= CLOCK(w);
    widget_t wt;
    wt.priv = t->text;

    return text_set_bgcolor(&wt, name);
}

static void *ICACHE_FLASH_ATTR clock_new(void*what)
{
    clockw_t *s = os_calloc(sizeof(clockw_t),1);
    s->text = text_new(what);
    return s;
}

static void ICACHE_FLASH_ATTR clock_destroy(void*what)
{
    clockw_t *t= (clockw_t*)what;
    text_destroy(t->text);
    os_free(what);
}

void ICACHE_FLASH_ATTR clock_redraw(widget_t *w, int x, int y, gfxinfo_t *gfx)
{
    unsigned millis;
    char clk[32];
    uint8 h,m,s;
    int update = 0;

    clockw_t *t = CLOCK(w);
    widget_t wt;
    wt.priv = t->text;

    time_get_hmsm(&h, &m, &s, &millis);
    int thisup = (millis>=500);
    if (t->is24h) {
        if ( (t->h!=h) ||
            (t->m!=m) ||
            (t->s!=s))
        {
            update=1;
        }
    } else {
        // Check millis
        int lastup = (t->millis>=500);
        if ( (t->h!=h) ||
            (t->m!=m) ||
            (lastup!=thisup))
        {
            update=1;
        }

    }
    t->h=h;
    t->m=m;
    t->s=s;
    t->millis=millis;

    if (update) {
        if (t->is24h) {
            os_sprintf(clk, "%02d:%02d:%02d",
                       (unsigned)h,
                       (unsigned)m,
                       (unsigned)s
                      );
        } else {
            os_sprintf(clk, "%02d%c%02d",
                       (unsigned)h,
                       thisup ? ':':' ',
                       (unsigned)m
                      );
        }

        updateText( t->text, clk);
    }
    text_redraw(&wt, x, y, gfx);
}

static property_t properties[] = {
    { 1,"font",  T_STRING, SETTER(clock_set_font), NULL },
    { 2,"color",  T_STRING, SETTER(clock_set_color),  NULL },
    { 3,"bgcolor",  T_STRING, SETTER(clock_set_bgcolor),  NULL },
    END_OF_PROPERTIES
};

widgetdef_t clock_widget = {
    .name = "clock",
    .properties = properties,
    .alloc = &clock_new,
    .redraw = &clock_redraw,
    .destroy = &clock_destroy
};

