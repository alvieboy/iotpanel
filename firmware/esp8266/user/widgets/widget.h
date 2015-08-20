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

struct widget;

typedef struct {
    uint8_t id;
    const char *name;
    eType type;
    int (*setter)(void*, void*);
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

typedef struct {
    const char *name;
    property_t *properties;
    void *(*alloc)(void*);
    void (*redraw)(struct widget*, int, int, gfxinfo_t *gfx);
    void (*destroy)(void*);
} widgetdef_t;

typedef struct widget {
    char name[NAMELEN+1];
    const widgetdef_t *def;
    void *priv;
    uint8_t ref;
} widget_t;

typedef struct widget_entry {
    struct widget_entry *next;
    widget_t *widget;
    int x, y;
} widget_entry_t;

typedef struct {
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


#endif
