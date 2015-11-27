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
    if (str!=t->pstr) {
        if (t->pstr)
            os_free(t->pstr);
        t->pstr = os_strdup(str);
    }
    t->update = 1;
}
LOCAL const char *ICACHE_FLASH_ATTR textGetCurrentString(text_t *t)
{
    return t->pstr;
}

void ICACHE_FLASH_ATTR drawTextWidget(text_t *t, int x, int y)
{
    if (t==NULL || t->gfx==NULL || t->font==NULL)
        return;
    const char *str = textGetCurrentString(t);
    if (str==NULL)
        return;
    
    if (t->update) {
        if (t->gfx) {
            t->gfx = updateTextFramebuffer(t->gfx, t->font, str );
        } else {
            t->gfx = allocateTextFramebuffer( str, t->font);
        }
        /* Draw */
        drawText( t->gfx, t->font, 0,0, str,  t->fg, t->bg);
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
    s->pstr = NULL;
    s->gfx = NULL;
    return s;
}

void ICACHE_FLASH_ATTR text_destroy(void*what)
{
    text_t *s = what;
    if (s->pstr)
        os_free(s->pstr);
    if (s->paltstr)
        os_free(s->paltstr);
    os_free(what);
}

void ICACHE_FLASH_ATTR text_redraw(widget_t *w, int x, int y, gfxinfo_t *gfx)
{
    text_t *t= TEXT(w);

    if (textGetCurrentString(t) == NULL)
        return;

    if (!t->gfx) {
        setupText(t, gfx, t->font, textGetCurrentString(t));
    }
    drawTextWidget(t,x,y);
}

LOCAL int ICACHE_FLASH_ATTR text_set_alttext(widget_t *w, const char *name)
{
    text_t *t= TEXT(w);
    if (t->paltstr)
        os_free(t->paltstr);
    t->paltstr = os_strdup(name);
    return 0;
}

LOCAL int ICACHE_FLASH_ATTR text_set_speed(widget_t *w, const char *name)
{
    return 0;
}

STRING_GETTER( text_t, pstr );
STRING_GETTER( text_t, paltstr );
GENERIC_GETTER( text_t, int, speed );
FONT_GETTER( text_t, font );
COLOR_GETTER( text_t, fg );
COLOR_GETTER( text_t, bg );

static property_t properties[] = {
    { 1, T_STRING, "text",   SETTER(text_set_text),    &text_t_get_pstr },
    { 2, T_STRING, "font",   SETTER(text_set_font),    &text_t_get_font },
    { 3, T_STRING, "color",  SETTER(text_set_color),   &text_t_get_fg },
    { 4, T_STRING, "bgcolor",SETTER(text_set_bgcolor), &text_t_get_bg },
    { 5, T_STRING, "alttext",SETTER(text_set_alttext), &text_t_get_paltstr },
    { 6, T_INT,    "speed",  SETTER(text_set_speed),   &text_t_get_speed },
    END_OF_PROPERTIES
};

widgetdef_t text_widget = {
    .name = "text",
    .properties = properties,
    .alloc = &text_new,
    .redraw = &text_redraw,
    .destroy = &text_destroy
};

