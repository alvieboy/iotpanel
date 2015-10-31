#include "widget.h"
#include "ets_sys.h"
#include "osapi.h"
#include <stdlib.h>
#include "gfx.h"
#include "widget_registry.h"
#include "alloc.h"
#include <string.h>
#include "debug.h"
#include "serdes.h"

LOCAL screen_t screens[MAX_SCREENS] = {{{0}}};
LOCAL screen_t *current_screen = &screens[0];

int ICACHE_FLASH_ATTR widget_set_property(widget_t*widget, const char *name, const char *value)
{
    property_t *prop;
    int li;
    char *end;

    for (prop = widget->def->properties; prop->name; prop++) {
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

void ICACHE_FLASH_ATTR widget_move(widget_entry_t *widget, int x, int y)
{
    widget->x = x;
    widget->y = y;
}


LOCAL void ICACHE_FLASH_ATTR widget_redraw(widget_entry_t *w, gfxinfo_t *gfx)
{
    w->widget->def->redraw(w->widget, w->x, w->y, gfx);
}

void ICACHE_FLASH_ATTR screen_draw(screen_t *screen, gfxinfo_t *gfx)
{
    widget_entry_t *w;
    gfx_clear(gfx);
    for (w=screen->widgets;w;w=w->next) {
        widget_redraw(w,gfx);
        //drawScrollingText(&scr);
    }
}

void ICACHE_FLASH_ATTR screen_add_widget(screen_t *screen, widget_t *widget, int x, int y)
{
    widget_entry_t *e = os_malloc(sizeof(widget_entry_t));
    widget_ref(widget);
    e->widget = widget;
    widget->parent = screen;

    e->next = NULL;
    if (screen->widgets==NULL)
        screen->widgets = e;
    else {
        widget_entry_t *it = screen->widgets;
        while (it->next)
            it = it->next;
        it->next = e;
    }
    e->x = x;
    e->y = y;
}

void ICACHE_FLASH_ATTR draw_current_screen(gfxinfo_t *gfx)
{
    if ((current_screen == NULL) || (current_screen->name[0] == '\0'))
        return;
    screen_draw(current_screen, gfx);
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

screen_t* ICACHE_FLASH_ATTR screen_find(const char *name)
{
    int i;
    for (i=0;i<MAX_SCREENS;i++) {
        if (strcmp(screens[i].name, name)==0) {
            return &screens[i];
        }
    }
    return NULL;
}

LOCAL void ICACHE_FLASH_ATTR widget_destroy(widget_t *w)
{
    w->def->destroy(w->priv);
    os_free(w);
}


void ICACHE_FLASH_ATTR screen_select(screen_t *s)
{
    current_screen = s;
}

void ICACHE_FLASH_ATTR widget_unref(widget_t *w)
{
    if (w->ref>0)
        w->ref--;
    if (w->ref==0)
        widget_destroy(w);
}

void ICACHE_FLASH_ATTR widget_ref(widget_t *w)
{
    w->ref++;
}

widget_t *ICACHE_FLASH_ATTR widget_create(const char *class, const char *name)
{
    const widgetdef_t *def = widgetdef_find(class);

    if (NULL==def)
        return NULL;

    widget_t *w = os_malloc(sizeof(widget_t));
    w->def = def;
    w->ref = 0;
    w->priv = def->alloc(NULL);
    strncpy(w->name, name, sizeof(w->name));
    return w;
}


LOCAL widget_t* ICACHE_FLASH_ATTR widget_find_in_screen(screen_t *s, const char *name)
{
    widget_entry_t *w = s->widgets;
    while (w) {
        if (strcmp(name,w->widget->name)==0)
            break;
        w = w->next;
    }
    if (w)
        return w->widget;
    return NULL;
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
    current_screen = NULL;

    /* Iterate through all screens */
    
    for (i=0;i<MAX_SCREENS;i++) {
        if (screens[i].name[0] != '\0') {
            /* Destroy all widgets */
            widget_entry_t *w = screens[i].widgets;
            while (w) {
                widget_entry_t *d = w;
                w=w->next;
                //widget_destroy(d->widget);
                widget_unref(d->widget);
                os_free(d);
            }
            screens[i].name[0] = '\0';
            screens[i].widgets = NULL;
         }
     }
}

int ICACHE_FLASH_ATTR widget_serialize_properties(serializer_t *ser, widget_t*widget)
{
    property_t *prop;
    char *strval;
    unsigned intval;


    for (prop = widget->def->properties; prop->name; prop++) {
        os_printf("Serialize prop %s\n", prop->name);

        serialize_string( ser, prop->name );

        switch (prop->type) {
        case T_INT:
            prop->getter( widget, &intval);
            serialize_uint32( ser, intval );
            break;
        case T_STRING:
            prop->getter( widget, &strval);
            serialize_string( ser, strval );
            break;
        default:
            return -1;
        }
    }
    return 0;
}

void ICACHE_FLASH_ATTR screen_serialize(serializer_t *ser, screen_t *screen)
{
    serialize_string( ser, screen->name );

    widget_entry_t *w = screen->widgets;
    while (w) {
        if (screen != w->widget->parent) {
            /* Cloned widget */
            serialize_uint8(ser, 0x02);
        } else {
            /* Normal widget */
            serialize_uint8(ser, 0x01);
        }
        os_printf("Serialize widget %s\n", w->widget->def->name);
        serialize_string(ser, w->widget->name);
        serialize_uint8(ser, w->x);
        serialize_uint8(ser, w->y);

        if (screen == w->widget->parent) {
            widget_serialize_properties(ser, w->widget);
        }
        w = w->next;
    }
    /* Last */
    serialize_uint8(ser, 0x00);
}

void ICACHE_FLASH_ATTR screen_serialize_all(serializer_t *ser)
{
    int i;
    current_screen = NULL;

    /* Iterate through all screens */
    
    for (i=0;i<MAX_SCREENS;i++) {
        if (screens[i].name[0] != '\0') {
            screen_serialize( ser, &screens[i]);
        }
    }
    /* Last one */
    serialize_uint8( ser, 0x00 );
}

