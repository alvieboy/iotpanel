#include "text.h"
#include "widget.h"
#include "alloc.h"
#include "debug.h"
#include <string.h>

void ICACHE_FLASH_ATTR setupText(text_t *t, const gfxinfo_t *dest,
                                 const font_t *font,
                                 const char *str)
{
    if (t==NULL)
        return;

    t->dest = dest;
    t->font = font;
    updateText(t, str);

    t->gfx = allocateTextFramebuffer(str, t->font);
    drawText( t->gfx, t->font,0,0, str,  t->fg, t->bg);
}

LOCAL void ICACHE_FLASH_ATTR text_set_text(widget_t *w, const char *str)
{
    text_t *t= TEXT(w);
    updateText(t, str);
}

void ICACHE_FLASH_ATTR updateText(text_t *t, const char *str)
{
    if (t==NULL)
        return;

    strcpy(t->str, str);
    t->update = 1;
}

void ICACHE_FLASH_ATTR drawTextWidget(text_t *t, int x, int y)
{
    if (t==NULL || t->gfx==NULL || t->font==NULL)
        return;
    
    if (t->update) {
        if (t->gfx) {
            t->gfx = updateTextFramebuffer(t->gfx, t->font, t->str);
        } else {
            t->gfx = allocateTextFramebuffer(t->str, t->font);
        }
        /* Draw */
        drawText( t->gfx, t->font, 0,0, t->str,  t->fg, t->bg);
        t->update = 0;
    }
    overlayFramebuffer(t->gfx, t->dest, x, y, t->fg==t->bg ? 0:-1);
}

int ICACHE_FLASH_ATTR text_set_font(widget_t *w, const char *name)
{
    text_t *t= TEXT(w);
    t->font = font_find(name);
    if (t->font==NULL)
        return -1;
    t->update = 1;
    return 0;
}

int ICACHE_FLASH_ATTR text_set_color(widget_t *w, const char *name)
{
    text_t *t= TEXT(w);
    if (color_parse(name, &t->fg)<0)
        return -1;
    t->update = 1;
    return 0;
}

int ICACHE_FLASH_ATTR text_set_bgcolor(widget_t *w, const char *name)
{
    text_t *t= TEXT(w);
    if (color_parse(name, &t->bg)<0)
        return -1;
    t->update = 1;
    return 0;
}

void *ICACHE_FLASH_ATTR text_new(void*what)
{
    text_t *s = os_calloc(sizeof(text_t),1);
    s->font = font_find("thumb");
    return s;
}

void ICACHE_FLASH_ATTR text_destroy(void*what)
{
    os_free(what);
}

void ICACHE_FLASH_ATTR text_redraw(widget_t *w, int x, int y, gfxinfo_t *gfx)
{
    text_t *t= TEXT(w);

    if (!t->gfx) {
        setupText(t, gfx, t->font, t->str);
    }
    drawTextWidget(t,x,y);
}

static property_t properties[] = {
    { 1,"text",  T_STRING, SETTER(text_set_text),  NULL },
    { 2,"font",  T_STRING, SETTER(text_set_font),  NULL },
    { 3,"color",  T_STRING, SETTER(text_set_color),  NULL },
    { 4,"bgcolor",  T_STRING, SETTER(text_set_bgcolor),  NULL },
    END_OF_PROPERTIES
};

widgetdef_t text_widget = {
    .name = "text",
    .properties = properties,
    .alloc = &text_new,
    .redraw = &text_redraw,
    .destroy = &text_destroy
};

