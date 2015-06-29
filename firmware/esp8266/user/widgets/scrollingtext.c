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
    updateScrollingText(t, str);

    t->gfx = allocateTextFramebuffer(str, t->font);
    drawText( t->gfx, t->font,0,0, str,  t->fg, t->bg);

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

    strcpy(t->str, str);
    t->update = 1;
}

void ICACHE_FLASH_ATTR drawScrollingText(scrollingtext_t *t)
{
    if (t==NULL || t->gfx==NULL || t->font==NULL)
        return;
    DEBUG("Drawing x %d, y %d t=%p gfx=%p\n", t->x, t->y, t, t->gfx);

    switch (overlayFramebuffer(t->gfx, t->dest, t->x, t->y)) {
    case -1:
        t->x = t->dest->width-1;
        if (t->update) {
            if (t->gfx) {
                DEBUG("Need update\n");
                t->gfx = updateTextFramebuffer(t->gfx, t->font, t->str);
            } else {
                DEBUG("Need allocate\n");
                t->gfx = allocateTextFramebuffer(t->str, t->font);
            }
            /* Draw */
            DEBUG("Draw scrolling\n");
            drawText( t->gfx, t->font, 0,0, t->str,  t->fg, t->bg);
            t->update = 0;
        }

        break;
    default:
        DEBUG("Draw ok\n");
        t->x--;
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

static void *ICACHE_FLASH_ATTR scrollingtext_new(void*what)
{
    scrollingtext_t *s = os_calloc(sizeof(scrollingtext_t),1);
    s->font = font_find("thumb");
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

static property_t properties[] = {
    { "text",  T_STRING, SETTER(scrollingtext_set_text),  NULL },
    { "font",  T_STRING, SETTER(scrollingtext_set_font),  NULL },
    { "color",  T_STRING, SETTER(scrollingtext_set_color),  NULL },
    END_OF_PROPERTIES
};

widgetdef_t scrollingtext_widget = {
    .name = "scrollingtext",
    .properties = properties,
    .alloc = &scrollingtext_new,
    .redraw = &scrollingtext_redraw,
    .destroy = &scrollingtext_destroy
};

