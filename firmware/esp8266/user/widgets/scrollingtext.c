#include "scrollingtext.h"
#include "widget.h"
#include "alloc.h"
#include "debug.h"
#include <string.h>

void ICACHE_FLASH_ATTR setupScrollingText(scrollingtext_t *t, const gfxinfo_t *dest,
                                           const font_t *font,
                                           int y, const char *str)
{
    if (t==NULL)
        return;

    t->dest = dest;
    t->x = dest->width-1;
    t->y = y;
    t->font = font;
    if (str==NULL) {
        t->str[0] = '\0';
    }

    textrendersettings_t render;
    render.font = t->font;
    render.w = -1;
    render.h = -1;
    render.wrap = 0;

    updateScrollingText(t, str);

    t->gfx = allocateTextFramebuffer(str, &render);

    drawText( t->gfx, &render, 0, 0, str,  t->fg, t->bg);

}

LOCAL void ICACHE_FLASH_ATTR scrollingtext_set_text(widget_t *w, const char *str)
{
     scrollingtext_t *t= SCROLLINGTEXT(w);
    updateScrollingText(t, str);
}

void ICACHE_FLASH_ATTR updateScrollingText(scrollingtext_t *t, const char *str)
{
    if (t==NULL)
        return;
    if (str!=t->str)
        strcpy(t->str, str);
    t->update = 1;
}

void ICACHE_FLASH_ATTR drawScrollingText(scrollingtext_t *t)
{
    if (t==NULL || t->gfx==NULL || t->font==NULL)
        return;
    DEBUG("Drawing x %d, y %d t=%p gfx=%p\n", t->x, t->y, t, t->gfx);

    switch (overlayFramebuffer(t->gfx, t->dest, t->x, t->y, -1)) {
    case -1:
        t->x = t->dest->width-1;
        if (t->update) {

            textrendersettings_t render;
            render.font = t->font;
            render.w = -1;
            render.h = -1;
            render.wrap = 0;

            if (t->gfx) {
                DEBUG("Need update\n");
                t->gfx = updateTextFramebuffer(t->gfx, &render, t->str);
            } else {
                DEBUG("Need allocate\n");
                t->gfx = allocateTextFramebuffer(t->str, &render);
            }
            /* Draw */
            DEBUG("Draw scrolling\n");
            drawText( t->gfx, &render, 0,0, t->str,  t->fg, t->bg);
            t->update = 0;
        }

        break;
    default:
        DEBUG("Draw ok\n");
        if (t->count<=0) {
            t->x--;
            t->count = t->max;
        } else {
            t->count--;
        }
        break;
    }
    DEBUG("Finished\n");
}

int ICACHE_FLASH_ATTR scrollingtext_set_font(widget_t *w, const char *name)
{
    scrollingtext_t *t= SCROLLINGTEXT(w);
    t->font = font_find(name);
    if (t->font==NULL)
        return -1;
    t->update = 1;
    return 0;
}

int ICACHE_FLASH_ATTR scrollingtext_set_color(widget_t *w, const char *name)
{
    scrollingtext_t *t= SCROLLINGTEXT(w);
    if (color_parse(name, &t->fg)<0)
        return -1;
    t->bg = t->fg;
    t->update = 1;
    return 0;
}

int ICACHE_FLASH_ATTR scrollingtext_set_speed(widget_t *w, int *value)
{
    scrollingtext_t *t= SCROLLINGTEXT(w);
    if (value<0)
        return -1;
    t->max = *value;
    return 0;
}

static void *ICACHE_FLASH_ATTR scrollingtext_new(void*what)
{
    scrollingtext_t *s = os_calloc(sizeof(scrollingtext_t),1);
    s->font = font_find("thumb");
    s->count = 4;
    return s;
}

static void ICACHE_FLASH_ATTR scrollingtext_destroy(void*what)
{
    os_free(what);
}

void ICACHE_FLASH_ATTR scrollingtext_redraw(widget_t *w, int x, int y, gfxinfo_t *gfx)
{
    scrollingtext_t *t= SCROLLINGTEXT(w);
    t->y = y;

    if (!t->gfx) {
        setupScrollingText(t, gfx, t->font, t->y, t->str);
    }
    drawScrollingText(t);
}

STRING_GETTER( scrollingtext_t, str );
FONT_GETTER( scrollingtext_t, font );
COLOR_GETTER( scrollingtext_t, fg );
GENERIC_GETTER( scrollingtext_t, int, max );

static property_t properties[] = {
    { 1, T_STRING,"text",  SETTER(scrollingtext_set_text),  &scrollingtext_t_get_str },
    { 2, T_STRING,"font",  SETTER(scrollingtext_set_font),  &scrollingtext_t_get_font },
    { 3, T_STRING,"color", SETTER(scrollingtext_set_color), &scrollingtext_t_get_fg },
    { 4, T_INT,   "speed", SETTER(scrollingtext_set_speed), &scrollingtext_t_get_max },
    END_OF_PROPERTIES
};

widgetdef_t scrollingtext_widget = {
    .name = "scrollingtext",
    .properties = properties,
    .alloc = &scrollingtext_new,
    .redraw = &scrollingtext_redraw,
    .destroy = &scrollingtext_destroy
};

