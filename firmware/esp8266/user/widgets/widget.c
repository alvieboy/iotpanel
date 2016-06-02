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
#include "protos.h"
#include "schedule.h"
#include "error.h"

#define DEBUGSERIALIZE(x...) /* os_printf(x) */

LOCAL screen_t screens[MAX_SCREENS];
LOCAL screen_t *current_screen = &screens[0];

extern void setBlanking(int a);
extern int getBlanking();

void screens_init()
{
}


LOCAL inline bool is_integer_property( eType type )
{
    // Convert if integer type
    bool integer = true;
    switch (type) {
    case T_STRING:
        integer = false;
        break;
    default:
        break;
    }
    return integer;
}

LOCAL inline int check_unsigned_bounds( long value, uint8_t size)
{
    unsigned uv = (unsigned) value;
    unsigned bits = 8*size;
    unsigned mask = ~((1<<bits)-1);
    if (uv & mask)
        return EOUTOFBOUNDS;
    return NOERROR;
}

LOCAL inline int check_signed_bounds( long value, uint8_t size)
{
    unsigned bits = 8*size;
    long max = (1<<(bits-1))-1;
    long min = -1 - max;

    DEBUGSERIALIZE("Check bounds %ld between %ld and %ld\n", value, min, max);

    if ((value>max) || (value<min))
        return EOUTOFBOUNDS;

    DEBUGSERIALIZE("Bounds ok\n");
    return NOERROR;
}

const property_t * ICACHE_FLASH_ATTR widget_get_property(widget_t*widget,const char *name)
{
    const property_t *prop;
    for (prop = widget->def->properties; prop->name; prop++) {
        if (strcmp(prop->name,name)==0) {
            return prop;
        }
    }
    return NULL;
}


int ICACHE_FLASH_ATTR widget_set_property(widget_t*widget, const char *name, const char *value)
{
    const property_t *prop;
    long li;
    char *end;
    int valid = EINVALIDPROPERTY;

    for (prop = widget->def->properties; prop->name; prop++) {
        DEBUGSERIALIZE("find prop %s %s\n", prop->name, name);
        if (strcmp(prop->name,name)==0) {
            li = 0;
            if (is_integer_property( prop->type )) {
                li = (long)strtol(value,&end,10);
                if (*end !='\0') {
                    return EINVALIDARGUMENT;
                }
                // Check bounds
                switch (prop->type) {
                case T_UINT8:
                    valid = check_unsigned_bounds( li, sizeof(uint8_t) );
                    break;
                case T_UINT16:
                    valid = check_unsigned_bounds( li, sizeof(uint16_t) );
                    break;
                case T_UINT32:
                    valid = check_unsigned_bounds( li, sizeof(uint32_t) );
                    break;
                case T_INT8:
                    valid = check_signed_bounds( li, sizeof(int8_t) );
                    break;
                case T_INT16:
                    valid = check_signed_bounds( li, sizeof(int16_t) );
                    break;
                case T_INT32:
                    valid = check_signed_bounds( li, sizeof(int32_t) );
                    break;
                case T_BOOL:
                    valid = (li != 0) && (li!=1);
                    break;
                default:
                    DEBUGSERIALIZE("Invalid property\n");
                    valid = EINVALIDPROPERTY;
                }
                if (valid!=NOERROR) {
                    DEBUGSERIALIZE("Property out of bounds %ld for type %d (valid %d)\n", li, prop->type, valid);
                    return valid;
                }
            }

            switch (prop->type) {
#define SETBLOCK(type)  { type val = li; valid = prop->setter( widget, &val ); } break
            case T_UINT8:
            case T_BOOL:
                SETBLOCK(uint8_t);
            case T_INT8:
                SETBLOCK(int8_t);
            case T_UINT16:
                SETBLOCK(uint16_t);
            case T_INT16:
                SETBLOCK(int16_t);
            case T_UINT32:
                SETBLOCK(uint32_t);
            case T_INT32:
                SETBLOCK(int32_t);
            case T_STRING:
                return prop->setter( widget, (void*)value );
                break;
            default:
                //os_printf("Unknonw prop type\n");
                return EINVALIDPROPERTY;
            }
#undef SETBLOCK
        }
    }
    return valid;
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
    }
}


