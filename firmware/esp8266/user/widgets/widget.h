#ifndef __WIDGET_H__
#define __WIDGET_H__

#include "gfx.h"

#define MAX_SCREENS 4
#define NAMELEN 8

typedef enum {
    T_INT,
    T_STRING
} eType;

#define SETTER(x) ( (int(*)(void*,void*))&x )

struct widget;

typedef struct {
    const char *name;
    eType type;
    int (*setter)(void*, void*);
    void *data;
} property_t;
#define END_OF_PROPERTIES { 0,0,0 }

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
    struct widget *(*alloc)(void*);
    void (*redraw)(struct widget*, int, int, gfxinfo_t *gfx);
    void (*destroy)(void*);
} widgetdef_t;

typedef struct widget {
    char name[NAMELEN+1];
    struct widget *next;
    const widgetdef_t *def;
    void *priv;
    int x, y;
} widget_t;

typedef struct {
    char name[NAMELEN+1];
    widget_t *widgets;
} screen_t;

int widget_set_property(widget_t*widget, const char *name, const char *value);

void screen_draw(screen_t *screen, gfxinfo_t *gfx);
void screen_add_widget(screen_t *screen, widget_t *widget, int x, int y);
void draw_current_screen(gfxinfo_t *gfx);
screen_t* screen_create(const char *name);
widget_t *widget_create(const char *class, const char *name);
void widget_destroy(widget_t *w);
widget_t* widget_find(const char *name);


#endif
