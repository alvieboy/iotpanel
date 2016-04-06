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
        return -1;
    return 0;
}

LOCAL inline int check_signed_bounds( long value, uint8_t size)
{
    unsigned bits = 8*size;
    long max = (1<<(bits-1))-1;
    long min = -1 - max;

    os_printf("Check bounds %ld between %ld and %ld\n", value, min, max);

    if ((value>max) || (value<min))
        return -1;
    os_printf("Bounds ok\n");
    return 0;
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
    int valid = -1;

    for (prop = widget->def->properties; prop->name; prop++) {
        os_printf("find prop %s %s\n", prop->name, name);
        if (strcmp(prop->name,name)==0) {

            if (is_integer_property( prop->type )) {
                li = (long)strtol(value,&end,10);
                if (*end !='\0') {
                    return -1;
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
                    os_printf("Invalid property\n");
                    valid = -1;
                }
                if (valid!=0) {
                    os_printf("Property out of bounds %ld for type %d (valid %d)\n", li, prop->type, valid);
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
                return -1;
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


LOCAL void ICACHE_FLASH_ATTR screen_add_widget_impl(screen_t *screen, widget_t *widget, int x, int y, int cloned)
{
    widget_entry_t *e = os_malloc(sizeof(widget_entry_t));
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
}

void ICACHE_FLASH_ATTR screen_add_widget(screen_t *screen, widget_t *widget, int x, int y)
{
    screen_add_widget_impl(screen,widget,x,y,0);
}

void ICACHE_FLASH_ATTR screen_add_cloned_widget(screen_t *screen, widget_t *widget, int x, int y)
{
    screen_add_widget_impl(screen,widget,x,y,1);
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
            os_printf("Creating new screen at %d '%s'\n", i, name);
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
    int r = 0;
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
        os_printf("Serialize prop %s\n", prop->name);

        if (prop->getter==NULL)
            continue;

        serialize_string( ser, prop->name );

        switch (prop->type) {
        case T_UINT8:
        case T_BOOL:
            r = prop->getter( widget, &intval.u8);
            if (r==0)
                serialize_uint8( ser, intval.u8);
            break;
        case T_UINT16:
            r = prop->getter( widget, &intval.u16);
            if (r==0)
                serialize_uint16( ser, intval.u16);
            break;
        case T_UINT32:
            r = prop->getter( widget, &intval.u32);
            if (r==0)
                serialize_uint32( ser, intval.u32);
            break;

        case T_INT8:
            r = prop->getter( widget, &intval.i8);
            if (r==0)
                serialize_uint8( ser, intval.i8);
            break;
        case T_INT16:
            r = prop->getter( widget, &intval.i16);
            if (r==0)
                serialize_uint16( ser, intval.i16);
            break;
        case T_INT32:
            r = prop->getter( widget, &intval.i32);
            if (r==0)
                serialize_uint32( ser, intval.i32);
            break;
        case T_STRING:
            prop->getter( widget, &strval);
            serialize_string( ser, strval );
            break;
        default:
            r = -1;
        }
    }
    if (r==0)
        r = serialize_uint8(ser, 0x00);
    return r;
}

#define WIDGET_NORMAL 0x01
#define WIDGET_CLONED 0x02

void ICACHE_FLASH_ATTR screen_serialize(serializer_t *ser, screen_t *screen)
{
    os_printf("Serializing screen %s\n", screen->name);
    serialize_string( ser, screen->name );
    int cnt = 0;

    widget_entry_t *w = screen->widgets;
    while (w) {
        if (screen != w->widget->parent) {
            /* Cloned widget */
            os_printf("Cloning widget from screen '%s'", w->widget->parent);
            serialize_uint8(ser, WIDGET_CLONED);
        } else {
            /* Normal widget */
            serialize_uint8(ser, WIDGET_NORMAL);
        }
        os_printf("Serialize widget %d '%s' class '%s'\n", cnt, w->widget->name, w->widget->def->name);
        serialize_string(ser, w->widget->name);
        serialize_int16(ser, w->x);
        serialize_int16(ser, w->y);

        if (screen == w->widget->parent) {
            serialize_string(ser, w->widget->def->name);
            widget_serialize_properties(ser, w->widget);
        }
        cnt++;
        w = w->next;
    }
    /* Last */
    serialize_uint8(ser, 0x00);
}

void ICACHE_FLASH_ATTR serialize_all(serializer_t *ser)
{
    int i;
    //current_screen = NULL;

    /* Iterate through all screens */
    os_printf("Current screen %p \n", current_screen);

    ser->initialize(ser);
    ser->truncate(ser);

    for (i=0;i<MAX_SCREENS;i++) {
        if (screens[i].name[0] != '\0') {
            os_printf("Serializing screen %d '%s'\n", i, screens[i].name);
            screen_serialize( ser, &screens[i]);
        }
    }
    /* Last one */
    serialize_uint8( ser, 0x00 );

    // Serialize current screen
    if (current_screen) {
        os_printf("Current screen %p %s\n", current_screen, current_screen->name);
        serialize_string( ser, current_screen->name );
    } else {
        os_printf("NULL current screen\n");
        serialize_uint8(ser, 0);
    }

    serialize_int32(ser, getBlanking());

    schedule_serialize(ser);


    ser->finalise(ser);
    ser->release(ser);
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
                    os_printf("String prop '%s'\n", value);
                    valid = prop->setter( w, (void*)value );
                    os_free(value);
                } else {
                    os_printf("Error deserializing string\n");
                    return -1;
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
    do {
        w = NULL;
        if (deserialize_uint8(ser,&type)<0) {
            return -1;
        }
        if (type==0) {
            break;
        }

        if (deserialize_string(ser, &widgetname[0], &namesize, sizeof(widgetname))<0) {
            DEBUGSERIALIZE("Cannot get widget name\n");
            return -1;
        }
        if (deserialize_uint16(ser,&x)<0) {
            DEBUGSERIALIZE("Cannot get x coordinate\n");
            return -1;
        }
        if (deserialize_uint16(ser,&y)<0) {
            DEBUGSERIALIZE("Cannot get y coordinate\n");
            return -1;
        }

        DEBUGSERIALIZE("Widget type %d '%s' at xy %d %d\n", type, widgetname,x,y);

        switch(type) {
        case WIDGET_CLONED:
            w = widget_find(widgetname);
            if (w==NULL) {
                DEBUGSERIALIZE("Cannot find widget with name '%s'\n", widgetname);
            }
            break;
        case WIDGET_NORMAL:
            if (deserialize_string(ser, &classname[0], &namesize, sizeof(classname))<0) {
                DEBUGSERIALIZE("Cannot get widget class name\n");
                return -1;
            }
            w = widget_create(classname, widgetname);
            if (w==NULL) {
                DEBUGSERIALIZE("Cannot create widget\n");
                return -1;
            }
            if (deserialize_properties(ser,w)<0) {
                DEBUGSERIALIZE("Cannot deserialize properties\n");
                return -1;
            }
            break;
        default:
            DEBUGSERIALIZE("Unknown widget type %d\n", type);
            break;
        }
        if (w==NULL) {
            DEBUGSERIALIZE("No Widget!\n");
            return -1;
        }
        screen_add_widget(screen,w,x,y);
    } while (1);
    return 0;
}

int ICACHE_FLASH_ATTR deserialize_all(serializer_t *ser)
{
    char screenname[NAMELEN+1];
    screen_t *screen = NULL;
    unsigned ssize;

    ser->initialize(ser);
    ser->rewind(ser);

    screen_destroy_all();
    do {
        if (deserialize_string(ser, &screenname[0], &ssize, sizeof(screenname))<0) {
            os_printf("Error deserializing screen name\n");
            ser->release(ser);

            return -1;
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

            return -1;
        }

        if (deserialize_screen(ser,screen)<0) {
            ser->release(ser);
            os_printf("Error deserializing screen\n");
            return -1;
        }
    } while (1);

    // Default screen
    if (deserialize_string(ser, &screenname[0], &ssize, sizeof(screenname))<0) {
        os_printf("Error deserializing default screen name\n");
        ser->release(ser);
        return -1;
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

    int32_t blank;
    if (deserialize_int32(ser, &blank)<0) {
        os_printf("Error deserializing blank\n");
        ser->release(ser);
        return -1;
    }
    os_printf("Blanking is %d\n", blank);
    setBlanking(blank);

    int r = schedule_deserialize(ser);
    ser->release(ser);
    return r;
}
