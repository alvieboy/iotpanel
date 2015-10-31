#ifndef __WIDGET_H__
#define __WIDGET_H__

#include "gfx.h"

#define MAX_SCREENS 8
#define NAMELEN 8

typedef enum {
    T_INT,
    T_STRING
} eType;

#define SETTER(x) ( (int(*)(void*,void*))&x )

typedef struct serializer_t serializer_t;

struct widget;

typedef struct {
    uint8_t id;
    const char *name;
    eType type;
    int (*setter)(void*, void*);
    int (*getter)(void*, void*);
    void *data;
} property_t;
#define END_OF_PROPERTIES { 0,0,0,0,0 }

#define INT_PROPERTY(name, struct, field) { \
    .name = name, \
    .type = I_INT, \
    .setter = generic_int_setter, \
    .data = offsetof( struct, field ) \
}
struct widget;
struct screen;

typedef struct {
    const char *name;
    property_t *properties;
    void *(*alloc)(void*);
    void (*redraw)(struct widget*, int, int, gfxinfo_t *gfx);
    void (*destroy)(void*);
} widgetdef_t;

#define GENERIC_GETTER(class, type, field) \
    LOCAL int get_##field(void *w, void *target) \
    { class *me = (class*)((widget_t*)w)->priv; \
    type *ti = (type*)target; \
    *ti = me->field; \
    return 0; \
    }

#define STRING_GETTER(class, field) \
    LOCAL int get_##field(void *w, void *target) \
    { class *me = (class*)((widget_t*)w)->priv; \
    const char **ti = (const char**)target; \
    *ti = &me->field[0]; \
    return 0; \
    }

#define FONT_GETTER(class, field) \
    LOCAL int get_##field(void *w, void *target) \
    { class *me = (class*)((widget_t*)w)->priv; \
    const char**ti = (const char**)target; \
    *ti = me->field->name; \
    return 0; \
    }

#define COLOR_GETTER(class, field) \
    LOCAL int get_##field(void *w, void *target) \
    { class *me = (class*)((widget_t*)w)->priv; \
    const char **ti = (const char**)target; \
    *ti = color_name(me->field); \
    return 0; \
    }

typedef struct widget {
    char name[NAMELEN+1];
    const widgetdef_t *def;
    struct screen *parent;
    void *priv;
    uint8_t ref;
} widget_t;

typedef struct widget_entry {
    struct widget_entry *next;
    widget_t *widget;
    int x, y;
} widget_entry_t;

typedef struct screen {
    char name[NAMELEN+1];
    widget_entry_t *widgets;
} screen_t;


void screen_draw(screen_t *screen, gfxinfo_t *gfx);
void screen_add_widget(screen_t *screen, widget_t *widget, int x, int y);
screen_t* screen_find(const char *name);
void screen_select(screen_t*);

void screen_destroy_all();
void draw_current_screen(gfxinfo_t *gfx);

int widget_set_property(widget_t*widget, const char *name, const char *value);
screen_t* screen_create(const char *name);
widget_t *widget_create(const char *class, const char *name);
widget_t* widget_find(const char *name);
void widget_ref(widget_t*widget);
void widget_unref(widget_t*widget);

void screen_serialize(serializer_t *ser, screen_t *screen);
void screen_serialize_all(serializer_t *ser);


#endif
