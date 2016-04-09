#include "text.h"
#include "widget.h"
#include "alloc.h"
#include "debug.h"
#include <string.h>
#include "protos.h"

void ICACHE_FLASH_ATTR setupText(text_t *t, const gfxinfo_t *dest,
                                 const font_t *font,
                                 const char *str)
{
    if (t==NULL)
        return;

    t->dest = dest;
    t->font = font;
    updateText(t, str);

    textrendersettings_t render;
    render.font = t->font;
    render.w = t->width;
    render.h = t->height;
    render.wrap = t->wrap;
    render.align = t->align;
    DEBUG("Text area width %d wrap %d\n", t->width, t->wrap);
    t->gfx = allocateTextFramebuffer(str, &render);
    if (t->gfx)
        drawText( t->gfx, &render, 0, 0, str,  t->fg, t->bg);
}

LOCAL int ICACHE_FLASH_ATTR text_set_text(widget_t *w, const char *str)
{
    text_t *t= TEXT(w);
    updateText(t, str);
    return 0;
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
        textrendersettings_t render;
        render.font = t->font;
        render.w = t->width;
        render.h = t->height;
        render.align = t->align;
        render.wrap = t->wrap;

        DEBUG("Text area width %d wrap %d %d\n", (int)t->width, (int)t->wrap, (int)render.wrap);

        if (t->gfx) {
            t->gfx = updateTextFramebuffer(t->gfx, &render, str );
        } else {
            t->gfx = allocateTextFramebuffer( str, &render);
        }
        /* Draw */
        if (t->gfx)
            drawText( t->gfx, &render, 0,0, str,  t->fg, t->bg);

        t->update = 0;
    }
    if (t->align == ALIGN_RIGHT && t->width>0) {
       // printf("\n\n\nADDING oFFSET %d\\n\n", (t->dest->width - t->gfx->width));
        x += (t->width - t->gfx->width);
    }
    if (t->gfx)
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
    if (s) {
        s->font = font_find("thumb");
        s->pstr = NULL;
        s->gfx = NULL;
        s->width = -1;
        s->height = -1;
        s->wrap = 0;
        s->align = ALIGN_LEFT;
    }
    return s;
}

void ICACHE_FLASH_ATTR text_destroy(void*what)
{
    text_t *s = what;
    if (s->pstr)
        os_free(s->pstr);
    if (s->paltstr)
        os_free(s->paltstr);
    // Free up gfx.
    if (s->gfx)
        destroyTextFramebuffer(s->gfx);
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

LOCAL int ICACHE_FLASH_ATTR text_set_speed(widget_t *w, const uint8_t *name)
{
    return 0;
}

LOCAL int ICACHE_FLASH_ATTR text_set_width(widget_t *w, int16_t *val)
{
    text_t *t=TEXT(w);
    t->width = *val;
    t->update = 1;
    return 0;
}

LOCAL int ICACHE_FLASH_ATTR text_set_wrap(widget_t *w, uint8_t *val)
{
    text_t *t=TEXT(w);
    t->wrap = *val;
    t->update = 1;
    return 0;
}

LOCAL int ICACHE_FLASH_ATTR text_set_align(widget_t *w, const char *val)
{
    text_t *t=TEXT(w);
    if (strcmp(val,"right")==0) {
        t->align = ALIGN_RIGHT;
    } else
        t->align = ALIGN_LEFT;
    t->update = 1;
    return 0;
}

LOCAL int ICACHE_FLASH_ATTR text_get_align(widget_t *w, const char **val)
{
    text_t *t=TEXT(w);
    if (t->align == ALIGN_RIGHT)
        *val = "right";
    else
        *val = "left";
    return 0;
}



const char * ICACHE_FLASH_ATTR text_get_font(widget_t *w)
{
    text_t *t = TEXT(w);
    return t->font->name;
}
const char *text_get_color(widget_t *w)
{
    text_t *t = TEXT(w);
    return color_name(t->fg);
}
const char *text_get_bgcolor(widget_t *w)
{
    text_t *t = TEXT(w);
    return color_name(t->bg);
}



STRING_GETTER( text_t, pstr );
STRING_GETTER( text_t, paltstr );
GENERIC_GETTER( text_t, uint8_t, speed );
FONT_GETTER( text_t, font );
COLOR_GETTER( text_t, fg );
COLOR_GETTER( text_t, bg );
GENERIC_GETTER( text_t, int16_t, width );
GENERIC_GETTER( text_t, uint8_t, wrap );
//GENERIC_GETTER( text_t, int8_t, height );

static property_t properties[] = {
    { 1, T_STRING, "text",   SETTER(text_set_text),    &text_t_get_pstr },
    { 2, T_STRING, "font",   SETTER(text_set_font),    &text_t_get_font },
    { 3, T_STRING, "color",  SETTER(text_set_color),   &text_t_get_fg },
    { 4, T_STRING, "bgcolor",SETTER(text_set_bgcolor), &text_t_get_bg },
    { 5, T_STRING, "alttext",SETTER(text_set_alttext), &text_t_get_paltstr },
    { 6, T_UINT8,  "speed",  SETTER(text_set_speed),   &text_t_get_speed },
    { 7, T_INT16,  "width",  SETTER(text_set_width),   &text_t_get_width },
    { 8, T_BOOL,   "wrap",   SETTER(text_set_wrap),    &text_t_get_wrap },
    { 9, T_STRING, "align",  SETTER(text_set_align),   GETTER(text_get_align)},

//    { 8, T_INT,    "height",  SETTER(text_set_speed),   &text_t_get_speed },
    END_OF_PROPERTIES
};

widgetdef_t text_widget = {
    .name = "text",
    .properties = properties,
    .alloc = &text_new,
    .redraw = &text_redraw,
    .destroy = &text_destroy
};

