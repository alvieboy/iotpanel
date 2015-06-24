#include "widget.h"
#include "ets_sys.h"
#include "osapi.h"
#include <stdlib.h>
#include "gfx.h"
#include "widget_registry.h"
#include "alloc.h"
#include <string.h>

LOCAL screen_t screens[MAX_SCREENS] = {{{0}}};
LOCAL int current_screen = 0;

int ICACHE_FLASH_ATTR widget_set_property(widget_t*widget, const char *name, const char *value)
{
    property_t *prop;
    int li;
    char *end;

    for (prop = widget->def->properties; prop; prop++) {
        if (strcmp(prop->name,name)==0) {
            switch (prop->type) {
            case T_INT:
                li = (int)strtol(value,&end,10);
                if (*end !='\0')
                    return -1;
                return prop->setter( widget, &li );
                break;
            case T_STRING:
                return prop->setter( widget, (void*)value );
                break;
            default:
                return -1;
            }
        }
    }
    return -1;
}

LOCAL void ICACHE_FLASH_ATTR widget_redraw(widget_t *w, gfxinfo_t *gfx)
{
    w->def->redraw(w, w->x, w->y, gfx);
}

void ICACHE_FLASH_ATTR screen_draw(screen_t *screen, gfxinfo_t *gfx)
{
    widget_t *w;
    gfx_clear(gfx);
    for (w=screen->widgets;w;w=w->next) {
        widget_redraw(w,gfx);
        //drawScrollingText(&scr);
    }
}

void ICACHE_FLASH_ATTR screen_add_widget(screen_t *screen, widget_t *widget, int x, int y)
{
    widget->next = NULL;
    if (screen->widgets==NULL)
        screen->widgets = widget;
    else {
        widget_t *it = screen->widgets;
        while (it->next)
            it = it->next;
        it->next = widget;
    }
    widget->x = x;
    widget->y = y;
}

void ICACHE_FLASH_ATTR draw_current_screen(gfxinfo_t *gfx)
{
    if (screens[current_screen].name[0] == '\0')
        return;
    screen_draw(&screens[current_screen], gfx);
}

screen_t* ICACHE_FLASH_ATTR screen_create(const char *name)
{
    int i;
    for (i=0;i<MAX_SCREENS;i++) {
        if (screens[i].name[0] == '\0') {
            strncpy( screens[i].name, name, sizeof(screens[i].name) );
            screens[i].widgets = NULL;
            return &screens[i];
        }
    }
    return NULL;
}

void ICACHE_FLASH_ATTR widget_destroy(widget_t *w)
{
    w->def->destroy(w->priv);
    os_free(w);
}

widget_t *ICACHE_FLASH_ATTR widget_create(const char *class, const char *name)
{
    const widgetdef_t *def = widgetdef_find(class);

    if (NULL==def)
        return NULL;

    widget_t *w = os_malloc(sizeof(widget_t));
    w->def = def;
    w->priv = def->alloc(NULL);
    w->x=0;
    w->y=0;
    w->next = NULL;
    strncpy(w->name, name, sizeof(w->name));
    return w;
}


LOCAL widget_t* ICACHE_FLASH_ATTR widget_find_in_screen(screen_t *s, const char *name)
{
    widget_t *w = s->widgets;
    while (w) {
        if (strcmp(name,w->name)==0)
            break;
        w = w->next;
    }
    return w;
}

widget_t* ICACHE_FLASH_ATTR widget_find(const char *name)
{
    widget_t *r = NULL;
    int i;
    /* Iterate through all screens */
     for (i=0;i<MAX_SCREENS;i++) {
         if (screens[i].name[0] != '\0') {
             r = widget_find_in_screen( &screens[i], name);
             if (r)
                 break;
         }
     }
     return r;
}

void ICACHE_FLASH_ATTR screen_destroy_all()
{
    int i;
    /* Iterate through all screens */
    for (i=0;i<MAX_SCREENS;i++) {
        if (screens[i].name[0] != '\0') {
            /* Destroy all widgets */
            widget_t *w = screens[i].widgets;
            while (w) {
                widget_t *d = w;
                w=w->next;
                widget_destroy(d);
            }
            screens[i].name[0] = '\0';
         }
     }
 
}