LOCAL int ICACHE_FLASH_ATTR screen_add_widget_impl(screen_t *screen, widget_t *widget, int x, int y, int cloned)
{
    widget_entry_t *e = os_malloc(sizeof(widget_entry_t));
    if (e==NULL)
        return ENOMEM;

    widget_ref(widget);

    e->widget = widget;

    if (!cloned)
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
    return NOERROR;
}

int ICACHE_FLASH_ATTR screen_add_widget(screen_t *screen, widget_t *widget, int x, int y)
{
    return screen_add_widget_impl(screen,widget,x,y,0);
}

int ICACHE_FLASH_ATTR screen_add_cloned_widget(screen_t *screen, widget_t *widget, int x, int y)
{
    return screen_add_widget_impl(screen,widget,x,y,1);
}

int ICACHE_FLASH_ATTR screen_move_widget(screen_t *screen, widget_t *widget, int x, int y)
{
    int r = ENOTFOUND;
    widget_entry_t *it = screen->widgets;
    while (it) {
        if (it->widget == widget) {
            it->x = x;
            it->y = y;
            r=NOERROR;
            break;
        }
        it = it->next;
    }
    return r;
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
            DEBUGSERIALIZE("Creating new screen at %d '%s'\n", i, name);
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
    if (w) {
        w->def = def;
        w->ref = 0;
        w->priv = def->alloc(NULL);
        strncpy(w->name, name, sizeof(w->name));
    }
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
    int r = NOERROR;
    typedef union {
        uint32_t u32;
        int32_t i32;
        uint16_t u16;
        int16_t i16;
        uint8_t u8;
        int8_t i8;
    } numericvalue;

    numericvalue intval;

    for (prop = widget->def->properties; prop->name; prop++) {
        DEBUGSERIALIZE("Serialize prop %s\n", prop->name);

        if (prop->getter==NULL)
            continue;

        r = serialize_string( ser, prop->name );
        if (r<0)
            return r;

        switch (prop->type) {
        case T_UINT8:
        case T_BOOL:
            r = prop->getter( widget, &intval.u8);
            if (r==NOERROR)
                r = serialize_uint8( ser, intval.u8);
            break;
        case T_UINT16:
            r = prop->getter( widget, &intval.u16);
            if (r==NOERROR)
                r = serialize_uint16( ser, intval.u16);
            break;
        case T_UINT32:
            r = prop->getter( widget, &intval.u32);
            if (r==NOERROR)
                r = serialize_uint32( ser, intval.u32);
            break;

        case T_INT8:
            r = prop->getter( widget, &intval.i8);
            if (r==NOERROR)
                r = serialize_uint8( ser, intval.i8);
            break;
        case T_INT16:
            r = prop->getter( widget, &intval.i16);
            if (r==NOERROR)
                r = serialize_uint16( ser, intval.i16);
            break;
        case T_INT32:
            r = prop->getter( widget, &intval.i32);
            if (r==NOERROR)
                r = serialize_uint32( ser, intval.i32);
            break;
        case T_STRING:
            r = prop->getter( widget, &strval);
            if (r==NOERROR)
                r = serialize_string( ser, strval );
            break;
        default:
            r = EINTERNALERROR;
            break;
        }
        if (r!=NOERROR)
            break;
    }
    if (r==NOERROR) {
        r = serialize_uint8(ser, 0x00);
    }
    return r;
}

#define WIDGET_NORMAL 0x01
#define WIDGET_CLONED 0x02

int ICACHE_FLASH_ATTR screen_serialize(serializer_t *ser, screen_t *screen)
{
    DEBUGSERIALIZE("Serializing screen %s\n", screen->name);
    int r = NOERROR;
    int cnt = 0;

    do {
        r = serialize_string( ser, screen->name );

        if (r!=NOERROR)
            break;

        widget_entry_t *w = screen->widgets;

        while (w) {
            if (screen != w->widget->parent) {
                /* Cloned widget */
                DEBUGSERIALIZE("Cloning widget from screen '%s'", w->widget->parent);
                r =serialize_uint8(ser, WIDGET_CLONED);
            } else {
                /* Normal widget */
                r = serialize_uint8(ser, WIDGET_NORMAL);
            }

            if (r!=NOERROR)
                break;

            DEBUGSERIALIZE("Serialize widget %d '%s' class '%s'\n", cnt, w->widget->name, w->widget->def->name);

            r =serialize_string(ser, w->widget->name);
            if (r!=NOERROR) break;
            r= serialize_int16(ser, w->x);
            if (r!=NOERROR) break;
            r= serialize_int16(ser, w->y);
            if (r!=NOERROR) break;

            if (screen == w->widget->parent) {
                r =serialize_string(ser, w->widget->def->name);
                if (r!=NOERROR) break;
                r =widget_serialize_properties(ser, w->widget);
                if (r!=NOERROR) break;
            }
            cnt++;
            w = w->next;
        }
        /* Last */
        r = serialize_uint8(ser, 0x00);
    } while (0);
    return r;
}

