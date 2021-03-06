#ifndef __TEXT_H__
#define __TEXT_H__

#include "gfx.h"
#include "color.h"
#include "widget.h"

#define TEXT(w) ((text_t*)((w)->priv))


typedef struct
{
    gfxinfo_t *gfx;
    const gfxinfo_t *dest;
    char str[128];
    int update;
    uint8 fg,bg;
    const font_t *font;
} text_t;

void setupText(text_t *t, const gfxinfo_t *dest, const font_t*, const char *str);
void updateText(text_t *t, const char *str);
void drawTextWidget(text_t *t, int x, int y);


int text_set_font(widget_t *w, const char *name);
int text_set_color(widget_t *w, const char *name);
int text_set_bgcolor(widget_t *w, const char *name);
void *text_new(void*what);
void text_destroy(void*what);
void text_redraw(widget_t *w, int x, int y, gfxinfo_t *gfx);


#endif