int ICACHE_FLASH_ATTR serialize_all(serializer_t *ser)
{
    int i;
    int r = NOERROR;
    /* Iterate through all screens */
    do {
        DEBUGSERIALIZE("Current screen %p \n", current_screen);

        r = ser->initialize(ser, -1);
        if (r!=NOERROR) break;

        r = ser->truncate(ser);
        if (r!=NOERROR) break;

        for (i=0;i<MAX_SCREENS;i++) {
            if (screens[i].name[0] != '\0') {
                DEBUGSERIALIZE("Serializing screen %d '%s'\n", i, screens[i].name);
                r = screen_serialize( ser, &screens[i]);
                if (r!=NOERROR) break;
            }
        }
        /* Last one */
        r = serialize_uint8( ser, 0x00 );
        if (r!=NOERROR) break;

        // Serialize current screen
        if (current_screen) {
            DEBUGSERIALIZE("Current screen %p %s\n", current_screen, current_screen->name);
            r = serialize_string( ser, current_screen->name );
        } else {
            DEBUGSERIALIZE("NULL current screen\n");
            r = serialize_uint8(ser, 0);
        }
        if (r!=NOERROR) break;

        r = serialize_int32(ser, getBlanking());
        if (r!=NOERROR) break;

        r = schedule_serialize(ser);
        if (r!=NOERROR) break;

        r = ser->finalise(ser);
        if (r!=NOERROR) break;

        r = ser->release(ser);
    } while (0);
    return r;
}

LOCAL int ICACHE_FLASH_ATTR deserialize_properties(serializer_t *ser, widget_t *w)
{
    char propname[NAMELEN+1];
    unsigned namesize;
    const property_t *prop;
    int r = 0;
    int valid=0;

    do {
        if (deserialize_string(ser, &propname[0], &namesize, sizeof(propname))<0) {
            DEBUGSERIALIZE("Cannot get widget property name\n");
            return -1;
        }
        if (namesize==0)
            break;

        DEBUGSERIALIZE("Prop '%s'\n", propname);
        prop = widget_get_property(w, propname);
        if (NULL==prop) {
            DEBUGSERIALIZE("No such property\n");
            return -1;
        }
        DEBUGSERIALIZE("Prop type is %d\n", prop->type);

        switch (prop->type) {
#define SETBLOCK(type,des)  { type val; r = des(ser,&val); if (r<0) break; valid = prop->setter( w, &val ); } break
        case T_UINT8:
        case T_BOOL:
            SETBLOCK(uint8_t, deserialize_uint8);
        case T_INT8:
            SETBLOCK(int8_t, deserialize_int8);
        case T_UINT16:
            SETBLOCK(uint16_t,deserialize_uint16);
        case T_INT16:
            SETBLOCK(int16_t, deserialize_int16);
        case T_UINT32:
            SETBLOCK(uint32_t, deserialize_uint32);
        case T_INT32:
            SETBLOCK(int32_t, deserialize_int32);
        case T_STRING:
            {
                DEBUGSERIALIZE("Deserialize string with alloc\n");
                char *value = deserialize_string_alloc(ser);
                if (value) {
                    DEBUGSERIALIZE("String prop '%s'\n", value);
                    valid = prop->setter( w, (void*)value );
                    os_free(value);
                } else {
                    DEBUGSERIALIZE("Error deserializing string\n");
                    return ENOMEM; // Or maybe not....
                }
            }
            break;
        }

    } while (1);

    if (valid!=0)
        r=-1;

    return r;
}

LOCAL int ICACHE_FLASH_ATTR deserialize_screen(serializer_t *ser, screen_t *screen)
{
    uint8_t type;
    char widgetname[NAMELEN+1];
    char classname[NAMELEN+1];
    unsigned namesize;
    widget_t *w;
    uint16_t x,y;
    int r = NOERROR;

    do {
        w = NULL;
        r = deserialize_uint8(ser,&type);
        if (r<0) break;

        if (type==0) {
            break;
        }

        r = deserialize_string(ser, &widgetname[0], &namesize, sizeof(widgetname));
        if (r<0) {
            DEBUGSERIALIZE("Cannot get widget name\n");
            break;
        }

        r = deserialize_uint16(ser,&x);
        if (r<0) {
            DEBUGSERIALIZE("Cannot get x coordinate\n");
            break;
        }
        r = deserialize_uint16(ser,&y);
        if (r<0) {
            DEBUGSERIALIZE("Cannot get y coordinate\n");
            break;
        }

        DEBUGSERIALIZE("Widget type %d '%s' at xy %d %d\n", type, widgetname,x,y);

        switch(type) {
        case WIDGET_CLONED:
            w = widget_find(widgetname);
            if (w==NULL) {
                DEBUGSERIALIZE("Cannot find widget with name '%s'\n", widgetname);
                r = ENOTFOUND;
            }
            break;
        case WIDGET_NORMAL:
            r = deserialize_string(ser, &classname[0], &namesize, sizeof(classname));
            if (r<0) {
                DEBUGSERIALIZE("Cannot get widget class name\n");
                break;
            }
            w = widget_create(classname, widgetname);
            if (w==NULL) {
                DEBUGSERIALIZE("Cannot create widget\n");
                r = ENOMEM;
                break;
            }
            r = deserialize_properties(ser,w);
            if (r<0) {
                DEBUGSERIALIZE("Cannot deserialize properties\n");
                break;
            }
            break;
        default:
            DEBUGSERIALIZE("Unknown widget type %d\n", type);
            r = EINVALIDARGUMENT;
            break;
        }

        if (r!=NOERROR)
            break;

        if (w==NULL) {
            DEBUGSERIALIZE("No Widget!\n");
            return EINTERNALERROR;
        }
        r = screen_add_widget(screen,w,x,y);
    } while (1);
    return r;
}

int ICACHE_FLASH_ATTR deserialize_all(serializer_t *ser)
{
    char screenname[NAMELEN+1];
    screen_t *screen = NULL;
    unsigned ssize;
    int r;
    int32_t blank;

    r = ser->initialize(ser, -1);

    if (r<0)
        return r;

    r = ser->rewind(ser);

    if (r<0)
        return r;

    screen_destroy_all();

    do {
        r = deserialize_string(ser, &screenname[0], &ssize, sizeof(screenname));
        if (r<0) {
            os_printf("Error deserializing screen name\n");
            ser->release(ser);
            break;
        }
        if (ssize==0) {
            os_printf("Last screen\n");
            break; // Last screen
        }

        os_printf("Screen %s\n", screenname);

        screen = screen_create(screenname);

        if (screen==NULL) {
            os_printf("Could not create screen\n");
            ser->release(ser);
            r = ENOMEM;
            break;
        }

        r = deserialize_screen(ser,screen);
        if (r<0) {
            ser->release(ser);
            os_printf("Error deserializing screen\n");
        }
    } while (1);

    if (r!=NOERROR)
        return r;

    // Default screen
    r = deserialize_string(ser, &screenname[0], &ssize, sizeof(screenname));
    if (r<0) {
        os_printf("Error deserializing default screen name\n");
        ser->release(ser);
        return r;
    }

    if (ssize) {
        os_printf("Default screen '%s'\n", screenname);
        screen = screen_find( screenname );
    } else {
        os_printf("No default screen\n");
    }

    if (screen)
        screen_select(screen);
    else
        screen_select( &screens[0] );

    r = deserialize_int32(ser, &blank);
    if (r<0) {
        os_printf("Error deserializing blank\n");
        ser->release(ser);
        return r;
    }
    os_printf("Blanking is %d\n", blank);
    setBlanking(blank);

    r = schedule_deserialize(ser);
    ser->release(ser);
    return r;
}
